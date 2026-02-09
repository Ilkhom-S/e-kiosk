/* @file Класс для архивации/разархивации папок и файлов. */

#include <QtCore/QCoreApplication>
#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>

#include <Common/ILog.h>

#include <Packer/Packer.h>
#include <zlib.h>

namespace CPacker {
const int DefaultTimeout = 300 * 1000; // таймаут распаковки/запаковки в мс.

// ported from https://stackoverflow.com/questions/2690328/qt-quncompress-gzip-data
const int GzipWindowsBit = 15 + 16;
const int GzipChunkSize = 32 * 1024;
}; // namespace CPacker

//---------------------------------------------------------------------------
// Возвращает имя исполняемого файла 7za в зависимости от платформы
QString Packer::getToolExecutableName() {
#ifdef Q_OS_WIN
    return "7za.exe";
#else
    return "7za";
#endif
}

//------------------------------------------------------------------------------
Packer::Packer(const QString &aToolPath, ILog *aLog)
    : m_ExitCode(0), m_UpdateMode(false), m_Format(Zip), m_Level(9), m_Recursive(false),
      m_Timeout(CPacker::DefaultTimeout), m_ToolPath(getToolExecutableName()) {
    if (aLog) {
        setLog(aLog);
    }

    setToolPath(aToolPath);
}

//------------------------------------------------------------------------------
void Packer::setToolPath(const QString &aToolPath) {
    if (!aToolPath.isEmpty()) {
        m_ToolPath = QDir::toNativeSeparators(
            QDir::cleanPath(aToolPath + QDir::separator() + getToolExecutableName()));

        toLog(LogLevel::Debug, QString("Zip tool path: '%1'").arg(m_ToolPath));
    }
}

//------------------------------------------------------------------------------
void Packer::setUpdateMode(bool aUpdateMode) {
    m_UpdateMode = aUpdateMode;
}

//---------------------------------------------------------------------------
bool Packer::gzipCompress(const QByteArray &aInBuffer,
                          const QString &aFileName,
                          QByteArray &aOutBuffer,
                          int aLevel) {
    // Prepare output
    aOutBuffer.clear();

    // Is there something to do?
    if (aInBuffer.length()) {
        // Declare vars
        int flush = 0;

        // Prepare deflator status
        z_stream strm;
        memset(&strm, 0, sizeof(strm));

        // Initialize deflator
        int ret = deflateInit2(&strm,
                               qMax(-1, qMin(9, aLevel)),
                               Z_DEFLATED,
                               CPacker::GzipWindowsBit,
                               8,
                               Z_DEFAULT_STRATEGY);

        if (ret != Z_OK) {
            return false;
        }

        gz_header header;
        memset(&header, 0, sizeof(header));
        QByteArray nameBuffer = aFileName.toLatin1();
        nameBuffer.append('\0');
        header.name = reinterpret_cast<Bytef *>(const_cast<char *>(nameBuffer.constData()));
        header.name_max = nameBuffer.size();
        header.time = QDateTime::currentDateTime().toSecsSinceEpoch();

        ret = deflateSetHeader(&strm, &header);
        if (ret != Z_OK) {
            return false;
        }

        // Prepare output
        aOutBuffer.clear();

        // Extract pointer to input data
        const char *inputData = aInBuffer.data();
        int inputDataLeft = aInBuffer.length();

        // Compress data until available
        do {
            // Determine current chunk size
            int chunkSize = qMin(CPacker::GzipChunkSize, inputDataLeft);

            // Set deflator references
            strm.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(inputData));
            strm.avail_in = chunkSize;

            // Update interval variables
            inputData += chunkSize;
            inputDataLeft -= chunkSize;

            // Determine if it is the last chunk
            flush = (inputDataLeft <= 0 ? Z_FINISH : Z_NO_FLUSH);

            // Deflate chunk and cumulate output
            do {
                // Declare vars
                char out[CPacker::GzipChunkSize];

                // Set deflator references
                strm.next_out = reinterpret_cast<unsigned char *>(out);
                strm.avail_out = CPacker::GzipChunkSize;

                // Try to deflate chunk
                ret = deflate(&strm, flush);

                // Check errors
                if (ret == Z_STREAM_ERROR) {
                    // Clean-up
                    deflateEnd(&strm);

                    // Return
                    return false;
                }

                // Determine compressed size
                int have = (CPacker::GzipChunkSize - strm.avail_out);

                // Cumulate result
                if (have > 0) {
                    aOutBuffer.append(reinterpret_cast<const char *>(out), have);
                }
            } while (strm.avail_out == 0);

        } while (flush != Z_FINISH);

        // Clean-up
        deflateEnd(&strm);

        // Return
        return (ret == Z_STREAM_END);
    }

    return true;
}

//------------------------------------------------------------------------------
bool Packer::gzipUncompress(const QByteArray &aInBuffer,
                            QString &aFileName,
                            QByteArray &aOutBuffer) {
    // Prepare output
    aFileName.clear();
    aOutBuffer.clear();

    // Is there something to do?
    if (aInBuffer.length() > 0) {
        // Prepare inflater status
        z_stream strm;
        memset(&strm, 0, sizeof(strm));

        // Initialize inflater
        int ret = inflateInit2(&strm, CPacker::GzipWindowsBit);

        if (ret != Z_OK) {
            return false;
        }

        gz_header header;
        memset(&header, 0, sizeof(header));
        QByteArray nameBuffer;
        nameBuffer.fill('\0', 256);
        header.name = reinterpret_cast<Bytef *>(nameBuffer.data());
        header.name_max = nameBuffer.size();

        ret = inflateGetHeader(&strm, &header);
        if (ret != Z_OK) {
            return false;
        }

        // Extract pointer to aInBuffer data
        const char *inputData = aInBuffer.data();
        int inputDataLeft = aInBuffer.length();

        // Decompress data until available
        do {
            // Determine current chunk size
            int chunkSize = qMin(CPacker::GzipChunkSize, inputDataLeft);

            // Check for termination
            if (chunkSize <= 0) {
                break;
            }

            // Set inflater references
            strm.next_in = reinterpret_cast<unsigned char *>(const_cast<char *>(inputData));
            strm.avail_in = chunkSize;

            // Update interval variables
            inputData += chunkSize;
            inputDataLeft -= chunkSize;

            // Inflate chunk and cumulate output
            do {
                // Declare vars
                char out[CPacker::GzipChunkSize];

                // Set inflater references
                strm.next_out = reinterpret_cast<unsigned char *>(out);
                strm.avail_out = CPacker::GzipChunkSize;

                // Try to inflate chunk
                ret = inflate(&strm, Z_NO_FLUSH);

                switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                case Z_STREAM_ERROR:
                    inflateEnd(&strm); // Clean-up
                    return false;
                }

                // Determine decompressed size
                int have = (CPacker::GzipChunkSize - strm.avail_out);

                // Cumulate result
                if (have > 0) {
                    aOutBuffer.append(reinterpret_cast<const char *>(out), have);
                }

            } while (strm.avail_out == 0);

        } while (ret != Z_STREAM_END);

        if (strlen(reinterpret_cast<const char *>(header.name))) {
            aFileName = QString::fromLatin1(reinterpret_cast<const char *>(header.name));
        }

        // Clean-up
        inflateEnd(&strm);

        // Return
        return (ret == Z_STREAM_END);
    }

    return true;
}

//------------------------------------------------------------------------------
QString Packer::pack(const QString &aTargetName,
                     const QString &aSourceDir,
                     const QStringList &aSearchMasks,
                     const QStringList &aExcludeWildcard) {
    QStringList files = pack(aTargetName, aSourceDir, aSearchMasks, aExcludeWildcard, 0);

    return files.isEmpty() ? QString() : files.first();
}

//------------------------------------------------------------------------------
QStringList Packer::pack(const QString &aTargetName,
                         const QString &aSourceDir,
                         const QStringList &aSearchMasks,
                         const QStringList &aExcludeWildcard,
                         int aMaxPartSize) {
    QStringList zipArguments =
        QStringList() << (m_UpdateMode ? "u" : "a") << (m_Format == SevenZip ? "-t7z" : "-tzip")
                      << "-bd" << QString("-mx=%1").arg(m_Level) << "-ssw" << "-y" << aTargetName;

    if (m_Recursive) {
        zipArguments << "-r";
    }

    for (const auto &mask : aSearchMasks) {
        zipArguments << aSourceDir + QDir::separator() + mask;
    }

    if (aMaxPartSize) {
        zipArguments << QString("-v%1b").arg(aMaxPartSize);
    }

    for (const auto &wildcard : aExcludeWildcard) {
        if (!wildcard.isEmpty()) {
            zipArguments << QString("-xr!%1").arg(wildcard);
        }
    }

    QFileInfo info(aTargetName);

    if (!m_UpdateMode) {
        // remove old archive
        for (const QString &file : QDir(info.absolutePath(), info.fileName() + "*").entryList()) {
            QFile::remove(QDir::toNativeSeparators(
                QDir::cleanPath(info.absolutePath() + QDir::separator() + file)));
        }
    }

    toLog(LogLevel::Normal,
          QString("Executing command: %1 %2").arg(m_ToolPath).arg(zipArguments.join(" ")));

    m_ZipProcess.start(m_ToolPath, zipArguments);
    if (!m_ZipProcess.waitForFinished(m_Timeout)) {
        m_ExitCode = -1;
        toLog(LogLevel::Error,
              QString("Unknown error while executing command or timeout expired(%1 sec): %2 %3")
                  .arg(m_Timeout / 1000., 0, 'f', 1)
                  .arg(m_ToolPath)
                  .arg(zipArguments.join(" ")));

        return QStringList();
    }

    m_ExitCode = m_ZipProcess.exitCode();
    m_Messages = QString::fromLocal8Bit(m_ZipProcess.readAllStandardOutput()).remove("\r");

    if (m_ExitCode == 1) {
        toLog(
            LogLevel::Warning,
            QString("Execute command have some warning: %1 %2. Return code: %3. Output stream: %4")
                .arg(m_ToolPath)
                .arg(zipArguments.join(" "))
                .arg(m_ExitCode)
                .arg(m_Messages));
    } else {
        if (m_ExitCode > 1) {
            toLog(LogLevel::Error,
                  QString("Can't execute command: %1 %2. Return code: %3. Output stream: %4")
                      .arg(m_ToolPath)
                      .arg(zipArguments.join(" "))
                      .arg(m_ExitCode)
                      .arg(m_Messages));

            return QStringList();
        }
    }

    return QDir(info.absolutePath(), info.fileName() + "*").entryList();
}

//------------------------------------------------------------------------------
bool Packer::test(const QString &aTargetName) {
    QString zipCommand = QString("t \"%1\"").arg(QDir::toNativeSeparators(aTargetName));

    m_ZipProcess.setArguments(QStringList() << zipCommand);
    m_ZipProcess.start(m_ToolPath);
    if (!m_ZipProcess.waitForFinished(m_Timeout)) {
        m_ExitCode = -1;
        toLog(LogLevel::Error,
              QString("Unknown error while executing command: %1 %2")
                  .arg(m_ToolPath)
                  .arg(zipCommand));

        return false;
    }
    m_ExitCode = m_ZipProcess.exitCode();
    m_Messages = QString::fromLocal8Bit(m_ZipProcess.readAllStandardOutput()).remove("\r");
    return m_ExitCode == 0;
}

//------------------------------------------------------------------------------
bool Packer::unpack(const QString &aSourceName,
                    const QString &aDestinationDir,
                    bool aSkipExisting,
                    const QStringList &aExtractFiles /*= QStringList()*/) {
    QStringList commanParams;

    commanParams << "x" << "-bd" << "-y";
    if (!aDestinationDir.isEmpty()) {
        commanParams << "-o" + aDestinationDir;
    }
    if (aSkipExisting) {
        commanParams << "-aos";
    }

    commanParams << aSourceName;
    commanParams.append(aExtractFiles);

    m_ZipProcess.start(m_ToolPath, commanParams);

    if (!m_ZipProcess.waitForFinished(m_Timeout)) {
        m_ExitCode = -1;
        toLog(LogLevel::Error,
              QString("Unknown error while executing command: %1 %2.")
                  .arg(m_ToolPath)
                  .arg(commanParams.join(" ")));

        return false;
    }

    m_ExitCode = m_ZipProcess.exitCode();
    m_Messages = QString::fromLocal8Bit(m_ZipProcess.readAllStandardOutput()).remove("\r");

    return m_ExitCode == 0;
}

//------------------------------------------------------------------------------
int Packer::exitCode() const {
    return m_ExitCode;
}

//------------------------------------------------------------------------------
const QString &Packer::messages() const {
    return m_Messages;
}

//------------------------------------------------------------------------------
void Packer::setLevel(int aLevel) {
    Q_ASSERT(aLevel >= 3 && aLevel <= 9);

    m_Level = aLevel;
}

//------------------------------------------------------------------------------
void Packer::setFormat(Packer::Format aFormat) {
    m_Format = aFormat;
}

//------------------------------------------------------------------------------
void Packer::setTimeout(int aTimeout) {
    m_Timeout = aTimeout;
}

//------------------------------------------------------------------------------
void Packer::terminate() {
    if (m_ZipProcess.state() != QProcess::NotRunning) {
        toLog(LogLevel::Error, "Terminate packer process.");

        m_ZipProcess.kill();
    }
}

//------------------------------------------------------------------------------
QString Packer::exitCodeDescription() const {
    switch (m_ExitCode) {
    case -1:
        return QString("Error or timeout execution %1").arg(getToolExecutableName());
    case 0:
        return "OK";
    case 1:
        return "OK (1)";
    case 2:
        return "7zip: Fatal error";
    case 7:
        return "7zip: Command line error";
    case 8:
        return "7zip: Not enough memory for operation";
    case 255:
        return "7zip: User stopped the process";
    default:
        return QString("7zip: Unknown exit code: %1").arg(m_ExitCode);
    }
}

//------------------------------------------------------------------------------
void Packer::setRecursive(bool aRecursive) {
    m_Recursive = aRecursive;
}

//------------------------------------------------------------------------------

/* @file Файл дистрибутива. */

#include "File.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>

#include <NetworkTaskManager/HashVerifier.h>
#include <utility>

File::File() : m_Size(0) {}

//---------------------------------------------------------------------------
File::File(QString aName, QString aHash, QString aUrl, qint64 aSize /*= 0*/)
    : m_Name(std::move(aName)), m_Hash(std::move(aHash)), m_Url(std::move(aUrl)), m_Size(aSize) {}

//---------------------------------------------------------------------------
bool File::operator==(const File &aFile) const {
    return (this->m_Name.compare(aFile.m_Name, Qt::CaseInsensitive) == 0 &&
            this->m_Hash.compare(aFile.m_Hash, Qt::CaseInsensitive) == 0);
}

//---------------------------------------------------------------------------
File::Result File::verify(const QString &aTempFilePath) const {
    QFileInfo fInfo(aTempFilePath);

    if (!fInfo.exists()) {
        return NotFullyDownloaded;
    }

    if (size() > 0) {
        if (fInfo.size() < size()) {
            return NotFullyDownloaded;
        }
        if (fInfo.size() > size()) {
            // файл на диске больше файла на сервере - нужно удалить и качать заново
            return Error;
        }
    }

    QFile f(aTempFilePath);
    if (f.open(QIODevice::ReadOnly)) {
        Sha256Verifier sha256V(hash());

        bool hashOK = sha256V.verify(nullptr, f.readAll());

        if (!hashOK && size() == 0) {
            return NotFullyDownloaded;
        }

        return hashOK ? OK : Error;
    }

    return Error;
}

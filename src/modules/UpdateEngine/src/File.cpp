/* @file Файл дистрибутива. */

#include "File.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>

#include <NetworkTaskManager/HashVerifier.h>

File::File() {
    m_Size = 0;
}

//---------------------------------------------------------------------------
File::File(const QString &aName, const QString &aHash, const QString &aUrl, qint64 aSize /*= 0*/)
    : m_Name(aName), m_Hash(aHash), m_Url(aUrl), m_Size(aSize) {}

//---------------------------------------------------------------------------
bool File::operator==(const File &aFile) const {
    return (this->m_Name.compare(aFile.m_Name, Qt::CaseInsensitive) == 0 &&
            this->m_Hash.compare(aFile.m_Hash, Qt::CaseInsensitive) == 0);
}

//---------------------------------------------------------------------------
File::Result File::verify(const QString &aTempFilePath) {
    QFileInfo fInfo(aTempFilePath);

    if (!fInfo.exists()) {
        return NotFullyDownloaded;
    }

    if (size() > 0) {
        if (fInfo.size() < size()) {
            return NotFullyDownloaded;
        } else if (fInfo.size() > size()) {
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
    } else {
        return Error;
    }
}

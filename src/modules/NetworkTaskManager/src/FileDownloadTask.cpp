/* @file Реализация класса-запроса для скачивания файла с докачкой. */

#include "FileDownloadTask.h"

#include <QtCore/QFile>

#include "FileDataStream.h"

FileDownloadTask::FileDownloadTask(const QUrl &aUrl, const QString &aPath)
    : m_Url(aUrl), m_Path(aPath) {
    setUrl(m_Url);
    setDataStream(new FileDataStream(m_Path));
    setFlags(NetworkTask::Continue);
}

//------------------------------------------------------------------------
QString FileDownloadTask::getPath() const {
    return m_Path;
}

//------------------------------------------------------------------------
void FileDownloadTask::closeFile() {
    getDataStream()->close();
}

//------------------------------------------------------------------------
void FileDownloadTask::resetFile() {
    closeFile();

    QFile::remove(m_Path);

    setDataStream(new FileDataStream(m_Path));
}

//------------------------------------------------------------------------

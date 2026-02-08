/* @file Реализация класса-запроса для скачивания файла с докачкой. */

#include "FileDownloadTask.h"

#include <QtCore/QFile>

#include <utility>

#include "FileDataStream.h"

FileDownloadTask::FileDownloadTask(QUrl aUrl, QString aPath)
    : m_Url(std::move(aUrl)), m_Path(std::move(aPath)) {
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

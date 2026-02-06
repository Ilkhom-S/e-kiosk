/* @file Реализация класса-запроса для скачивания файла с докачкой. */

#include "FileDownloadTask.h"

#include <QtCore/QFile>

#include "FileDataStream.h"

FileDownloadTask::FileDownloadTask(const QUrl &aUrl, const QString &aPath)
    : mUrl(aUrl), mPath(aPath) {
    setUrl(mUrl);
    setDataStream(new FileDataStream(mPath));
    setFlags(NetworkTask::Continue);
}

//------------------------------------------------------------------------
QString FileDownloadTask::getPath() const {
    return mPath;
}

//------------------------------------------------------------------------
void FileDownloadTask::closeFile() {
    getDataStream()->close();
}

//------------------------------------------------------------------------
void FileDownloadTask::resetFile() {
    closeFile();

    QFile::remove(mPath);

    setDataStream(new FileDataStream(mPath));
}

//------------------------------------------------------------------------

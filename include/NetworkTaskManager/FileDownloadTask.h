/* @file Реализация класса-запроса для скачивания файла с докачкой. */

#pragma once

#include <QtCore/QString>
#include <QtCore/QUrl>

#include "NetworkTask.h"

//------------------------------------------------------------------------
class FileDownloadTask : public NetworkTask {
    Q_OBJECT

public:
    FileDownloadTask(QUrl aUrl, QString aPath);

    QString getPath() const;

    void closeFile();

public slots:
    /// Удаляем файл и создаем его заново
    void resetFile();

protected:
    QUrl m_Url;
    QString m_Path;
};

//------------------------------------------------------------------------

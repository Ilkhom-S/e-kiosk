/* @file Реализация базового класса для сетевого запроса. */

#pragma once

#include <NetworkTaskManager/NetworkTask.h>

//------------------------------------------------------------------------
/// Запрос на загрузку файла по пути aUrl.
class DownloadTask : public NetworkTask {
public:
    DownloadTask(const QUrl &aUrl);
};

//------------------------------------------------------------------------

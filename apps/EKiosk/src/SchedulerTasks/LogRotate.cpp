/* @file Реализация задачи ротации журнальных файлов. */

// Модули
#include <Common/BasicApplication.h>

#include <System/IApplication.h>

// Проект
#include "LogRotate.h"
#include "Services/TerminalService.h"

//---------------------------------------------------------------------------
LogRotate::LogRotate(const QString &aName, const QString &aLogName, const QString &aParams)
    : ITask(aName, aLogName, aParams) {}

//---------------------------------------------------------------------------
void LogRotate::execute() {
    // Получаем доступ к терминальному сервису для закрытия логов перед ротацией
    auto *app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

    if (app) {
        auto *terminalService =
            dynamic_cast<TerminalService *>(app->getCore()->getTerminalService());

        if (terminalService) {
            // Закрываем все активные логи через клиента терминального сервиса
            terminalService->getClient()->execute("close_logs");
        }
    }

    // Выполняем ротацию всех журнальных файлов
    ILog::logRotateAll();

    emit finished(m_Name, true);
}

//---------------------------------------------------------------------------
bool LogRotate::subscribeOnComplete(QObject *aReceiver, const char *aSlot) {
    return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot) != nullptr;
}

//---------------------------------------------------------------------------

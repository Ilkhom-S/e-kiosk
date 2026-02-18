/* @file Реализация сторожевого сервиса как обычного приложения. */

#include <QtWidgets/QApplication>

#include <Common/BasicApplication.h>
#include <Common/Version.h>

#include <cstdlib>
#include <iostream>
#include <singleapplication.h>

#include "WatchService.h"

namespace CWatchService {
const QString Name = "WatchService";
} // namespace CWatchService

//----------------------------------------------------------------------------
void qtMessageHandler(QtMsgType /*aType*/,
                      const QMessageLogContext & /*aContext*/,
                      const QString &aMessage) {
    static ILog *log = ILog::getInstance("QtMessages");

    log->write(LogLevel::Normal, aMessage);
}

//----------------------------------------------------------------------------
// Точка входа в приложение сторожевого сервиса
int main(int aArgc, char *aArgv[]) {
    BasicQtApplication<SingleApplication> application(
        CWatchService::Name, Humo::getVersion(), aArgc, aArgv);

#ifndef Q_OS_MACOS
    // Check for single instance BEFORE creating QApplication
    if (application.getQtApplication().isSecondary()) {
        // single_instance_guard.sendMessage(application.getQtApplication().arguments().join('
        // ').toUtf8());
        qDebug() << "App already running.";
        qDebug() << "Primary instance PID: " << application.getQtApplication().primaryPid();
        qDebug() << "Primary instance user: " << application.getQtApplication().primaryUser();
        LOG(application.getLog(), LogLevel::Warning, "Another instance is already running.");
        return 0;
    }
#endif
    // TODO: restore breakpad integration when compatible with SingleApplication
    // QBreakpadInstance.setDumpPath(application.getWorkingDirectory() + "/logs/");

    // Перенаправляем логи
    ILog *mainLog = application.getLog();
    ILog::getInstance("ConfigManager")->setDestination(mainLog->getDestination());
    ILog::getInstance("CryptEngine")->setDestination(mainLog->getDestination());
    ILog::getInstance("QtMessages")->setDestination(mainLog->getDestination());

    // На macOS [NSApplication terminate:] вызывает exit() напрямую, минуя возврат из exec(),
    // поэтому qInstallMessageHandler(nullptr) после exec() может не выполниться.
    // Регистрируем atexit-хендлер (LIFO — выполнится раньше Qt-статиков) для гарантированного
    // снятия message handler до разрушения Qt-инфраструктуры.
    std::atexit([] { qInstallMessageHandler(nullptr); });
    qInstallMessageHandler(qtMessageHandler);

    WatchService service;

    int result = application.exec();

    qInstallMessageHandler(nullptr);

    return result;
}

//----------------------------------------------------------------------------

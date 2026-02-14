/* @file Реализация задачи запуска модуля обновления. */

// SDK
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Modules
#include <Common/BasicApplication.h>

#include <System/IApplication.h>

// Project
#include "RunUpdater.h"
#include "Services/RemoteService.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
RunUpdater::RunUpdater(const QString &aName, const QString &aLogName, const QString &aParams)
    : ITask(aName, aLogName, aParams), ILogable(aLogName), m_Params(aParams) {}

//---------------------------------------------------------------------------
RunUpdater::~RunUpdater() {}

//---------------------------------------------------------------------------
void RunUpdater::execute() {
    // Получаем доступ к приложению и настройкам терминала
    auto *app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

    PPSDK::ICore *core = app->getCore();
    PPSDK::TerminalSettings *terminalSettings = dynamic_cast<PPSDK::TerminalSettings *>(
        core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));

    auto urls = terminalSettings->getUpdaterUrls();
    if (urls.size() != 2 || urls[0].isEmpty() || urls[1].isEmpty()) {
        toLog(LogLevel::Error, "Invalid configuration updater URLs");

        emit finished(m_Name, false);
        return;
    }

    // Регистрируем команду обновления в сервисе мониторинга
    PPSDK::IRemoteService *monitoring = core->getRemoteService();

    int cmdId = monitoring->registerUpdateCommand(
        PPSDK::IRemoteService::Update, urls[0], urls[1], m_Params);

    if (cmdId > 0) {
        toLog(LogLevel::Normal,
              QString("Register update command %1 in monitioring service. Components '%2'")
                  .arg(cmdId)
                  .arg(m_Params));
    } else {
        toLog(LogLevel::Warning,
              QString("Error register update command in monitioring service. Components '%1'")
                  .arg(m_Params));
    }

    emit finished(m_Name, cmdId > 0);
}

//---------------------------------------------------------------------------
bool RunUpdater::subscribeOnComplete(QObject *aReceiver, const char *aSlot) {
    bool result =
        connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot) != nullptr;
    return result;
}

//---------------------------------------------------------------------------

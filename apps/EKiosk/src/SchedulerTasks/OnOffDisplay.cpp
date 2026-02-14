/* @file Реализация задачи включения энергосберегающего режима. */

// SDK
#include <SDK/PaymentProcessor/Core/EventTypes.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

// Modules
#include <Common/BasicApplication.h>

#include <SysUtils/ISysUtils.h>
#include <System/IApplication.h>

// Project
#include "OnOffDisplay.h"
#include "Services/GUIService.h"
#include "Services/ServiceNames.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
OnOffDisplay::OnOffDisplay(const QString &aName, const QString &aLogName, const QString &aParams)
    : ITask(aName, aLogName, aParams), ILogable(aLogName), m_Enable(false), m_Type(Standby) {
    // Парсим параметры: время начала, время окончания, тип действия
    QStringList params = aParams.split(";");

    if (params.size() >= 2) {
        m_From = QTime::fromString(params[0], "hh:mm");
        m_Till = QTime::fromString(params[1], "hh:mm");

        m_Enable = m_From.isValid() && !m_From.isNull() && m_Till.isValid() && !m_Till.isNull();
    }

    if (params.size() > 2) {
        if (params[2].compare("saver", Qt::CaseInsensitive) == 0) {
            m_Type = ScreenSaver;
        }

        if (params[2].compare("shutdown", Qt::CaseInsensitive) == 0) {
            m_Type = Shutdown;
        }
    }
}

//---------------------------------------------------------------------------
OnOffDisplay::~OnOffDisplay() {}

//---------------------------------------------------------------------------
void OnOffDisplay::execute() {
    if (m_Enable) {
        QTime now = QTime::currentTime();

        // Проверяем, находится ли текущее время в заданном диапазоне (учитываем переход через
        // полночь)
        bool off =
            m_Till < m_From ? (now > m_From || now < m_Till) : (now > m_From && now < m_Till);

        if (off) {
            auto *app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

            // Проверяем, можно ли выключать экран (если GUI сервис не против)
            if (!app->getCore()->getService(CServices::GUIService)->canShutdown()) {
                toLog(LogLevel::Normal, "Power saving disabled by GUI service");

                emit finished(m_Name, false);
                return;
            }

            // Выполняем действие в зависимости от типа
            switch (m_Type) {
            case ScreenSaver:
                ISysUtils::runScreenSaver();
                break;

            case Shutdown:
                app->getCore()->getEventService()->sendEvent(
                    PPSDK::Event(PPSDK::EEventType::Shutdown));
                break;

            default:
                ISysUtils::displayOn(false); // Standby режим - отключаем дисплей
            }
        } else {
            ISysUtils::displayOn(true); // Включаем дисплей вне диапазона
        }

        toLog(LogLevel::Normal, QString("Display %1").arg(off ? "OFF" : "ON"));
    }

    emit finished(m_Name, true);
}

//---------------------------------------------------------------------------
bool OnOffDisplay::subscribeOnComplete(QObject *aReceiver, const char *aSlot) {
    return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot) != nullptr;
}

//---------------------------------------------------------------------------

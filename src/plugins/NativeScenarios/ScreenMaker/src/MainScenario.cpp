/* @file Плагин сценария для создания скриншотов */

#include "MainScenario.h"

#include <QtCore/QDebug>
#include <QtCore/QSettings>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QWidget>

#include <SDK/GUI/IGraphicsHost.h>
#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/IGUIService.h>
#include <SDK/PaymentProcessor/Scripting/Core.h>
#include <SDK/Plugins/IExternalInterface.h>
#include <SDK/Plugins/PluginInitializer.h>

#include "ScenarioPlugin.h"

namespace {
/// Конструктор плагина.
SDK::Plugin::IPlugin *createPlugin(SDK::Plugin::IEnvironment *aFactory,
                                   const QString &aInstancePath) {
    return new ScreenMaker::MainScenarioPlugin(aFactory, aInstancePath);
}
} // namespace

REGISTER_PLUGIN(makePath(SDK::PaymentProcessor::Application,
                         PPSDK::CComponents::ScenarioFactory,
                         CScenarioPlugin::PluginName),
                &createPlugin,
                &SDK::Plugin::PluginInitializer::emptyParameterList,
                ScreenMakerMainScenario);

namespace ScreenMaker {

//---------------------------------------------------------------------------
MainScenario::MainScenario(SDK::PaymentProcessor::ICore *aCore, ILog *aLog)
    : Scenario(CScenarioPlugin::PluginName, aLog), m_Core(aCore), m_DrawAreaWindow(nullptr) {
    int displayIndex = 0;

    // ВАЖНО: Пытаемся получить display index из конфигурации, но это - некритичная операция.
    // Если adapter недоступен, используем default displayIndex=0.
    try {
        SDK::PaymentProcessor::ISettingsAdapter *adapter =
            m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter);

        if (adapter) {
            auto *terminalSettings =
                reinterpret_cast<SDK::PaymentProcessor::TerminalSettings *>(adapter);
            if (terminalSettings) {
                QString path = terminalSettings->getAppEnvironment().userDataPath +
                               QDir::separator() + "interface.ini";

                if (!path.isEmpty()) {
                    QSettings settings(path, QSettings::IniFormat);
                    foreach (QString key, settings.allKeys()) {
                        if (key == "interface/display") {
                            displayIndex = settings.value(key).toInt();
                            break;
                        }
                    }
                }
            }
        }
    } catch (std::exception &) {
        // Ignore any errors - use default displayIndex
        toLog(LogLevel::Warning,
              "ScreenMaker: Exception while reading display settings, using default");
    } catch (...) {
        // Ignore any errors - use default displayIndex
        toLog(LogLevel::Warning,
              "ScreenMaker: Unknown error while reading display settings, using default");
    }

    QRect rect = m_Core->getGUIService()->getScreenSize(displayIndex);

    m_DrawAreaWindow = new DrawAreaWindow(m_Core, rect);
}

//---------------------------------------------------------------------------
MainScenario::~MainScenario() = default;

//---------------------------------------------------------------------------
bool MainScenario::initialize(const QList<GUI::SScriptObject> & /*aScriptObjects*/) {
    return true;
}

//---------------------------------------------------------------------------
void MainScenario::start(const QVariantMap & /*aContext*/) {
    // m_Core->getGUIService()->show("ScreenMakerWidget", QVariantMap());
    m_DrawAreaWindow->updateImage(m_Core->getGUIService()->getScreenshot().toImage());
    m_DrawAreaWindow->setVisible(true);
}

//---------------------------------------------------------------------------
void MainScenario::stop() {
    m_TimeoutTimer.stop();
    m_DrawAreaWindow->setVisible(false);
}

//---------------------------------------------------------------------------
void MainScenario::pause() {}

//---------------------------------------------------------------------------
void MainScenario::resume(const QVariantMap & /*aContext*/) {}

//---------------------------------------------------------------------------
void MainScenario::signalTriggered(const QString & /*aSignal*/,
                                   const QVariantMap & /*aArguments*/) {
    QVariantMap parameters;
    emit finished(parameters);
}

//---------------------------------------------------------------------------
QString MainScenario::getState() const {
    return {"main"};
}

//---------------------------------------------------------------------------
void MainScenario::onTimeout() {
    signalTriggered("finish", QVariantMap());
}

//--------------------------------------------------------------------------
bool MainScenario::canStop() {
    return true;
}

} // namespace ScreenMaker

//---------------------------------------------------------------------------

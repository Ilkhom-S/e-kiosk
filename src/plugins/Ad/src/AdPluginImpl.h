/* @file Реализация плагина рекламы. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QTimer>

#include <SDK/Plugins/IKernel.h>
#include <SDK/Plugins/IPlugin.h>

#include "AdService.h"

//---------------------------------------------------------------------------
// Note: Adopted from original TerminalClient Ad plugin implementation
// Platform-specific: Compatible with Qt5/Qt6 for kiosk advertisement management
class AdPluginImpl : public QObject, public SDK::Plugin::IPlugin {
    Q_OBJECT

public:
    //---------------------------------------------------------------------------
    // Конструктор плагина
    AdPluginImpl(SDK::Plugin::IEnvironment *environment, QString instancePath);

    //---------------------------------------------------------------------------
    // Деструктор плагина
    virtual ~AdPluginImpl();

    // IPlugin interface implementation
    //---------------------------------------------------------------------------
    // Возвращает название плагина
    QString getPluginName() const override;

    //---------------------------------------------------------------------------
    // Возвращает имя файла конфигурации
    QString getConfigurationName() const override;

    //---------------------------------------------------------------------------
    // Возвращает текущую конфигурацию
    QVariantMap getConfiguration() const override;

    //---------------------------------------------------------------------------
    // Устанавливает новую конфигурацию
    void setConfiguration(const QVariantMap &configuration) override;

    //---------------------------------------------------------------------------
    // Сохраняет конфигурацию
    bool saveConfiguration() override;

    //---------------------------------------------------------------------------
    // Проверяет готовность плагина
    bool isReady() const override;

    //---------------------------------------------------------------------------
    // Инициализация плагина
    bool initialize(SDK::Plugin::IKernel *kernel);

    //---------------------------------------------------------------------------
    // Запуск плагина
    bool start();

    //---------------------------------------------------------------------------
    // Остановка плагина
    bool stop();

private:
    QString m_instancePath;
    SDK::Plugin::IEnvironment *m_environment;
    SDK::Plugin::IKernel *m_kernel;
    ILog *m_log;
    AdService *m_adService;
    QTimer *m_adTimer;

    //---------------------------------------------------------------------------
    // Настройка таймера рекламы
    void setupAdTimer();

    //---------------------------------------------------------------------------
    // Обработка показа рекламы
    void handleAdDisplay();

    //---------------------------------------------------------------------------
    // Загрузка конфигурации рекламы
    void loadAdConfiguration();
};

//---------------------------------------------------------------------------
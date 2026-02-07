/* @file Прокси класс для работы с объектами ядра в скриптах. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QVariantMap>

#include <SDK/PaymentProcessor/Scripting/AdService.h>
#include <SDK/PaymentProcessor/Scripting/DeviceService.h>
#include <SDK/PaymentProcessor/Scripting/FundsService.h>
#include <SDK/PaymentProcessor/Scripting/GUIService.h>
#include <SDK/PaymentProcessor/Scripting/HIDService.h>
#include <SDK/PaymentProcessor/Scripting/NetworkService.h>
#include <SDK/PaymentProcessor/Scripting/PaymentService.h>
#include <SDK/PaymentProcessor/Scripting/PrinterService.h>
#include <SDK/PaymentProcessor/Scripting/Settings.h>

namespace SDK {
namespace PaymentProcessor {

class ICore;

namespace Scripting {

//------------------------------------------------------------------------------
/// Имена объектов для экспорта в скрипты.
namespace CProxyNames {
extern const char Core[];
extern const char EventType[];
extern const char PaymentStep[];
extern const char PaymentStepResult[];
extern const char Payment[];
} // namespace CProxyNames

//------------------------------------------------------------------------------
/// Класс для логирования в скриптах.
class Log : public QObject, public ILogable {
    Q_OBJECT

public slots:
    /// Логирование нормального сообщения.
    void normal(const QString &aMessage) const { toLog(LogLevel::Normal, aMessage); }
    /// Логирование предупреждения.
    void warning(const QString &aMessage) const { toLog(LogLevel::Warning, aMessage); }
    void error(const QString &aMessage) const { toLog(LogLevel::Error, aMessage); }
    void debug(const QString &aMessage) const { toLog(LogLevel::Debug, aMessage); }
};

//------------------------------------------------------------------------------
/// Класс для работы со свойствами в скриптах.
class Properties : public QObject {
    Q_OBJECT

public:
    /// Конструктор.
    Properties(QVariantMap &aProperties) : mProperties(aProperties) {}

public slots:
    /// Получить значение свойства.
    QVariant get(const QString &aName) {
        if (has(aName)) {
            return mProperties[aName];
        }

        return QVariant();
    }

    /// Установить значение свойства.
    void set(const QString &aName, const QVariant &aValue) {
        mProperties[aName] = aValue;
        emit updated();
    }

    /// Проверить наличие свойства.
    bool has(const QString &aName) { return mProperties.contains(aName); }

signals:
    /// Сигнал об обновлении.
    void updated();

private:
    /// Ссылка на свойства.
    QVariantMap &mProperties;
};

//------------------------------------------------------------------------------
/// Прокси класс для работы с объектами ядра в скриптах.
class Core : public QObject {
    Q_OBJECT

    Q_PROPERTY(QObject *payment READ getPayment CONSTANT)
    Q_PROPERTY(QObject *printer READ getPrinter CONSTANT)
    Q_PROPERTY(QObject *charge READ getCharge CONSTANT)
    Q_PROPERTY(QObject *network READ getNetwork CONSTANT)
    Q_PROPERTY(QObject *graphics READ getGraphics CONSTANT)
    Q_PROPERTY(QObject *hardware READ getHardware CONSTANT)
    Q_PROPERTY(QObject *hid READ getHID CONSTANT)
    Q_PROPERTY(QObject *ad READ getAd CONSTANT)
    Q_PROPERTY(QObject *environment READ getSettings CONSTANT)
    Q_PROPERTY(QObject *log READ getLog CONSTANT)
    Q_PROPERTY(QObject *userProperties READ getUserProperties NOTIFY userPropertiesUpdated)

public:
    /// Конструктор.
    Core(ICore *aCore);

    /// Возвращает ядро.
    ICore *getCore() const;

    /// Делает сервис aService доступным в скрипте под именем aName.
    void installService(const QString &aName, QObject *aService);

    /// Установить лог, который будет виден из скрипта.
    void setLog(ILog *aLog);

public slots:
    /// Возвращает указатель на сервис, который был добавлен с помощью функции installService.
    QObject *getService(const QString &aName);

    /// Уведомление владельца оболочки о некоем событии aEvent с параметрами aParameters.
    void postEvent(int aEvent, QVariant aParameters);

    /// Метод для подсчета md5.
    QString getMD5Hash(const QString &aSource);

signals:
    /// Сигнал об обновлении пользовательских свойств.
    void userPropertiesUpdated();

protected:
    /// Возвращает указатель на интерфейс работы с платежами.
    QObject *getPayment();

    /// Возвращает указатель на интерфейс работы с принтером.
    QObject *getPrinter();

    /// Возвращает указатель на интерфейс работы с валидатором.
    QObject *getCharge();

    /// Возвращает указатель на интерфейс работы со сканером
    QObject *getHID();

    /// Возвращает указатель на интерфейс работы с сетью.
    QObject *getNetwork();

    /// Возвращает указатель на интерфейс работы с графикой.
    QObject *getGraphics();

    /// Возвращает указатель на интерфейс работы с рекламным контентом.
    QObject *getAd();

    /// Возвращает указатель на интерфейс работы С устройствами.
    QObject *getHardware();

    /// Возвращает указатель на интерфейс работы с настройками.
    QObject *getSettings();

    /// Возвращает указатель на интерфейс работы с логом.
    QObject *getLog();

    /// Возвращает объект с пользовательскими настройками
    QObject *getUserProperties();

private slots:
    /// Обработчик события.
    void onPostEvent(int aEvent, QVariant aParameters) const;

private:
    /// Указатель на ядро.
    ICore *mCore;

    /// Пользовательские свойства.
    Properties mUserProperties;
    /// Карта сервисов.
    QMap<QString, QObject *> mServices;

    /// Сервис средств.
    FundsService mFundsService;
    /// Сервис принтера.
    PrinterService mPrinterService;
    /// Сервис сети.
    NetworkService mNetworkService;
    /// Сервис платежей.
    PaymentService mPaymentService;
    /// Сервис GUI.
    GUIService mGUIService;
    /// Сервис рекламы.
    AdService mAdService;
    /// Сервис устройств.
    DeviceService mDeviceService;
    /// Настройки.
    Settings mSettings;
    /// Лог.
    Log mLog;
    HIDService mHID;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

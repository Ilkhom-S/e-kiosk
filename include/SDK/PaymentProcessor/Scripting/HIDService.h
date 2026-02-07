/* @file Прокси-класс для работы со сканером в скриптах. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Core/Event.h>

namespace SDK {
namespace PaymentProcessor {

class ICore;
class IHIDService;

namespace Scripting {

/// Константы для HID.
namespace HID {
extern const char STRING[];
extern const char EXTERNAL_DATA[];
extern const char RAW[];
extern const char RAW_BASE64[];
extern const char SOURCE[];
extern const char SOURCE_CAMERA[];
extern const char SOURCE_SCANNER[];
extern const char SOURCE_CARD[];
extern const char SIGNAL[];
extern const char SIGNAL_INSERT[];
extern const char SIGNAL_EJECT[];

extern const char CAMERA_FACE_DETECTED[];
extern const char CAMERA_FACE_DETECTED_IMAGE[];
} // namespace HID

//------------------------------------------------------------------------------
/// Прокси-класс для работы со сканером в скриптах.
class HIDService : public QObject, public ILogable {
    Q_OBJECT

public:
    /// Конструктор.
    HIDService(ICore *aCore);

public slots:
    /// Обновить параметры сервиса.
    void updateParameters(const QVariantMap &aParameters);

    /// Выполнить внешний обработчик.
    void executeExternalHandler(const QVariantMap &aExpression);

    /// Включить устройство.
    void enable(const QString &aName = QString());

    /// Отключить устройство.
    void disable(const QString &aName = QString());

signals:
    /// Сигнал получения данных со сканера
    void HIDData(const QVariantMap &aData);

    /// Сигнал об ошибке чтения штих-кода/карты/т.п.
    void error();

    void externalHandler(const QVariantMap &aExpression);

private slots:
    /// Обработчик сигнала от сервиса ядра
    void onData(const QVariantMap &aData);

    /// Карта вставлена
    void onInserted(const QVariantMap &aData);

    /// Карта изъята
    void onEjected();

private:
    /// Возвращает скрипт on_external_data для активного платежа, если он есть.
    QString getExternalData();

private:
    /// Указатель на ядро.
    ICore *mCore;
    /// Указатель на сервис HID.
    IHIDService *mService;
    /// Параметры.
    QVariantMap mParameters;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

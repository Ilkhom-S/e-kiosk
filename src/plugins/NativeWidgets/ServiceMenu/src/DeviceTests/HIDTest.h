/* @file Класс для тестирования сканеров. */

#pragma once

#include <QtCore/QFutureWatcher>

#include <Common/ObjectPointer.h>

#include <SDK/Drivers/IHID.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace Driver {
class IDevice;
} // namespace Driver
} // namespace SDK

//------------------------------------------------------------------------------
class HIDTest : public QObject, public SDK::PaymentProcessor::IDeviceTest {
    Q_OBJECT

public:
    HIDTest(SDK::Driver::IDevice *aDevice, const QString &aInstancePath);

    /// Возвращает имена и описания тестов.
    virtual QList<QPair<QString, QString>> getTestNames() const;

    /// Запускает тестирование устройства.
    virtual bool run(const QString &aName = QString());

    /// Остановка процесса тестирования.
    virtual void stop();

    /// Можно тестировать?
    virtual bool isReady();

    /// Возвращает true, если тест устройства возвращает результат теста
    virtual bool hasResult();

signals:
    /// Сигнал о получении результатов теста.
    void result(const QString &aTestName, const QVariant &aTestResult);

private slots:
    void onData(const QVariantMap &aData);

private:
    ObjectPointer<SDK::Driver::IHID> m_HID;
    QList<QPair<QString, QString>> m_TestNames;
};

//------------------------------------------------------------------------------

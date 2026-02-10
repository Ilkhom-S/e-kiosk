/* @file Обобщенный тест устройств. */

#pragma once

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFutureWatcher>

#include <Common/ObjectPointer.h>

#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace Driver {
class IDevice;
} // namespace Driver
} // namespace SDK

//------------------------------------------------------------------------------
class GenericDeviceTest : public QObject, public SDK::PaymentProcessor::IDeviceTest {
    Q_OBJECT

public:
    GenericDeviceTest(SDK::Driver::IDevice *aDevice);
    virtual ~GenericDeviceTest();

    /// Возвращает имена и описания тестов.
    virtual QList<QPair<QString, QString>> getTestNames() const;

    virtual bool run(const QString &aTestName);

    virtual void stop();

    virtual bool isReady();

    virtual bool hasResult();

signals:
    /// Сигнал о получении результатов теста.
    void result(const QString &aTestName, const QVariant &aTestResult);

private slots:
    void onTestFinished();

private:
    QFutureWatcher<void> m_Result;
    ObjectPointer<SDK::Driver::IDevice> m_Device;

    const QString m_GenericTest;
};

//------------------------------------------------------------------------------

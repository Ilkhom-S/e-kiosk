/* @file Интерфейс фискального регистратора. */

#pragma once

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFuture>

#include <Common/ObjectPointer.h>

#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace PaymentProcessor {
class ICore;
} // namespace PaymentProcessor
namespace Driver {
class IDevice;
class IPrinter;
} // namespace Driver
} // namespace SDK

//------------------------------------------------------------------------------
class PrinterTest : public SDK::PaymentProcessor::IDeviceTest {
    Q_OBJECT

public:
    PrinterTest(SDK::Driver::IDevice *mDevice, SDK::PaymentProcessor::ICore *aCore);

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

private slots:
    void onPrinted(bool aError);

private:
    ObjectPointer<SDK::Driver::IPrinter> mPrinter;
    SDK::PaymentProcessor::ICore *mCore;
    QFuture<bool> mTestResult;
};

//------------------------------------------------------------------------------

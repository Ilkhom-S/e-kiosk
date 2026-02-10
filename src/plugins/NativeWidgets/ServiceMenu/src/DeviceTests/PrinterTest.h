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
class PrinterTest : public QObject, public SDK::PaymentProcessor::IDeviceTest {
    Q_OBJECT

public:
    PrinterTest(SDK::Driver::IDevice *aDevice, SDK::PaymentProcessor::ICore *aCore);

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

signals:
    void result(const QString &aTestName, const QVariant &aTestResult);

private:
    ObjectPointer<SDK::Driver::IPrinter> m_Printer;
    SDK::PaymentProcessor::ICore *m_Core;
    QFuture<bool> m_TestResult;
};

//------------------------------------------------------------------------------

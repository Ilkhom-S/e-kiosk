/* @file Класс для тестирования диспенсеров. */

#pragma once

#include <QtCore/QFutureWatcher>
#include <QtCore/QStringList>

#include <Common/ObjectPointer.h>

#include <SDK/Drivers/IDispenser.h>
#include <SDK/PaymentProcessor/IDeviceTest.h>

namespace SDK {
namespace Driver {
class IDevice;
} // namespace Driver
namespace PaymentProcessor {
class ICore;
} // namespace PaymentProcessor
} // namespace SDK

//------------------------------------------------------------------------------
class DispenserTest : public QObject, public SDK::PaymentProcessor::IDeviceTest {
    Q_OBJECT

public:
    DispenserTest(SDK::Driver::IDevice *aDevice,
                  QString aConfigurationName,
                  SDK::PaymentProcessor::ICore *aCore);

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
    void onDispensed(int aCashUnit, int aCount);
    void onRejected(int aCashUnit, int aCount);

private:
    ObjectPointer<SDK::Driver::IDispenser> m_Dispenser;
    QString m_ConfigurationName;
    SDK::PaymentProcessor::ICore *m_Core;
    QStringList m_Results;
};

//------------------------------------------------------------------------------

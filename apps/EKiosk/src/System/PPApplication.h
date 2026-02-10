/* @file Класс приложения для PaymentProcessor. */

#pragma once

#include <QtCore/QAbstractNativeEventFilter>
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QObject>
#include <QtCore/QSharedMemory>
#include <QtCore/QTimer>
#include <QtCore/QWaitCondition>
#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>

#include <Common/BasicApplication.h>
#include <Common/SafeApplication.h>

#include "System/IApplication.h"

class ServiceController;
class IServiceManager;

//------------------------------------------------------------------------
/// Класс приложения для PaymentProcessor.
class PPApplication : public QObject,
                      public QAbstractNativeEventFilter,
                      public IApplication,
                      public BasicQtApplication<SafeQApplication> {
    Q_OBJECT

public:
    PPApplication(const QString &aName,
                  const QString &aVersion,
                  int &aArgumentCount,
                  char **aArguments);
    virtual ~PPApplication();

    int exec();

    virtual SDK::PaymentProcessor::ICore *getCore() override;
    virtual QVariantMap getArguments() const override;
    virtual QSettings &getSettings() const override;
    virtual QList<QImage> getScreenshot() override;
    virtual IApplication::AppInfo getAppInfo() const override;
    virtual ILog *getLog() const override;
    virtual QString getUserDataPath() const override;
    virtual QString getPluginPath() const override;
    virtual QString getUserPluginPath() const override;

    static void
    qtMessageHandler(QtMsgType aType, const QMessageLogContext &aContext, const QString &aMessage);

signals:
    void screenshot();

private slots:
    void onScreenshot();
    void exit(int aResultCode);
    void closeBySystem_Request(QSessionManager &aSessionManager);

private:
    /// Обработка системных сообщений, отключение скринсейвера, монитора и т.п.
    virtual bool
    nativeEventFilter(const QByteArray &aEventType, void *aMessage, qintptr *aResult) override;

private:
    ServiceController *m_ServiceController;
    QList<QImage> m_Screenshots;
    QWaitCondition m_ScreenshotCondition;
    QMutex m_ScreenshotMutex;
    QVariantMap m_Arguments;
    QSharedMemory m_Protection;
};

//------------------------------------------------------------------------

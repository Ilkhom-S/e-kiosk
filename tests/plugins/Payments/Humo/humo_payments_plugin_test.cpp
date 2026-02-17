#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtTest/QtTest>

#include <SDK/Plugins/IPluginFactory.h>

#include "../../common/PluginTestBase.h"

namespace {
QString getPluginPath() {
    const QByteArray envPath = qgetenv("HUMO_PAYMENTS_PLUGIN");
    if (!envPath.isEmpty()) {
        return QString::fromUtf8(envPath);
    }

    // Try to resolve via PluginTestBase search first
    const QString resolved = PluginTestBase::findPlugin("humo_payments");
    if (!resolved.isEmpty())
        return resolved;

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString baseDir = QDir(appDir).filePath("../../../../bin/plugins");
    const QString cleanBase = QDir::cleanPath(baseDir);

    const QStringList candidates = {QDir(cleanBase).filePath("libhumo_paymentsd.dylib"),
                                    QDir(cleanBase).filePath("libhumo_payments.dylib"),
                                    QDir(cleanBase).filePath("libhumo_payments.so"),
                                    QDir(cleanBase).filePath("humo_paymentsd.dll"),
                                    QDir(cleanBase).filePath("humo_payments.dll")};

    for (const QString &path : candidates) {
        if (QFile::exists(path)) {
            return path;
        }
    }

    return {};
}
} // namespace

class HumoPaymentsPluginTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginExists();
    void testPluginLoading();
    void testPluginCreation();
};

void HumoPaymentsPluginTest::testPluginExists() {
    const QString pluginPath = getPluginPath();
    QVERIFY2(!pluginPath.isEmpty(), "Plugin path not found. Set HUMO_PAYMENTS_PLUGIN env var.");
    QVERIFY(QFile::exists(pluginPath));
}

void HumoPaymentsPluginTest::testPluginLoading() {
    const QString pluginPath = getPluginPath();
    QVERIFY2(!pluginPath.isEmpty(), "Plugin path not found. Set HUMO_PAYMENTS_PLUGIN env var.");

    PluginTestBase testBase(pluginPath);
    SDK::Plugin::IPluginFactory *factory = testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);
}

void HumoPaymentsPluginTest::testPluginCreation() {
    const QString pluginPath = getPluginPath();
    QVERIFY2(!pluginPath.isEmpty(), "Plugin path not found. Set HUMO_PAYMENTS_PLUGIN env var.");

    PluginTestBase testBase(pluginPath);
    SDK::Plugin::IPluginFactory *factory = testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    const QStringList plugins = factory->getPluginList();
    QVERIFY(!plugins.isEmpty());

    const QString &instancePath = plugins.first();
    SDK::Plugin::IPlugin *plugin = factory->createPlugin(instancePath, instancePath);
    QVERIFY(plugin != nullptr);

    factory->destroyPlugin(plugin);
}

QTEST_MAIN(HumoPaymentsPluginTest)
#include "humo_payments_plugin_test.moc"

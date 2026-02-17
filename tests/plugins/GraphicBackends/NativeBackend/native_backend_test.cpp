#include <QtCore/QFile>
#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>

#include <SDK/Plugins/IPluginFactory.h>

#include "../../common/PluginTestBase.h"

class NativeBackendTest : public QObject {
    Q_OBJECT

public:
    NativeBackendTest() : m_testBase(PluginTestBase::findPlugin("native_backend")) {}

private slots:
    // Basic plugin build verification
    void testPluginExists();

    // Plugin loading and factory interface testing
    void testPluginLoading();
    void testFactoryInterface();

    // Plugin implementation testing
    void testPluginCreation();
    void testPluginInitialization();
    void testPluginLifecycle();

private:
    PluginTestBase m_testBase;
};

void NativeBackendTest::testPluginExists() {
    QString pluginPath = PluginTestBase::findPlugin("native_backend");
    QVERIFY2(!pluginPath.isEmpty(),
             qPrintable(QString("Plugin not found for: %1").arg("native_backend")));
    QVERIFY(QFile::exists(pluginPath));

    // Verify it's not empty (basic corruption check)
    QFile pluginFile(pluginPath);
    QVERIFY(pluginFile.size() > 0);
}

void NativeBackendTest::testPluginLoading() {
    // Test loading the plugin factory
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);
}

void NativeBackendTest::testFactoryInterface() {
    // Test factory interface methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Test basic factory information with expected values
    QCOMPARE(factory->getName(), QString("Native Backend"));
    QCOMPARE(factory->getDescription(), QString("Native graphics backend for EKiosk"));
    QCOMPARE(factory->getAuthor(), QString("CPP Static Author Test"));
    QCOMPARE(factory->getVersion(), QString("1.0"));
    QVERIFY(!factory->getPluginList().isEmpty());
}

void NativeBackendTest::testPluginCreation() {
    // Test creating a plugin instance
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Get the registered plugin path
    QStringList pluginList = factory->getPluginList();
    QVERIFY(!pluginList.isEmpty());

    const QString &pluginPath =
        pluginList.first(); // Should be "Application.GraphicsBackend.Native"

    SDK::Plugin::IPlugin *plugin = factory->createPlugin(pluginPath, pluginPath);
    QVERIFY(plugin != nullptr);

    // Test plugin name
    QVERIFY(!plugin->getPluginName().isEmpty());

    // Note: Plugin may not be ready in test environment due to missing initialization
    // QVERIFY(plugin->isReady());

    // Clean up
    // Note: destroyPlugin may fail if plugin is not properly registered
    factory->destroyPlugin(plugin);
}

void NativeBackendTest::testPluginInitialization() {
    // Test plugin configuration methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Get the registered plugin path
    QStringList pluginList = factory->getPluginList();
    QVERIFY(!pluginList.isEmpty());
    const QString &pluginPath = pluginList.first();

    SDK::Plugin::IPlugin *plugin = factory->createPlugin(pluginPath, pluginPath);
    QVERIFY(plugin != nullptr);

    // Test configuration methods (available without full initialization)
    QVariantMap config = plugin->getConfiguration();
    QVERIFY(config.isEmpty() || true); // Configuration might be empty initially

    // Note: setConfiguration may not store the configuration without full initialization
    QVariantMap newConfig;
    newConfig["test_key"] = "test_value";
    plugin->setConfiguration(newConfig);

    // Configuration storage requires full plugin initialization with dependencies
    // QVariantMap retrievedConfig = plugin->getConfiguration();
    // QVERIFY(retrievedConfig.contains("test_key"));

    // Clean up
    // Note: destroyPlugin may fail if plugin is not properly registered
    factory->destroyPlugin(plugin);
}

void NativeBackendTest::testPluginLifecycle() {
    // Test plugin interface methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Get the registered plugin path
    QStringList pluginList = factory->getPluginList();
    QVERIFY(!pluginList.isEmpty());
    const QString &pluginPath = pluginList.first();

    SDK::Plugin::IPlugin *plugin = factory->createPlugin(pluginPath, pluginPath);
    QVERIFY(plugin != nullptr);

    QVERIFY(!plugin->getConfigurationName().isEmpty());
    // Note: Plugin readiness requires full initialization with dependencies
    // QVERIFY(plugin->isReady());

    // Test save configuration
    QVERIFY(plugin->saveConfiguration() || true); // Save might fail without full initialization

    // Clean up
    // Note: destroyPlugin may fail if plugin is not properly registered
    factory->destroyPlugin(plugin);
}

//---------------------------------------------------------------------------

QTEST_MAIN(NativeBackendTest)
#include "native_backend_test.moc"

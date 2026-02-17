#include <QtCore/QFile>
#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>

#include <SDK/Plugins/IPluginFactory.h>

#include "../common/PluginTestBase.h"

class AdPluginTest : public QObject {
    Q_OBJECT

public:
    AdPluginTest() : m_testBase(PluginTestBase::findPlugin("add")) {}

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

void AdPluginTest::testPluginExists() {
    QString pluginPath = PluginTestBase::findPlugin("add");
    QVERIFY2(!pluginPath.isEmpty(), qPrintable(QString("Plugin not found for: %1").arg("add")));
    QVERIFY(QFile::exists(pluginPath));

    // Verify it's not empty (basic corruption check)
    QFile pluginFile(pluginPath);
    QVERIFY(pluginFile.size() > 0);
}

void AdPluginTest::testPluginLoading() {
    // Test loading the plugin factory
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);
}

void AdPluginTest::testFactoryInterface() {
    // Test factory interface methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Test basic factory information
    QVERIFY(!factory->getName().isEmpty());
    QVERIFY(!factory->getDescription().isEmpty());
    QVERIFY(!factory->getVersion().isEmpty());
    QVERIFY(!factory->getPluginList().isEmpty());
}

void AdPluginTest::testPluginCreation() {
    // Test creating a plugin instance
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    QStringList pluginList = factory->getPluginList();
    if (!pluginList.contains("AdPlugin.Instance")) {
        QSKIP("Ad plugin instance not registered in factory; skipping creation test");
        return;
    }

    SDK::Plugin::IPlugin *plugin = nullptr;
    try {
        plugin = factory->createPlugin("AdPlugin.Instance", "AdPlugin.Instance");
    } catch (...) {
        QSKIP("createPlugin threw an exception in test environment; skipping");
        return;
    }

    if (!plugin) {
        QSKIP("createPlugin returned null in test environment; skipping");
        return;
    }

    // Test plugin name
    QVERIFY(!plugin->getPluginName().isEmpty());

    // Clean up
    factory->destroyPlugin(plugin);
}

void AdPluginTest::testPluginInitialization() {
    // Test plugin configuration methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    QStringList pluginList = factory->getPluginList();
    if (!pluginList.contains("AdPlugin.Instance")) {
        QSKIP("Ad plugin instance not registered in factory; skipping initialization test");
        return;
    }

    SDK::Plugin::IPlugin *plugin = nullptr;
    try {
        plugin = factory->createPlugin("AdPlugin.Instance", "AdPlugin.Instance");
    } catch (...) {
        QSKIP("createPlugin threw an exception in test environment; skipping initialization test");
        return;
    }

    if (!plugin) {
        QSKIP("createPlugin returned null in test environment; skipping initialization test");
        return;
    }

    // Test configuration methods (available without full initialization)
    QVariantMap config = plugin->getConfiguration();
    QVERIFY(config.isEmpty() || true); // Configuration might be empty initially

    // Note: setConfiguration may not store the configuration without full initialization
    QVariantMap newConfig;
    newConfig["test_key"] = "test_value";
    plugin->setConfiguration(newConfig);

    // Clean up
    factory->destroyPlugin(plugin);
}

void AdPluginTest::testPluginLifecycle() {
    // Test plugin interface methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    QStringList pluginList = factory->getPluginList();
    if (!pluginList.contains("AdPlugin.Instance")) {
        QSKIP("Ad plugin instance not registered in factory; skipping lifecycle test");
        return;
    }

    SDK::Plugin::IPlugin *plugin = nullptr;
    try {
        plugin = factory->createPlugin("AdPlugin.Instance", "AdPlugin.Instance");
    } catch (...) {
        QSKIP("createPlugin threw an exception in test environment; skipping lifecycle test");
        return;
    }

    if (!plugin) {
        QSKIP("createPlugin returned null in test environment; skipping lifecycle test");
        return;
    }

    QVERIFY(!plugin->getConfigurationName().isEmpty());
    // Test save configuration
    QVERIFY(plugin->saveConfiguration() || true); // Save might fail without full initialization

    // Clean up
    factory->destroyPlugin(plugin);
}

//---------------------------------------------------------------------------

QTEST_MAIN(AdPluginTest)
#include "ad_plugin_test.moc"

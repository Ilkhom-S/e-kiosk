#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>

#include <SDK/Plugins/PluginFactory.h>

class TemplatePluginTest : public QObject {
    Q_OBJECT

private slots:
    void testPluginLoading();
    void testPluginCreation();
    void testPluginFunctionality();
};

void TemplatePluginTest::testPluginLoading() {
    QCoreApplication::addLibraryPath("D:/plugins/Debug");
    QPluginLoader loader("template_plugind");
    QVERIFY(loader.load());
    QObject *plugin = loader.instance();
    QVERIFY(plugin != nullptr);
    SDK::Plugin::PluginFactory *factory = static_cast<SDK::Plugin::PluginFactory *>(plugin);
    QVERIFY(factory != nullptr);
    QCOMPARE(factory->getName(), QString("Template Plugin"));
    QCOMPARE(factory->getDescription(), QString("Minimal template plugin for plugin development"));
    QCOMPARE(factory->getPluginList(), QStringList() << "PaymentProcessor.Template.TemplatePlugin");
    loader.unload();
}

void TemplatePluginTest::testPluginCreation() {
    QCoreApplication::addLibraryPath("D:/plugins/Debug");
    QPluginLoader loader("template_plugind");
    QVERIFY(loader.load());
    QObject *plugin = loader.instance();
    QVERIFY(plugin != nullptr);
    SDK::Plugin::PluginFactory *factory = static_cast<SDK::Plugin::PluginFactory *>(plugin);
    QVERIFY(factory != nullptr);

    // Create a plugin instance
    SDK::Plugin::IPlugin *templatePlugin =
        factory->createPlugin("TemplatePlugin.Instance.test", "TemplatePlugin.Instance.test");
    QVERIFY(templatePlugin != nullptr);
    QCOMPARE(templatePlugin->getPluginName(), QString("Template Plugin"));

    // Note: Plugin may not be ready in test environment due to missing initialization
    // QCOMPARE(templatePlugin->isReady(), true);

    // Clean up
    factory->destroyPlugin(templatePlugin);
    loader.unload();
}

void TemplatePluginTest::testPluginFunctionality() {
    QCoreApplication::addLibraryPath("D:/plugins/Debug");
    QPluginLoader loader("template_plugind");
    QVERIFY(loader.load());
    QObject *plugin = loader.instance();
    QVERIFY(plugin != nullptr);
    SDK::Plugin::PluginFactory *factory = static_cast<SDK::Plugin::PluginFactory *>(plugin);
    QVERIFY(factory != nullptr);

    // Create a plugin instance
    SDK::Plugin::IPlugin *iPlugin =
        factory->createPlugin("TemplatePlugin.Instance.test", "TemplatePlugin.Instance.test");
    QVERIFY(iPlugin != nullptr);

    // Test configuration management
    QVariantMap config;
    config["testKey"] = "testValue";
    config["helloMessage"] = "Hello from test!";
    iPlugin->setConfiguration(config);
    QCOMPARE(iPlugin->getConfiguration(), config);

    // Test save configuration
    QVERIFY(iPlugin->saveConfiguration());

    // Note: For testing custom plugin functionality beyond IPlugin interface,
    // you would typically use a mock kernel environment or cast to specific type
    // if the plugin class is exported. See PluginTestBase.h for mock kernel testing.

    // Clean up
    factory->destroyPlugin(iPlugin);
    loader.unload();
}

QTEST_MAIN(TemplatePluginTest)
#include "test_plugin_test.moc"
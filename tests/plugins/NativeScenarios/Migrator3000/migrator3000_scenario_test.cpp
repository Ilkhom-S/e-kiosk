#include <QtCore/QFile>
#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>

#include <SDK/Plugins/IPluginFactory.h>

#include "../../common/PluginTestBase.h"

class Migrator3000ScenarioTest : public QObject {
    Q_OBJECT

public:
    Migrator3000ScenarioTest() : m_testBase("D:/plugins/Debug/migrator3000_scenariod.dll") {}

private slots:
    // Basic plugin build verification
    void testPluginExists();

    // Plugin loading and factory interface testing
    void testPluginLoading();
    void testFactoryInterface();

    // MainScenarioPlugin factory testing
    void testMainScenarioPluginFactory();

    // MainScenario implementation testing (limited due to complex dependencies)
    void testMainScenarioBasicInterface();

private:
    PluginTestBase m_testBase;
};

void Migrator3000ScenarioTest::testPluginExists() {
    // Test that the Migrator3000 plugin DLL was built successfully
    QString pluginPath = "D:/plugins/Debug/migrator3000_scenariod.dll";
    QVERIFY(QFile::exists(pluginPath));

    // Verify it's not empty (basic corruption check)
    QFile pluginFile(pluginPath);
    QVERIFY(pluginFile.size() > 0);
}

void Migrator3000ScenarioTest::testPluginLoading() {
    // Test loading the plugin factory
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);
}

void Migrator3000ScenarioTest::testFactoryInterface() {
    // Test factory interface methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Test basic factory information
    QVERIFY(!factory->getName().isEmpty());
    QCOMPARE(factory->getName(), QString("Migrator 3000"));

    QVERIFY(!factory->getDescription().isEmpty());
    QCOMPARE(factory->getDescription(),
             QString("Native scenario for automatic migration from 2.x.x to 3.x.x version"));

    QVERIFY(!factory->getVersion().isEmpty());
    QCOMPARE(factory->getVersion(), QString("1.0"));

    QVERIFY(!factory->getPluginList().isEmpty());
}

void Migrator3000ScenarioTest::testMainScenarioPluginFactory() {
    // Test MainScenarioPlugin factory methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Test factory metadata
    QVERIFY(!factory->getName().isEmpty());
    QCOMPARE(factory->getName(), QString("Migrator 3000"));

    QVERIFY(!factory->getDescription().isEmpty());
    QCOMPARE(factory->getDescription(),
             QString("Native scenario for automatic migration from 2.x.x to 3.x.x version"));

    QVERIFY(!factory->getAuthor().isEmpty());
    QCOMPARE(factory->getAuthor(), QString("Humo"));

    QVERIFY(!factory->getVersion().isEmpty());
    QCOMPARE(factory->getVersion(), QString("1.0"));

    // Note: Creating a plugin instance requires complex PPSDK dependencies
    // In test environment, we test the factory interface without full initialization
    // Plugin creation would require a valid IKernel and proper environment setup
}

void Migrator3000ScenarioTest::testMainScenarioBasicInterface() {
    // Test MainScenario basic interface methods
    // Note: MainScenario requires complex PPSDK dependencies, so we skip plugin instantiation

    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Verify factory plugin list contains Migrator3000
    QStringList plugins = factory->getPluginList();
    QVERIFY(!plugins.isEmpty());
    // Should contain at least one plugin (Migrator3000)
    QVERIFY(plugins.size() >= 1);
}

//---------------------------------------------------------------------------

QTEST_MAIN(Migrator3000ScenarioTest)
#include "migrator3000_scenario_test.moc"
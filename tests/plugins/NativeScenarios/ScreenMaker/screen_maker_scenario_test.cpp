// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QFile>
#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPluginFactory.h>

// System
#include "../../common/PluginTestBase.h"

class ScreenMakerScenarioTest : public QObject {
    Q_OBJECT

  public:
    ScreenMakerScenarioTest() : m_testBase("D:/plugins/Debug/screen_maker_scenariod.dll") {
    }

  private slots:
    void testPluginExists();
    void testPluginLoading();
    void testFactoryInterface();
    void testMainScenarioPluginFactory();
    void testMainScenarioBasicInterface();

  private:
    PluginTestBase m_testBase;
};

//---------------------------------------------------------------------------
void ScreenMakerScenarioTest::testPluginExists() {
    // Verify that the ScreenMaker plugin DLL exists at expected location
    QString pluginPath = "D:/plugins/Debug/screen_maker_scenariod.dll";
    QVERIFY2(QFile::exists(pluginPath), qPrintable(QString("Plugin DLL not found at: %1").arg(pluginPath)));

    // Verify it's not empty (basic corruption check)
    QFile pluginFile(pluginPath);
    QVERIFY(pluginFile.size() > 0);
}

//---------------------------------------------------------------------------
void ScreenMakerScenarioTest::testPluginLoading() {
    // Load the plugin factory using PluginTestBase
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY2(factory != nullptr, "Failed to load plugin factory");
}

//---------------------------------------------------------------------------
void ScreenMakerScenarioTest::testFactoryInterface() {
    // Test basic factory interface methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Verify plugin metadata
    QCOMPARE(factory->getName(), QString("Screenshot maker"));
    QVERIFY(!factory->getDescription().isEmpty());
    QCOMPARE(factory->getAuthor(), QString("Humo"));
    QCOMPARE(factory->getVersion(), QString("1.0"));

    // Verify plugin list
    QStringList pluginList = factory->getPluginList();
    QVERIFY(pluginList.count() >= 1);
}

//---------------------------------------------------------------------------
void ScreenMakerScenarioTest::testMainScenarioPluginFactory() {
    // Test plugin factory
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Verify factory has correct name
    QCOMPARE(factory->getName(), QString("Screenshot maker"));
}

//---------------------------------------------------------------------------
void ScreenMakerScenarioTest::testMainScenarioBasicInterface() {
    // Test factory returns valid plugin list
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Plugin factory should have at least one item in plugin list
    QStringList pluginList = factory->getPluginList();
    QVERIFY(pluginList.count() >= 1);
}

//---------------------------------------------------------------------------

QTEST_MAIN(ScreenMakerScenarioTest)
#include "screen_maker_scenario_test.moc"

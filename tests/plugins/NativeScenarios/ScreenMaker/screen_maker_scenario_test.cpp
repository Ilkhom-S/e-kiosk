// Qt
#include <Common/QtHeadersBegin.h>
#include <QCoreApplication>
#include <QTest>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/IPluginFactory.h>

// System
#include "../common/PluginTestBase.h"

class ScreenMakerScenarioTest : public QObject {
    Q_OBJECT

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
    QString pluginPath = m_testBase.getPluginPath("screen_maker_scenario");
    QVERIFY2(QFile::exists(pluginPath),
             qPrintable(QString("Plugin DLL not found at: %1").arg(pluginPath)));
}

//---------------------------------------------------------------------------
void ScreenMakerScenarioTest::testPluginLoading() {
    // Load the plugin factory using PluginTestBase
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory("screen_maker_scenario");
    QVERIFY2(factory != nullptr, "Failed to load plugin factory");
}

//---------------------------------------------------------------------------
void ScreenMakerScenarioTest::testFactoryInterface() {
    // Test basic factory interface methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory("screen_maker_scenario");
    QVERIFY(factory != nullptr);

    // Verify plugin metadata
    QCOMPARE(factory->getName(), QString("Screenshot maker"));
    QVERIFY(!factory->getDescription().isEmpty());
    QCOMPARE(factory->getAuthor(), QString("Humo"));
    QCOMPARE(factory->getVersion(), QString("1.0"));
    QCOMPARE(factory->getModuleName(), QString("screen_maker"));

    // Verify plugin list
    QStringList pluginList = factory->getPluginList();
    QVERIFY(pluginList.count() >= 1);
}

//---------------------------------------------------------------------------
void ScreenMakerScenarioTest::testMainScenarioPluginFactory() {
    // Test plugin factory creation (avoid complex dependencies by not creating plugin)
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory("screen_maker_scenario");
    QVERIFY(factory != nullptr);

    // Verify factory has correct module name
    QCOMPARE(factory->getModuleName(), QString("screen_maker"));

    // Verify factory is ready
    QVERIFY(factory != nullptr);
}

//---------------------------------------------------------------------------
void ScreenMakerScenarioTest::testMainScenarioBasicInterface() {
    // Test that plugin list contains ScreenMaker
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory("screen_maker_scenario");
    QVERIFY(factory != nullptr);

    QStringList pluginList = factory->getPluginList();
    QVERIFY(pluginList.contains("ScreenMaker"));
}

//---------------------------------------------------------------------------

QTEST_MAIN(ScreenMakerScenarioTest)
#include "screen_maker_scenario_test.moc"

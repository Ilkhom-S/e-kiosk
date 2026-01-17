// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

class TestPluginTest : public QObject {
    Q_OBJECT

  private slots:
    void testPluginLoading();
};

void TestPluginTest::testPluginLoading() {
    QCoreApplication::addLibraryPath("D:/plugins/Debug");
    QPluginLoader loader("test_plugind");
    QVERIFY(loader.load());
    QObject *plugin = loader.instance();
    QVERIFY(plugin != nullptr);
    SDK::Plugin::PluginFactory *factory = static_cast<SDK::Plugin::PluginFactory *>(plugin);
    QVERIFY(factory != nullptr);
    QCOMPARE(factory->getName(), QString("Test Plugin (Overridden)"));
    QCOMPARE(factory->getDescription(), QString("Minimal test plugin for verifying plugin system (Overridden)"));
    loader.unload();
}

QTEST_MAIN(TestPluginTest)
#include "test_plugin_test.moc"
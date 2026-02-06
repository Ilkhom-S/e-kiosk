#include <QtCore/QFile>
#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>

#include <SDK/Plugins/IPluginFactory.h>

#include "../common/PluginTestBase.h"

class QMLBackendTest : public QObject {
    Q_OBJECT

public:
    QMLBackendTest() : m_testBase("D:/plugins/Debug/qml_backendd.dll") {}

private slots:
    // Basic plugin build verification
    void testPluginExists();

    // Plugin loading and factory interface testing
    void testPluginLoading();
    void testFactoryInterface();

private:
    PluginTestBase m_testBase;
};

void QMLBackendTest::testPluginExists() {
    // Test that the QML Backend plugin DLL was built successfully
    QString pluginPath = "D:/plugins/Debug/qml_backendd.dll";
    QVERIFY(QFile::exists(pluginPath));
    qDebug() << "QML Backend plugin DLL exists at:" << pluginPath;

    // Verify it's not empty (basic corruption check)
    QFile pluginFile(pluginPath);
    QVERIFY(pluginFile.size() > 0);
    qDebug() << "Plugin DLL size:" << pluginFile.size() << "bytes";
}

void QMLBackendTest::testPluginLoading() {
    // Test loading the plugin factory
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);
    qDebug() << "Plugin factory loaded and initialized successfully";
}

void QMLBackendTest::testFactoryInterface() {
    // Test the factory interface methods
    SDK::Plugin::IPluginFactory *factory = m_testBase.loadPluginFactory();
    QVERIFY(factory != nullptr);

    // Test basic factory methods
    QString name = factory->getName();
    QVERIFY(!name.isEmpty());
    QCOMPARE(name, QString("QML graphics backend"));
    qDebug() << "Factory name:" << name;

    QString description = factory->getDescription();
    QVERIFY(!description.isEmpty());
    QCOMPARE(description, QString("QML based graphics backend for qml widgets"));
    qDebug() << "Factory description:" << description;

    QString author = factory->getAuthor();
    QVERIFY(!author.isEmpty());
    QCOMPARE(author, QString("Humo"));
    qDebug() << "Factory author:" << author;

    QString version = factory->getVersion();
    QVERIFY(!version.isEmpty());
    QCOMPARE(version, QString("1.0"));
    qDebug() << "Factory version:" << version;

    // Test plugin list
    QStringList plugins = factory->getPluginList();
    QVERIFY(!plugins.isEmpty());
    qDebug() << "Available plugins in factory:" << plugins;

    // Should contain the QML backend plugin
    bool hasQMLBackend = false;
    foreach (QString plugin, plugins) {
        if (plugin.contains("QML", Qt::CaseInsensitive) ||
            plugin.contains("GraphicsBackend", Qt::CaseInsensitive)) {
            hasQMLBackend = true;
            qDebug() << "Found QML Graphics Backend plugin:" << plugin;
            break;
        }
    }
    QVERIFY(hasQMLBackend);
}

QTEST_MAIN(QMLBackendTest)
#include "qml_backend_test.moc"
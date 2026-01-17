// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/Plugins/PluginFactory.h>

class QMLBackendTest : public QObject {
    Q_OBJECT

  private slots:
    void testPluginLoading();
};

void QMLBackendTest::testPluginLoading() {
    qDebug() << "Starting test";
    QString pluginPath = "D:/plugins/Debug";
    QCoreApplication::addLibraryPath(pluginPath);
    qDebug() << "Library paths:" << QCoreApplication::libraryPaths();
    QPluginLoader loader("qml_backendd");
    qDebug() << "Plugin file:" << loader.fileName();
    qDebug() << "Loading plugin...";
    bool loaded = loader.load();
    qDebug() << "Load result:" << loaded;
    if (!loaded) {
        qDebug() << "Error:" << loader.errorString();
        QFAIL("Plugin failed to load");
    }
    QObject *plugin = loader.instance();
    QVERIFY(plugin != nullptr);
    SDK::Plugin::PluginFactory *factory = static_cast<SDK::Plugin::PluginFactory *>(plugin);
    QVERIFY(factory != nullptr);
    QCOMPARE(factory->getName(), QString("QML graphics backend"));
    QCOMPARE(factory->getDescription(), QString("QML based graphics backend for qml widgets"));
    qDebug() << "QMLBackend plugin loaded successfully";
    loader.unload();
}

QTEST_MAIN(QMLBackendTest)
#include "qml_backend_test.moc"
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>

#include <SettingsManager/SettingsManager.h>

class TestSettingsManager : public QObject {
    Q_OBJECT

private slots:
    void testLoadXML();
    void testSaveXML();
};

void TestSettingsManager::testLoadXML() {
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());

    QString path = tmp.path() + "/cfg.xml";
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QByteArray xml = R"(<?xml version="1.0"?>
<settings>
  <field1>value1</field1>
  <field2>42</field2>
</settings>
)";
    QVERIFY(f.write(xml) == xml.size());
    f.close();

    SettingsManager mgr(tmp.path());
    QList<SSettingsSource> sources;
    sources.append(SSettingsSource(path, "adapter", true));

    QVERIFY(mgr.loadSettings(sources));

    TPtree &props = mgr.getProperties("adapter");
    auto it = props.find("settings");
    QVERIFY(it != props.not_found());

    TPtree &settings = props.to_iterator(it)->second;
    auto it2 = settings.find("field1");
    QVERIFY(it2 != settings.not_found());

    QString val = settings.to_iterator(it2)->second.get_value<QString>();
    QCOMPARE(val, QString("value1"));
}

void TestSettingsManager::testSaveXML() {
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());

    QString path = tmp.path() + "/cfg.xml";
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    QByteArray xml = R"(<?xml version="1.0"?>
<settings>
  <field1>old</field1>
</settings>
)";
    QVERIFY(f.write(xml) == xml.size());
    f.close();

    SettingsManager mgr(tmp.path());
    QList<SSettingsSource> sources;
    // mark as writable so saveSettings will attempt to persist changes
    sources.append(SSettingsSource(path, "adapter", false));

    QVERIFY(mgr.loadSettings(sources));

    TPtree &props = mgr.getProperties("adapter");
    auto it = props.find("settings");
    QVERIFY(it != props.not_found());

    TPtree &settings = props.to_iterator(it)->second;
    settings.put("field1", QString("newval").toStdWString());

    QVERIFY(mgr.saveSettings());

    // verify file contains updated value
    QFile rf(path);
    QVERIFY(rf.open(QIODevice::ReadOnly));
    QByteArray content = rf.readAll();
    rf.close();

    QVERIFY(content.contains("newval"));

    // verify backup exists (has suffix "_backup")
    QDir d = QFileInfo(path).dir();
    QStringList list =
        d.entryList(QStringList() << (QFileInfo(path).baseName() + ".*_backup"), QDir::Files);
    QVERIFY(!list.isEmpty());
}

QTEST_MAIN(TestSettingsManager)
#include "TestSettingsManager.moc"

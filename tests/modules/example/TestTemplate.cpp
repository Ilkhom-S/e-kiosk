#include <QtTest/QtTest>

#include <Common/ILog.h>

class TestTemplate : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() { /* setup */ }
    void cleanupTestCase() { /* teardown */ }
    void testBasic() { QVERIFY(true); }
};

QTEST_MAIN(TestTemplate)
#include "TestTemplate.moc"

//---------------------------------------------------------------------------
// EKiosk Example Test
//---------------------------------------------------------------------------

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

class TestExample : public QObject {
  Q_OBJECT
private slots:
  void testBasicCompare() {
    int expected = 42;
    int actual = 42;
    QCOMPARE(actual, expected);
  }
  void testString() {
    QString str = "Hello, World!";
    QVERIFY(!str.isEmpty());
    QVERIFY(str.contains("World"));
  }
};

QTEST_MAIN(TestExample)
#include "TestExample.moc"

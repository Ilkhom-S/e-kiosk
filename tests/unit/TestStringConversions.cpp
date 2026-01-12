// STL
#include <string>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

class TestStringConversions : public QObject {
  Q_OBJECT

private slots:
  void qtStringToWCharBuffer() {
    QString s = "ConnectionName";
    wchar_t buf[260];
    std::wstring w = s.toStdWString();
    int res = wcscpy_s(buf, sizeof(buf) / sizeof(wchar_t), w.c_str());
    QVERIFY(res == 0);
    QCOMPARE(QString::fromWCharArray(buf), s);
  }

  void utf16PointerMatches() {
    QString s = "SeShutdownPrivilege";
    const wchar_t *p = reinterpret_cast<const wchar_t *>(s.utf16());
    QCOMPARE(QString::fromWCharArray(p), s);
  }
};

QTEST_MAIN(TestStringConversions)
#include "TestStringConversions.moc"

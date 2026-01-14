// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

class TestStringConversions : public QObject {
    Q_OBJECT

  private slots:
    void initTestCase() {
    }
    void qtStringToWCharBuffer() {
        QString s = "hello";
        QVector<wchar_t> buf(s.size() + 1);
        const ushort *utf16 = s.utf16();
        for (int i = 0; i <= s.size(); ++i)
            buf[i] = (wchar_t)utf16[i];
        QCOMPARE(QString::fromWCharArray(buf.data()), s);
    }
    void utf16PointerMatches() {
        QString s = "привет"; // contains non-ASCII
        const ushort *p = s.utf16();
        QCOMPARE(QString::fromUtf16(p), s);
    }
    void cleanupTestCase() {
    }
};

QTEST_MAIN(TestStringConversions)
#include "TestStringConversions.moc"

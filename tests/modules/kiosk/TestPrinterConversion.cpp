// Platform
#include <windows.h>

// STL
#include <string>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <QtTest/QtTest>
#include <Common/QtHeadersEnd.h>

class TestPrinterConversion : public QObject {
  Q_OBJECT

private slots:
  void QStringToStdWStringAndLPWSTR() {
    QString name = "\\\\MyPrinter\\USB";
    std::wstring w = name.toStdWString();
    LPWSTR p = const_cast<LPWSTR>(w.c_str());
    QCOMPARE(QString::fromWCharArray(p), name);
  }
};

QTEST_MAIN(TestPrinterConversion)
#include "TestPrinterConversion.moc"

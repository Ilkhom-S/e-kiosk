// Qt
#include <Common/QtHeadersBegin.h>
#include <Common/QtHeadersEnd.h>
#include <QtCore/QString>

#include <QtTest/QtTest>

#include <string>
#include <windows.h>

class TestPrinterConversion : public QObject
{
	Q_OBJECT

private slots:
	void QStringToStdWStringAndLPWSTR()
	{
		QString name = "\\\\MyPrinter\\USB";
		std::wstring w = name.toStdWString();
		// simulate how code creates LPWSTR from std::wstring
		LPWSTR p = const_cast<LPWSTR>(w.c_str());
		QCOMPARE(QString::fromWCharArray(p), name);
	}
};

QTEST_MAIN(TestPrinterConversion)
#include "TestPrinterConversion.moc"

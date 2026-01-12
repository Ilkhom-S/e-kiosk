// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

#include <QtTest/QtTest>

class TestConnectPrivilege : public QObject
{
	Q_OBJECT

private slots:
	void privilegeUtf16()
	{
		QString privilege = "SeShutdownPrivilege";
		const wchar_t *p = reinterpret_cast<const wchar_t *>(privilege.utf16());
		QCOMPARE(QString::fromWCharArray(p), privilege);
	}
};

QTEST_MAIN(TestConnectPrivilege)
#include "TestConnectPrivilege.moc"

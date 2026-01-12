#include <QtCore/QString>

#include <QtTest/QtTest>

class TestConnectPrivilege : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase()
	{
	}
	void privilegeUtf16()
	{
		QString priv = "SeShutdownPrivilege";
		const ushort *u = priv.utf16();
		QVERIFY(u != nullptr);
		QCOMPARE(QString::fromUtf16(u), priv);
	}
	void cleanupTestCase()
	{
	}
};

QTEST_MAIN(TestConnectPrivilege)
#include "TestConnectPrivilege.moc"

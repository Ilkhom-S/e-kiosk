// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// System
#include <SysUtils/ISysUtils.h>

qlonglong ISysUtils::verifyTrust(const QString &aFile) {
#if defined(__APPLE__)
	// Use codesign to check signature
	QString cmd = QString("codesign -v --verify '%1' 2>&1").arg(aFile);
	int ret = system(cmd.toUtf8().constData());
	return (ret == 0) ? 0 : -1;
#elif defined(__linux__)
	// Use openssl to check signature (stub: always trusted)
	Q_UNUSED(aFile);
	return 0;
#else
	Q_UNUSED(aFile);
	return -1;
#endif
}

//---------------------------------------------------------------------------
// Получить из сертификата информацию о подписчике (Linux: openssl, macOS: codesign)
bool ISysUtils::getSignerInfo(const QString &aFile, SSignerInfo &aSigner) {
#if defined(__APPLE__)
	// Use codesign to get signer info
	QString cmd = QString("codesign -d --verbose=4 '%1' 2>&1").arg(aFile);
	FILE *fp = popen(cmd.toUtf8().constData(), "r");
	if (!fp) return false;
	char buffer[512];
	QString output;
	while (fgets(buffer, sizeof(buffer), fp)) {
		output += QString::fromLocal8Bit(buffer);
	}
	pclose(fp);
	QRegExp nameRx("Authority=(.*)");
	if (nameRx.indexIn(output) != -1) {
		aSigner.name = nameRx.cap(1).trimmed();
		return true;
	}
	return false;
#elif defined(__linux__)
	// Use openssl to get signer info (stub: not implemented)
	Q_UNUSED(aFile); aSigner.clear();
	return false;
#else
	Q_UNUSED(aFile); aSigner.clear();
	return false;
#endif
}




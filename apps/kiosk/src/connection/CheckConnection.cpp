#include "CheckConnection.h"

#include <Common/QtHeadersBegin.h>
#include <Common/QtHeadersEnd.h>

#include <QtCore/QJsonDocument>

CheckConnection::CheckConnection(QObject *parent) : QThread(parent)
{
	m_pTcpSocket = new QTcpSocket(this);
	connect(m_pTcpSocket, SIGNAL(connected()), SLOT(slotConnectedOk()));
	connect(m_pTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
			SLOT(slotErrorConnected(QAbstractSocket::SocketError)));

	manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
	connect(manager, SIGNAL(sslErrors(QNetworkReply *, QList<QSslError>)), this,
			SLOT(handleSslErrors(QNetworkReply *, QList<QSslError>)));

	abortTimer = new QTimer(this);
	abortTimer->setSingleShot(true);
	connect(abortTimer, SIGNAL(timeout()), this, SLOT(timeOutDisconect()));

	stsTimer = new QTimer(this);
	stsTimer->setSingleShot(true);
	connect(stsTimer, SIGNAL(timeout()), this, SLOT(timeOutDisconect()));
}

void CheckConnection::timeOutDisconect()
{
	emit this->emit_Ping(false);
}

void CheckConnection::handleSslErrors(QNetworkReply *reply, QList<QSslError> list)
{
	Q_UNUSED(reply)

	QString d = "";
	for (auto &e : list)
	{
		d += e.errorString() + "\n";
	}

	emit emit_toLoging(1, "PING", QString("Description: %1").arg(d));
}

void CheckConnection::slotConnectedOk()
{
	if (stsTimer->isActive())
		stsTimer->stop();

	m_pTcpSocket->disconnectFromHost();

	emit this->emit_Ping(true);
}

void CheckConnection::slotErrorConnected(QAbstractSocket::SocketError err)
{
	Q_UNUSED(err)

	if (stsTimer->isActive())
	{
		stsTimer->stop();
	}

	emit this->emit_Ping(false);
}

void CheckConnection::connectToHost(QString ipAddress)
{
	stsTimer->start(29000);

	if (m_pTcpSocket->state() == QAbstractSocket::ConnectedState)
	{
		m_pTcpSocket->disconnectFromHost();
	}

	m_pTcpSocket->connectToHost(ipAddress, 80);
}

bool CheckConnection::ping(int timeOut, QString ipAddress)
{
	QProcess pingProc;
	QString pingCmd;
	QByteArray contents;
	pingCmd = QString("ping -n 3 -w %1 %2").arg(timeOut * 1000).arg(ipAddress);
	pingProc.setProcessChannelMode(QProcess::MergedChannels);

	pingProc.start("c:/windows/system32/cmd.exe", QStringList() << "/c" << pingCmd, QIODevice::ReadOnly);
	pingProc.waitForFinished(12000);

	if (pingProc.state() != QProcess::NotRunning)
	{
		pingProc.close();
	}

	contents = pingProc.readAll();

	if (contents.contains("TTL"))
	{
		emit this->emit_Ping(true);
		return true;
	}
	else
	{
		emit this->emit_Ping(false);
		return false;
	}
}

void CheckConnection::checkConnection(int type)
{
	cmd = type;

	switch (cmd)
	{
	case TypePing::Ping:
	{
		this->start();
	}
	break;
	case TypePing::Socket:
	{
		this->connectToHost(serverAddress);
	}
	break;
	case TypePing::Request:
	{
		sendRequest(serverAddress);
	}
	break;
	}
}

void CheckConnection::run()
{
	this->ping(this->timePing, this->serverAddress);
}

void CheckConnection::setEndpoint(int timeOut, QString serverAddress)
{
	this->timePing = timeOut;
	this->serverAddress = serverAddress;
}

void CheckConnection::sendRequest(QString address)
{
	abortTimer->start(30000);

	QSslConfiguration config = QSslConfiguration::defaultConfiguration();
	config.setProtocol(QSsl::TlsV1_3);

	QNetworkRequest request;
	request.setUrl(QUrl(address));
	qDebug() << address;
	request.setSslConfiguration(config);

	manager->get(request);
}

void CheckConnection::replyFinished(QNetworkReply *reply)
{
	if (abortTimer->isActive())
	{
		abortTimer->stop();
	}

	int error_id = reply->error();

	reply->deleteLater();

	if (error_id == QNetworkReply::NoError)
	{
		auto replyData = reply->readAll();

		auto json = QJsonDocument::fromJson(replyData);
		emit emit_toLoging(0, "PING", QString("REPLY: %1").arg(QString::fromUtf8(replyData)));

		auto resp = json.toVariant().toMap();

		if (resp.value("message").toString().toLower() == "pong" || resp.value("status").toString().toLower() == "pong")
		{
			emit this->emit_Ping(true);
			return;
		}
	}
	else
	{
		emit emit_toLoging(0, "PING",
						   QString("SupportsSSL: %1 | SSLLibraryBuildVersion: %2 | SSLLibraryVersion: %3")
							   .arg(QSslSocket::supportsSsl())
							   .arg(QSslSocket::sslLibraryBuildVersionString(), QSslSocket::sslLibraryVersionString()));
		emit emit_toLoging(1, "PING", QString("ERROR: %1").arg(reply->errorString()));
	}

	emit this->emit_Ping(false);
}

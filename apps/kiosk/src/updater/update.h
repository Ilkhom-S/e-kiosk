#pragma once

/// Подключаемые файлы

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QSettings>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNode>
#include <Common/QtHeadersEnd.h>

// Project
#include "CopyFile.h"
#include "textprogressbar.h"

class DownloadManager : public QThread {
    Q_OBJECT
  public:
    DownloadManager();

    void setUpdatePointName(QString name);
    void setUrlServerIp(QString ip);
    void setXmlFileName(QString name);
    bool checkHashMonitor(QString hash);
    void setDbName(QSqlDatabase &db);

    bool bisyNow;

    QString settingsPath;

  protected:
    void run();

  private:
    QString getOldHash();
    QString fileChecksum(const QString &fileName, QCryptographicHash::Algorithm hashAlgorithm);

    void createDirIfNotExist(QString dirPath);
    void createPathDirIfNotExist(QString dirPath);
    void openDocumentXml();
    void traverseNode(const QDomNode &node);
    void addToDatabaseFile();
    bool updateRecordFile(int id, QString fieldName, QString value);
    bool updateRecordForMore(int id, QString hash, int size);
    bool insertNewRecordFile(QString path, QString name, QString size, QString hash);
    void updateGlobalHash();
    void downloadFileResponse();
    void createFileList(QString path);
    void reCopyAllFiles();
    QStringList getDirFiles(const QString &dirName);
    bool isUpdaterLocked();
    int getAppFileSize();

    CopyFileQs *copyFile;
    QStringList allFilesList;
    QMap<int, QMap<QString, QString>> ServerXmlMap;
    QMap<int, QMap<QString, QString>> copyXmlMap;
    QFile output;
    QNetworkReply *currentDownload;
    QNetworkAccessManager manager;
    TextProgressBar progressBar;
    QElapsedTimer downloadTime;
    QTimer *abortTimer;
    QTimer *resendTimer;
    QTimer *restartTimer;

    QString tmpDirectory;
    QString nowDownloadFileName;
    int nowDownloadFileSize;
    QString nowDownloadFileHash;
    int nowDownloadFileId;
    QString nowDownloadFilePath;
    QString nowDownloadFileAll;
    QString senderName;
    QUrl now_url;
    int totalCount;
    int Debuger;
    bool fileWrite;
    int countFileTag;
    int count_download;
    QSqlDatabase db_;
    QString FolderName;
    QString IpServer;
    QString XmlName;
    QString gblNewHash;

    QDir dirCont;

  private slots:
    void append(QUrl url);
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadReadyRead();
    void downloadFinished();
    void abortReply();
    void reDownloadFile();
    void downloadOneByOneFile();
    void afterCopyAllFiles();
    void compliteUpdateIn();
    void toRestartTimer();

  public slots:
    void startToUpdate();

  signals:
    void emit_Loging(int status, QString title, QString text);
    void emit_reDown();
    void emit_downOneByOne();
    void emit_killSheller();
    void emit_FilesUpdated(QVariantMap data);
    void emit_replaceApp(QString query, QVariantMap data);
};

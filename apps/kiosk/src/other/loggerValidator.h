#ifndef LOGGERVALIDATOR_H
#define LOGGERVALIDATOR_H

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <QThread>
#include <QTimer>

enum LogType { Query = 0, Response = 1 };

class LoggerValidator : public QThread {
    Q_OBJECT

  public:
    QTimer *writeTimer;
    QStringList lstLoging;
    QString account;

    LoggerValidator() {
        writeTimer = new QTimer();
        writeTimer->setInterval(1000);
        connect(writeTimer, SIGNAL(timeout()), this, SLOT(start()));

        QFile info;

        QString fileInit = "logvalidator";
        if (!info.exists(fileInit)) {
            // Тут надо создать папку
            QDir dir;
            dir.mkpath(fileInit);
        }

        writeTimer->start();
    }

    void setLogingText(int state, QByteArray data, QString text) {
        QString answer;
        QString time = QDateTime::currentDateTime().toString("HH:mm:ss:zzz");
        QByteArray baTmp;
        baTmp.clear();
        baTmp = data.toHex().toUpper();

        for (int i = 0; i < baTmp.size(); i += 2) {
            answer += QString(" %1%2").arg(baTmp.at(i)).arg(baTmp.at(i + 1));
        }

        QString stateInfo = "";
        switch (state) {
            case LogType::Query:
                stateInfo = "  запрос  ";
                break;

            case LogType::Response:
                stateInfo = "  ответ   ";
                break;
        }

        text = " (" + text + ")";

        lstLoging << time + stateInfo + " - " + answer + text;
    }

    virtual void run() {
        this->writeText();
        return;
    }

  private slots:

    void writeText() {
        QString str_date = QDate::currentDate().toString("yyyy-MM-dd");

        // проверяем если нету папки yyyy-MM-dd то создаем
        QString fileByDate = "logvalidator/" + str_date;

        QFile info;

        if (!info.exists(fileByDate)) {
            QDir dir;
            dir.mkpath(fileByDate);
        }

        if (lstLoging.count() > 0) {
            bool debugger = false;

            QString fileNameLocal = fileByDate + "/" + account + ".txt";

            // Создаем указательна на файл номер.txt
            QFile fileLogLocal(fileNameLocal);

            // Локальные данные
            if (fileLogLocal.exists()) {
                if (!fileLogLocal.open(QIODevice::Append | QIODevice::Text)) {
                    if (debugger) qDebug() << "error open file log QIODevice::Append";
                    return;
                }
            } else {
                if (!fileLogLocal.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    if (debugger) qDebug() << "error open file log QIODevice::WriteOnly";
                    return;
                }
            }

            // Переводим в строку список
            QString logLiens = lstLoging.join("\n");
            QTextStream outExp(&fileLogLocal);
            outExp.setCodec("UTF-8");
            outExp << logLiens << "\n";

            fileLogLocal.close();

            lstLoging.clear();
        }
    }
};

#endif  // LOGGERVALIDATOR_H

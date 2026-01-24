#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QObject>
#include <QtCore/QTextStream>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <Common/QtHeadersEnd.h>

namespace LoggerLevel {
    enum Enum { Info = 0, Warning = 1, Error = 2 };
} // namespace LoggerLevel

class Logger : public QThread {
    Q_OBJECT

  public:
    QTimer *timer;
    QStringList lstLoging;
    QString fileName;

    Logger() {
        timer = new QTimer();
        timer->setInterval(1000);
        connect(timer, SIGNAL(timeout()), this, SLOT(start()));

        QFile info;
        QString fileInit = "log";

        if (!info.exists(fileInit)) {
            // Тут надо создать папку
            QDir dir;
            dir.mkpath(fileInit);
        }

        timer->start();
    }

    void setLogingText(int state, QString title, QString text) {
        QString stateInfo = "";

        switch (state) {
            case LoggerLevel::Info:
                stateInfo = "INFO:    ";
                break;

            case LoggerLevel::Warning:
                stateInfo = "WARNING: ";
                break;

            case LoggerLevel::Error:
                stateInfo = "ERROR:   ";
                break;
        }

        QString time = QDateTime::currentDateTime().toString("HH:mm:ss");
        title = "| " + title + " |";

        lstLoging << time + " - " + stateInfo + title + " " + text;
    }

    virtual void run() {
        writeData();
        return;
    }

  private slots:
    void writeData() {
        if (lstLoging.count() > 0) {
            bool Debuger = false;

            QString str_date = QDate::currentDate().toString("dd.MM.yyyy");

            QString fileNameLocal = "log/" + str_date + ".txt";

            // Создаем указатель на файл dd.MM.yyyy
            QFile fileLogLocal(fileNameLocal);

            // Локальные данные
            if (fileLogLocal.exists()) {
                if (!fileLogLocal.open(QIODevice::Append | QIODevice::Text)) {
                    if (Debuger)
                        qDebug() << "error open file log QIODevice::Append";
                    return;
                }
            } else {
                if (!fileLogLocal.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    if (Debuger)
                        qDebug() << "error open file log QIODevice::WriteOnly";
                    return;
                }
            }

            // Переводим в строку список
            QString logLiens = lstLoging.join("\n");

            QTextStream outExp(&fileLogLocal);
            outExp << logLiens << "\n";

            fileLogLocal.close();

            lstLoging.clear();
        }
    }
};

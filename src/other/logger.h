#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QThread>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QDir>
#include <QDebug>

enum LogLevel {
    Info        = 0,
    Warning     = 1,
    Error       = 2
};

class Logger : public QThread
{
    Q_OBJECT

public:
    QTimer *timer;
    QStringList lstLoging;
    QString fileName;

    Logger() {
        timer = new QTimer();
        timer->setInterval(1000);
        connect(timer,SIGNAL(timeout()),this,SLOT(start()));

        QFile info;
        QString fileInit = "log";

        if (!info.exists(fileInit)) {
            //Тут надо создать папку
            QDir dir;
            dir.mkpath(fileInit);
        }

        timer->start();
    }

    void setLogingText(int state, QString title, QString text)
    {
        QString stateInfo = "";

        switch (state) {
            case LogLevel::Info :
                stateInfo = "INFO:    ";
                break;

            case LogLevel::Warning:
                stateInfo = "WARNING: ";
                break;

            case LogLevel::Error:
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
    void writeData(){
        if (lstLoging.count() > 0) {
            bool Debuger = false;

            QString str_date = QDate::currentDate().toString("dd.MM.yyyy");


            QString fileNameLocal = "log/" + str_date + ".txt";

            //Создаем указатель на файл dd.MM.yyyy
            QFile fileLogLocal(fileNameLocal);

            //Локальные данные
            if (fileLogLocal.exists()) {
                if (!fileLogLocal.open(QIODevice::Append | QIODevice::Text)){
                    if (Debuger) qDebug() << "error open file log QIODevice::Append";
                    return;
                }
            } else {
                if (!fileLogLocal.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    if (Debuger) qDebug() << "error open file log QIODevice::WriteOnly";
                    return;
                }
            }

            //Переводим в строку список
            QString logLiens = lstLoging.join("\n");

            QTextStream outExp(&fileLogLocal);
            outExp << logLiens << "\n";

            fileLogLocal.close();

            lstLoging.clear();
        }
    }
};

#endif // LOGGER_H

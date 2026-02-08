/* @file Запись в лог для qml плагинов. */

#pragma once

#include <QtCore/QDebug>
#include <QtCore/QObject>

//------------------------------------------------------------------------------
class Log {
public:
    enum LogLevel { Debug, Normal, Warning, Error };

    Log(LogLevel aLevel) : m_Level(aLevel) {}

    ~Log() {
        QObject *logger = initialize();

        if (logger) {
            switch (m_Level) {
            case Debug:
                QMetaObject::invokeMethod(logger, "debug", Q_ARG(const QString &, m_Message));
                break;
            case Normal:
                QMetaObject::invokeMethod(logger, "normal", Q_ARG(const QString &, m_Message));
                break;
            case Warning:
                QMetaObject::invokeMethod(logger, "warning", Q_ARG(const QString &, m_Message));
                break;
            case Error:
                QMetaObject::invokeMethod(logger, "error", Q_ARG(const QString &, m_Message));
                break;
            }
        } else {
            switch (m_Level) {
            case Debug:
                qDebug() << m_Message;
                break;
            case Normal:
                qDebug() << m_Message;
                break;
            case Warning:
            case Error:
                qWarning() << m_Message;
                break;
            }
        }
    }

    static QObject *initialize(QObject *aApplication = nullptr) {
        static QObject *application = nullptr;

        if (aApplication) {
            application = aApplication;
        }

        return application ? application->property("log").value<QObject *>() : nullptr;
    }

    Log &operator<<(const QString &aString) {
        m_Message.append(aString);
        return *this;
    }

private:
    QString m_Message;
    LogLevel m_Level;
};

//------------------------------------------------------------------------------

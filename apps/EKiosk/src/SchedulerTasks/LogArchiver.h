/* @file Реализация задачи архивации журнальных файлов. */

#pragma once

// Qt
#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QList>

#include <Common/QtHeadersBegin.h>
#include <Common/QtHeadersEnd.h>

// SDK
#include <SDK/PaymentProcessor/Core/ITask.h>

// Модули
#include <Common/ILogable.h>

#include <Packer/Packer.h>

//---------------------------------------------------------------------------
class LogArchiver : public QObject, public SDK::PaymentProcessor::ITask, public ILogable {
    Q_OBJECT

public:
    /// Конструктор задачи архивации логов
    LogArchiver(const QString &aName, const QString &aLogName, const QString &aParams);
    virtual ~LogArchiver();

    /// Выполнить архивацию старых логов в zip/7z архивы
    virtual void execute();

    /// Остановить выполнение задачи архивации
    virtual bool cancel();

    /// подписаться на сигнал окончания задания
    virtual bool subscribeOnComplete(QObject *aReceiver, const char *aSlot);

private:
    bool m_Canceled;      /// Флаг отмены операции
    int m_MaxSize;        /// Максимальный размер логов из настроек (МБ)
    QDir m_LogDir;        /// Папка с журнальными файлами
    QString m_KernelPath; /// Путь к рабочей директории приложения
    Packer m_Packer;      /// Упаковщик для создания архивов

private:
    /// Получить список дат, логи которых подлежат упаковке
    QList<QDate> getDatesForPack() const;

    /// Упаковать логи за указанную дату в архив
    void packLogs(QDate aDate);

    /// Удалить исходные логи за указанную дату после архивации
    void removeLogs(QDate aDate);

    /// Удалить старые архивы при превышении максимального размера
    void checkArchiveSize();

    /// Сформировать имя файла архива для указанной даты
    QString logArchiveFileName(QDate aDate);

    /// Удалить файл с логированием результата
    bool removeFile(const QFileInfo &aFile);

signals:
    void finished(const QString &aName, bool aComplete);
};

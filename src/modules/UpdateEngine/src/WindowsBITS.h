/* @file Обертка над подсистемой Windows BITS. */

#pragma once

// Windows-specific code: BITS is only available on Windows
#ifdef Q_OS_WIN32

#include <QtCore/QMap>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>
#include <QtCore/QUrl>

#include <Common/ILogable.h>

#include "WindowsBITS_i.h"

namespace CBITS {

//---------------------------------------------------------------------------
class CopyManager_p;

//---------------------------------------------------------------------------
class CopyManager : public ILogable {
public:
    explicit CopyManager(ILog *aLog);
    virtual ~CopyManager();

    /// Прекратить мучения BITS
    void shutdown();

    /// Проверка работоспособности подсистемы
    bool isReady() const;

    /// Получить список существующих заданий на скачивание
    QMap<QString, SJob> getJobs(const QString &aFilter);

    /// Создать задание
    bool createJob(const QString &aName, SJob &aJob, int aPriority);

    /// Сконфигурировать задачу для запуска приложения по окончанию скачивания
    bool setNotify(const QString &aApplicationPath, const QString &aParameters);

    /// Добавить в задачу файл для скачивания
    bool addTask(const QUrl &aUrl, const QString &aFileName);

    /// Открыть существующую задачу
    bool openJob(const SJob &aJob);

    /// Запустить текущую задачу в обработку
    bool resume();

    /// Сбросить текущую задачу
    bool cancel();

    /// Завершить обработку текущей задачи
    bool complete();

private:
    bool internalResume();
    QString makeJobName(const QString &aName = QString());

private:
    int m_JobsCount;
    int m_Priority;
    QString m_JobName;
    QString m_NotifyApplication;
    QString m_NotifyParameters;
    QSharedPointer<CopyManager_p> m_CopyManager;
};

//---------------------------------------------------------------------------
} // namespace CBITS

#endif // Q_OS_WIN32

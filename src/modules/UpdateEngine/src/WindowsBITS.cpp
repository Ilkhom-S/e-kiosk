/* @file Обертка над подсистемой Windows BITS. */

// Windows-specific code: BITS is only available on Windows
#ifdef Q_OS_WIN32

#include "WindowsBITS.h"

#include "WindowsBITS_p.h"

namespace CBITS {

//---------------------------------------------------------------------------
CopyManager::CopyManager(ILog *aLog) : ILogable(aLog), m_JobsCount(0), m_Priority(CBITS::HIGH) {
    m_CopyManager = QSharedPointer<CopyManager_p>(new CopyManager_p(aLog));
}

//---------------------------------------------------------------------------
CopyManager::~CopyManager() {}

//---------------------------------------------------------------------------
void CopyManager::shutdown() {
    m_CopyManager.clear();
}

//---------------------------------------------------------------------------
bool CopyManager::isReady() const {
    return m_CopyManager && m_CopyManager->isReady();
}

//---------------------------------------------------------------------------
QMap<QString, SJob> CopyManager::getJobs(const QString &aFilter) {
    QMap<QString, SJob> result;

    if (isReady()) {
        auto allJobs = m_CopyManager->getJobs();

        foreach (auto key, QStringList(allJobs.keys()).filter(aFilter, Qt::CaseInsensitive)) {
            result.insert(key, allJobs.value(key));
        }
    }

    return result;
}

//---------------------------------------------------------------------------
bool CopyManager::createJob(const QString &aName, SJob &aJob, int aPriority) {
    if (isReady()) {
        m_Priority = aPriority;
        return m_CopyManager->createJob(makeJobName(aName), aJob, aPriority);
    }

    return false;
}

//---------------------------------------------------------------------------
bool CopyManager::setNotify(const QString &aApplicationPath, const QString &aParameters) {
    m_NotifyApplication = aApplicationPath;
    m_NotifyParameters = aParameters;
    return true;
}

//---------------------------------------------------------------------------
bool CopyManager::addTask(const QUrl &aUrl, const QString &aFileName) {
    if (isReady()) {
        switch (m_CopyManager->addTask(aUrl, aFileName)) {
        case AddTaskResult::OK:
            return true;
        case AddTaskResult::Error:
            return false;
        case AddTaskResult::JobIsFull: {
            if (!internalResume()) {
                return false;
            }

            toLog(LoggerLevel::Normal, QString("BITS: Job %1 resumed.").arg(makeJobName()));

            m_JobsCount++;
            SJob newJob;
            return m_CopyManager->createJob(makeJobName(), newJob, m_Priority) &&
                   addTask(aUrl, aFileName);
        }
        }
    }

    return false;
}

//---------------------------------------------------------------------------
bool CopyManager::openJob(const SJob &aJob) {
    if (isReady()) {
        return m_CopyManager->openJob(aJob);
    }

    return false;
}

//---------------------------------------------------------------------------
bool CopyManager::internalResume() {
    return m_CopyManager->setJobNotify(m_NotifyApplication, m_NotifyParameters) &&
           m_CopyManager->resume();
}

//---------------------------------------------------------------------------
QString CopyManager::makeJobName(const QString &aName /*= QString()*/) {
    if (!aName.isEmpty()) {
        m_JobName = aName;
    }

    return m_JobName + QString("#%1").arg(m_JobsCount, 2, 10, QChar('0'));
}

//---------------------------------------------------------------------------
bool CopyManager::resume() {
    if (isReady() && internalResume()) {
        toLog(LoggerLevel::Normal, QString("BITS: Job %1 resumed.").arg(makeJobName()));

        return true;
    }

    return false;
}

//---------------------------------------------------------------------------
bool CopyManager::cancel() {
    if (isReady()) {
        return m_CopyManager->cancel();
    }

    return false;
}

//---------------------------------------------------------------------------
bool CopyManager::complete() {
    if (isReady()) {
        return m_CopyManager->complete();
    }

    return false;
}

//---------------------------------------------------------------------------
} // namespace CBITS

#endif // Q_OS_WIN32

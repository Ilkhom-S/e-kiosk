/* @file Обертка над подсистемой Windows BITS. Приватный класс. */

// Windows-specific code: BITS is only available on Windows
#ifdef Q_OS_WIN32

#include "WindowsBITS_p.h"

#include <QtCore/QDir>

#include "WindowsBITS_i.h"

namespace CBITS {
//---------------------------------------------------------------------------
QSet<Qt::HANDLE> CopyManager_p::m_ThreadInitialized;

//---------------------------------------------------------------------------
CopyManager_p::CopyManager_p(ILog *aLog) : ILogable(aLog) {
    HRESULT hr;

    if (!m_ThreadInitialized.contains(QThread::currentThreadId())) {
        hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
            toLog(LoggerLevel::Error,
                  QString("BITS: Error call CoInitializeEx(), result=0x%1").arg(hr, 0, 16));
            return;
        }

        // The impersonation level must be at least RPC_C_IMP_LEVEL_IMPERSONATE.
        hr = CoInitializeSecurity(NULL,
                                  -1,
                                  NULL,
                                  NULL,
                                  RPC_C_AUTHN_LEVEL_CONNECT,
                                  RPC_C_IMP_LEVEL_IMPERSONATE,
                                  NULL,
                                  EOAC_DYNAMIC_CLOAKING,
                                  0);
        if (FAILED(hr) && hr != RPC_E_TOO_LATE) {
            toLog(LoggerLevel::Error,
                  QString("BITS: Error call CoInitializeSecurity(), result=0x%1").arg(hr, 0, 16));
            return;
        }

        m_ThreadInitialized.insert(QThread::currentThreadId());
    }

    // Connect to BITS.
    hr = CoCreateInstance(__uuidof(BackgroundCopyManager),
                          NULL,
                          CLSCTX_LOCAL_SERVER,
                          __uuidof(IBackgroundCopyManager),
                          (void **)&m_QueueMgr);
    if (FAILED(hr)) {
        toLog(LoggerLevel::Error,
              QString("BITS: Failed to connect with BITS, result=0x%1").arg(hr, 0, 16));
    }
}

//---------------------------------------------------------------------------
CopyManager_p::~CopyManager_p() {}

//---------------------------------------------------------------------------
bool CopyManager_p::isReady() const {
    return (m_QueueMgr != nullptr);
}

//---------------------------------------------------------------------------
bool fillJobInfo(CCom_Ptr<IBackgroundCopyJob> &aJob, SJob &aJobInfo) {
    GUID id;
    if (FAILED(aJob->GetId(&id))) {
        return false;
    }

    aJobInfo.m_GuidID = QUuid(id);

    LPWSTR pwstrText = nullptr;
    if (FAILED(aJob->GetDisplayName(&pwstrText))) {
        return false;
    }

    aJobInfo.m_Name = QString::from_WCharArray(pwstrText);
    ::CoTaskMem_Free(pwstrText);

    if (FAILED(aJob->GetDescription(&pwstrText))) {
        return false;
    }

    aJobInfo.m_Desc = QString::from_WCharArray(pwstrText);
    ::CoTaskMem_Free(pwstrText);

    aJob->GetState((BG_JOB_STATE *)&aJobInfo.m_State);
    aJob->GetMinimum_RetryDelay((ULONG *)&aJobInfo.m_MinRetryDelay);
    aJob->GetNoProgressTimeout((ULONG *)&aJobInfo.m_NoProgressTimeout);

    BG_JOB_PROGRESS progress;
    if (FAILED(aJob->GetProgress(&progress))) {
        return false;
    }

    aJobInfo.m_Progress.bytesTotal = progress.BytesTotal;
    aJobInfo.m_Progress.bytesTransferred = progress.BytesTransferred;
    aJobInfo.m_Progress.filesTotal = progress.FilesTotal;
    aJobInfo.m_Progress.filesTransferred = progress.FilesTransferred;

    return true;
}

//---------------------------------------------------------------------------
QMap<QString, SJob> CopyManager_p::getJobs() {
    QMap<QString, SJob> result;

    if (m_QueueMgr == nullptr) {
        return result;
    }

    CCom_Ptr<IEnum_BackgroundCopyJobs> enum_Jobs;
    HRESULT hr = m_QueueMgr->Enum_Jobs(0, &enum_Jobs);

    if (SUCCEEDED(hr)) {
        ULONG ulCount = 0;
        hr = enum_Jobs->GetCount(&ulCount);
        hr = enum_Jobs->Reset();
        // signed/unsigned syndrome
        int iCount = ulCount;
        for (int i = 0; i < iCount; i++) {
            CCom_Ptr<IBackgroundCopyJob> job;
            hr = enum_Jobs->Next(1, &job, NULL);
            if (SUCCEEDED(hr)) {
                SJob jobItem;

                if (fillJobInfo(job, jobItem)) {
                    toLog(LoggerLevel::Debug, QString("BITS: JOB: %1").arg(jobItem.toString()));

                    result[jobItem.m_Name] = jobItem;
                } else {
                    toLog(LoggerLevel::Error, QString("BITS: Failed fill job info."));
                }
            } else {
                toLog(LoggerLevel::Error,
                      QString("BITS: Failed get next job, result=0x%1").arg(hr, 0, 16));
            }
        }
    } else {
        toLog(LoggerLevel::Error,
              QString("BITS: Failed to enum jobs BITS, result=0x%1").arg(hr, 0, 16));
    }

    return result;
}

//---------------------------------------------------------------------------
bool CopyManager_p::cancel() {
    return m_CurrentJob && SUCCEEDED(m_CurrentJob->Cancel());
}

//---------------------------------------------------------------------------
bool CopyManager_p::complete() {
    HRESULT hr = E_FAIL;

    if (m_CurrentJob) {
        hr = m_CurrentJob->Complete();

        if (FAILED(hr)) {
            toLog(LoggerLevel::Error,
                  QString("BITS: Failed to complete job, result=0x%1").arg(hr, 0, 16));
        }
    }

    return SUCCEEDED(hr);
}

//---------------------------------------------------------------------------
BG_JOB_PRIORITY priorityConvert(int aPriority) {
    switch (aPriority) {
    case CBITS::FOREGROUND:
        return BG_JOB_PRIORITY_FOREGROUND;
    case CBITS::HIGH:
        return BG_JOB_PRIORITY_HIGH;
    case CBITS::NORMAL:
        return BG_JOB_PRIORITY_NORMAL;
    case CBITS::LOW:
        return BG_JOB_PRIORITY_LOW;
    default:
        return BG_JOB_PRIORITY_HIGH;
    }
}

//---------------------------------------------------------------------------
bool CopyManager_p::createJob(const QString &aName, SJob &aJob, int aPriority) {
    if (!m_QueueMgr) {
        return false;
    }

    if (m_CurrentJob) {
        m_CurrentJob.Release();
    }

    GUID guidJob;
    HRESULT hr = m_QueueMgr->CreateJob(
        aName.toStdWString().c_str(), BG_JOB_TYPE_DOWNLOAD, &guidJob, &m_CurrentJob);
    if (FAILED(hr) || m_CurrentJob == nullptr) {
        toLog(LoggerLevel::Error,
              QString("BITS: Failed to create job, result=0x%1").arg(hr, 0, 16));

        return false;
    }

    m_CurrentJob->SetPriority(priorityConvert(aPriority));

    CCom_Ptr<IBackgroundCopyJobHttpOptions> httpOption;
    if (SUCCEEDED(m_CurrentJob->QueryInterface(&httpOption)) && httpOption) {
        ULONG flags = 0;
        httpOption->GetSecurityFlags(&flags);
        httpOption->SetSecurityFlags(flags | BG_SSL_IGNORE_CERT_CN_INVALID |
                                     BG_SSL_IGNORE_CERT_DATE_INVALID | BG_SSL_IGNORE_UNKNOWN_CA |
                                     BG_SSL_IGNORE_CERT_WRONG_USAGE);
    }

    fillJobInfo(m_CurrentJob, aJob);

    return (m_CurrentJob != nullptr);
}

//---------------------------------------------------------------------------
bool CopyManager_p::setJobNotify(const QString &aApplicationPath, const QString &aParameters) {
    CCom_Ptr<IBackgroundCopyJob2> job2;
    if (m_CurrentJob && SUCCEEDED(m_CurrentJob->QueryInterface(&job2)) && job2) {
        std::wstring appPath = aApplicationPath.toStdWString();

#ifdef _DEBUG
        appPath = L"C:\\Devil\\Projects\\term_dev\\TerminalClient\\MyWorkDir\\Updater.exe";
#endif

        std::wstring parameters = appPath + L" " + aParameters.toStdWString();
        auto hr1 = job2->SetNotifyCmdLine(appPath.c_str(), parameters.c_str());
        auto hr2 = job2->SetNotifyFlags(BG_NOTIFY_JOB_TRANSFERRED | BG_NOTIFY_JOB_ERROR);

        LPWSTR namePtr = nullptr;
        m_CurrentJob->GetDisplayName(&namePtr);
        QString name = QString::from_WCharArray(namePtr);
        CoTaskMem_Free(namePtr);

        if (SUCCEEDED(hr1) && SUCCEEDED(hr2)) {
            toLog(LoggerLevel::Normal,
                  QString("Set job '%1' notify: '%2' '%3'")
                      .arg(name)
                      .arg(aApplicationPath)
                      .arg(aParameters));

            return true;
        } else {
            toLog(LoggerLevel::Error,
                  QString("Failed set job '%1' notify: '%2' '%3'")
                      .arg(name)
                      .arg(aApplicationPath)
                      .arg(aParameters));
        }
    }

    return false;
}

//---------------------------------------------------------------------------
CBITS::AddTaskResult::Enum CopyManager_p::addTask(const QUrl &aUrl, const QString &aFileName) {
    if (m_CurrentJob) {
        QString url = aUrl.toString();
        QString path = QDir::toNativeSeparators(aFileName);

        auto hr = m_CurrentJob->AddFile(url.toStdWString().c_str(), path.toStdWString().c_str());
        if (SUCCEEDED(hr)) {
            toLog(LoggerLevel::Debug, QString("BITS: Add task to job OK: %1.").arg(url));

            return AddTaskResult::OK;
        }

        if (hr == BG_E_TOO_MANY_FILES_IN_JOB) {
            toLog(LoggerLevel::Error,
                  QString("BITS: Add task to job failed. Job is FULL. Url: %1").arg(url));

            return AddTaskResult::JobIsFull;
        }

        toLog(LoggerLevel::Error,
              QString("BITS: Add task to job failed: %1. HRESULT=0x%2. Url: %3")
                  .arg(getJobError())
                  .arg(hr, 8, 16)
                  .arg(url));
    }

    return AddTaskResult::Error;
}

//---------------------------------------------------------------------------
bool CopyManager_p::openJob(const SJob &aJob) {
    if (!m_QueueMgr) {
        return false;
    }

    if (m_CurrentJob) {
        m_CurrentJob.Release();
    }

    HRESULT hr = m_QueueMgr->GetJobW(aJob.m_GuidID, &m_CurrentJob);
    if (SUCCEEDED(hr) && m_CurrentJob) {
        return true;
    }

    toLog(LoggerLevel::Error,
          QString("BITS: Failed open the job: %1. HRESULT=0x%2.")
              .arg(aJob.toString())
              .arg(hr, 8, 16));
    return false;
}

//---------------------------------------------------------------------------
bool CopyManager_p::resume() {
    if (m_CurrentJob) {
        HRESULT hr = m_CurrentJob->Resume();
        if (SUCCEEDED(hr)) {
            return true;
        }

        toLog(LoggerLevel::Error,
              QString("BITS: Resume job failed: %1. HRESULT=0x%2.")
                  .arg(getJobError())
                  .arg(hr, 8, 16));
    }

    return false;
}

//---------------------------------------------------------------------------
QString CopyManager_p::getJobError() {
    CCom_Ptr<IBackgroundCopyError> error;
    if (SUCCEEDED(m_CurrentJob->GetError(&error)) && error) {
        LPWSTR errorMsg = nullptr;
        error->GetErrorDescription(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), &errorMsg);
        if (errorMsg) {
            QString msg = QString::from_WCharArray(errorMsg);
            CoTaskMem_Free(errorMsg);

            return msg;
        }
    }

    return QString();
}

//---------------------------------------------------------------------------
} // namespace CBITS

#endif // Q_OS_WIN32

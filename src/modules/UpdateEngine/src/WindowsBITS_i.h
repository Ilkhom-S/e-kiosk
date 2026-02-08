#pragma once

#include <QtCore/QDebug>
#include <QtCore/QString>
#include <QtCore/QUuid>

namespace CBITS {

//---------------------------------------------------------------------------
enum EJobStates {
    EJobStateQueued = 0,
    EJobStateConnecting = 1,
    EJobStateTransferring = 2,
    EJobStateSuspended = 3,
    EJobStateError = 4,
    EJobStateTransientError = 5,
    EJobStateTransferred = 6,
    EJobStateAcknowledged = 7,
    EJobStateCancelled = 8,
    EJobStateUnknown = 15,
};

//---------------------------------------------------------------------------
enum EFileStates {
    EFileStateTransferring = 9,
    EFileStateTransferred = 6,
    EFileStateUnknown = 15,
};

enum EJobPriority { FOREGROUND = 0, HIGH, NORMAL, LOW };

//-------------------------------------------------------------------------------------------------
struct SJobProgress {
    quint64 bytesTotal;
    quint64 bytesTransferred;
    quint32 filesTotal;
    quint32 filesTransferred;

    SJobProgress() : bytesTotal(0), bytesTransferred(0), filesTotal(0), filesTransferred(0) {}

    QString toString() const {
        return QString("%1/%2 bytes in %3/%4 files")
            .arg(bytesTransferred)
            .arg(bytesTotal)
            .arg(filesTransferred)
            .arg(filesTotal);
    }
};

//-------------------------------------------------------------------------------------------------
class SJob {
public:
    SJob() {
        m_State = EJobStateUnknown;
        m_MinRetryDelay = 0;
        m_NoProgressTimeout = 0;
    }

    bool isComplete() const {
        return m_State == EJobStateTransferred || m_State == EJobStateAcknowledged;
    }

    bool isFatal() const { return m_State == EJobStateError || m_State == EJobStateCancelled; }

    bool inProgress() const { return !isComplete() && !isFatal(); }

    QString toString() const {
        return QString(
                   "UUID: %1. Name: '%2'. Dscr: %3. State: %4. Progress: %5. MinRetryDelay: %6. "
                   "NoProgressTimeout: %7.")
            .arg(m_GuidID.toString())
            .arg(m_Name)
            .arg(m_Desc)
            .arg(m_State)
            .arg(m_Progress.toString())
            .arg(m_MinRetryDelay)
            .arg(m_NoProgressTimeout);
    }

public:
    QUuid m_GuidID;
    QString m_Name;
    QString m_Desc;
    quint32 m_State;
    SJobProgress m_Progress;
    quint32 m_MinRetryDelay;
    quint32 m_NoProgressTimeout;
};

//---------------------------------------------------------------------------
} // namespace CBITS

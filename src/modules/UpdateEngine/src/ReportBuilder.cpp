/* @file Генератор отчётов обновления. */

#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>

#include <Common/BasicApplication.h>

#include <UpdateEngine/ReportBuilder.h>
#include <utility>

ReportBuilder::ReportBuilder(QString aWorkDirectory /*= ""*/)
    : ILogable(CReportBuilder::LogName), m_WorkDirectory(std::move(aWorkDirectory)) {}

//------------------------------------------------------------------------
void ReportBuilder::open(const QString &aCommand, const QString &aUrl, const QString &aMd5) {
    QString fileName = (m_WorkDirectory.isEmpty() ? QDir::currentPath() : m_WorkDirectory) +
                       "/update/" + QString("update_%1.rpt").arg(aCommand);

    bool writeCreateDate = !QFile::exists(fileName);

    QFileInfo fileInfo(fileName);

    QDir dir;
    dir.mkpath(fileInfo.absolutePath());

    toLog(LogLevel::Normal, QString("Opening report file %1.").arg(fileName));

    m_Report = QSharedPointer<QSettings>(new QSettings(fileName, QSettings::IniFormat));

    m_Report->setValue("id", aCommand);
    m_Report->setValue("url", aUrl);
    m_Report->setValue("md5", aMd5);

    if (writeCreateDate) {
        m_Report->setValue("create_date",
                           QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
    }

    updateTimestamp();
}

//------------------------------------------------------------------------
void ReportBuilder::close() {
    m_Report.clear();
}

//------------------------------------------------------------------------
void ReportBuilder::setStatus(SDK::PaymentProcessor::IRemoteService::EStatus aStatus) {
    if (m_Report) {
        m_Report->setValue("status", aStatus);
        updateTimestamp();
    }
}

//------------------------------------------------------------------------
void ReportBuilder::setStatusDescription(const QString &aStatusMessage) {
    if (m_Report) {
        m_Report->setValue("status_desc", aStatusMessage);
        updateTimestamp();
    }
}

//------------------------------------------------------------------------
void ReportBuilder::setProgress(int aProgress) {
    if (m_Report) {
        m_Report->setValue("progress", aProgress);
        updateTimestamp();
    }
}

//------------------------------------------------------------------------
void ReportBuilder::updateTimestamp() {
    if (m_Report) {
        m_Report->setValue("last_update",
                           QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"));
        m_Report->sync();
    }
}

//------------------------------------------------------------------------

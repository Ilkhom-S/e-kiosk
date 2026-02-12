/* @file Реализация интерфейсов для работы с БД. */

#include "DatabaseUtils.h"

#include <QtCore/QFile>
#include <QtCore/QMutexLocker>
#include <QtCore/QRecursiveMutex>
#include <QtCore/QRegularExpression>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringList>

#include <Common/ExceptionFilter.h>

#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>
#include <memory>

#include "System/IApplication.h"

namespace CDatabaseUtils {
const QString EmptyDatabaseScript = ":/scripts/empty_db.sql";
const QString DatabasePatchParam = "db_patch";
const QString DatabaseUpdateParam = "db_update";

// Список патчей базы
const struct {
    int version;
    QString script;
} Patches[] = {
    {6, ":/scripts/db_patch_6.sql"},
    {7, ":/scripts/db_patch_7.sql"},
    {8, ":/scripts/db_patch_8.sql"},
    {9, ":/scripts/db_patch_9.sql"},
    {10, ":/scripts/db_patch_10.sql"},
    {11, ":/scripts/db_patch_11.sql"},
    {12, ":/scripts/db_patch_12.sql"},
};

} // namespace CDatabaseUtils

//---------------------------------------------------------------------------
DatabaseUtils::DatabaseUtils(IDatabaseProxy &aProxy, IApplication *aApplication)
    : m_Database(aProxy), m_Application(aApplication), m_Log(aApplication->getLog()),
      m_PaymentLog(ILog::getInstance("Payments")) {}

//---------------------------------------------------------------------------
DatabaseUtils::~DatabaseUtils() = default;

//---------------------------------------------------------------------------
bool DatabaseUtils::initialize() {
    try {
        if (!m_Database.isConnected()) {
            throw std::runtime_error("not connected.");
        }

        if (!m_Database.transaction()) {
            throw std::runtime_error("Cannot start database transaction.");
        }

        // Проверяем, созданы ли таблицы.
        if (databaseTableCount() == 0) {
            updateDatabase(CDatabaseUtils::EmptyDatabaseScript);
            // после этого скрипта databasePatch() = 5
        }

        for (const auto &patch : CDatabaseUtils::Patches) {
            if (databasePatch() < patch.version) {
                LOG(m_Log,
                    LogLevel::Normal,
                    QString("Patch database to version %1.").arg(patch.version));

                updateDatabase(patch.script);
            }
        }
    } catch (...) {
        EXCEPTION_FILTER(m_Log);

        m_Database.rollback();

        return false;
    }

    if (!m_Database.commit()) {
        LOG(m_Log, LogLevel::Error, "Failed to commit changes to the terminal database.");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::updateDatabase(const QString &aSqlScriptName) {
    QFile ftemp(aSqlScriptName);
    if (!ftemp.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Cannot open database script from resources.");
    }

    QString resSQL = ftemp.readAll();

    // Удалим комментарии ("-- some comment") и ("/* many \n comment */").
    QRegularExpression rx(R"((\/\*.*\*\/|\-\-.*\n))");
    ////////rx.setMinimal(true); // Removed for Qt5/6 compatibility // Removed for Qt5/6
    /// compatibility // Removed for
    /// Qt5/6 compatibility // Removed for Qt5/6 compatibility
    resSQL.replace(rx, "");

    // Удалим перевод строк с сохранением возможности писать ("CREATE\nTABLE")
    resSQL.replace("\r", "");
    resSQL.replace("\n", " ");

    QStringList creatingSteps = resSQL.split(";");

    long rowsAffected(0);

    foreach (const QString &step, creatingSteps) {
        if (step.trimmed().isEmpty()) {
            continue;
        }

        if (!m_Database.execDML(step, rowsAffected)) {
            throw std::runtime_error("Cannot execute regular sql query.");
        }
    }

    return true;
}

//---------------------------------------------------------------------------
int DatabaseUtils::databaseTableCount() const {
    QString queryMessage = "SELECT count(*) FROM sqlite_master WHERE type = 'table'";
    QScopedPointer<IDatabaseQuery> dbQuery(m_Database.execQuery(queryMessage));
    if (!dbQuery) {
        LOG(m_Log, LogLevel::Error, "Failed to check database tables presence.");
        return 0;
    }

    if (dbQuery->first()) {
        return dbQuery->value(0).toInt();
    }

    return 0;
}

//---------------------------------------------------------------------------
int DatabaseUtils::databasePatch() const {
    QString queryMessage = "SELECT `value` FROM device_param WHERE `name` = 'db_patch'";
    QScopedPointer<IDatabaseQuery> dbQuery(m_Database.execQuery(queryMessage));
    if (!dbQuery) {
        LOG(m_Log, LogLevel::Error, "Failed to check database patch version.");
        return 0;
    }

    if (dbQuery->first()) {
        return dbQuery->value(0).toInt();
    }

    return 0;
}

//---------------------------------------------------------------------------
IDatabaseQuery *DatabaseUtils::prepareQuery(const QString &aQuery) {
    QMutexLocker lock(&m_AccessMutex);

    std::unique_ptr<IDatabaseQuery> query(m_Database.createQuery(aQuery));

    return query ? query.release() : nullptr;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::execQuery(IDatabaseQuery *aQuery) {
    QMutexLocker lock(&m_AccessMutex);

    if (aQuery) {
        return aQuery->exec();
    }

    return false;
}

//---------------------------------------------------------------------------
void DatabaseUtils::releaseQuery(IDatabaseQuery *aQuery) {
    QMutexLocker lock(&m_AccessMutex);

    delete aQuery;
}

//---------------------------------------------------------------------------

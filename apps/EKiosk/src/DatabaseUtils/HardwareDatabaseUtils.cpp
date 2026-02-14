/* @file Реализация интерфейсов для работы с железом в БД. */

#include <QtCore/QDateTime>
#include <QtCore/QMutexLocker>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <SDK/Drivers/DeviceTypes.h>
#include <SDK/PaymentProcessor/Core/DatabaseConstants.h>

#include <DatabaseProxy/IDatabaseProxy.h>
#include <DatabaseProxy/IDatabaseQuery.h>

#include "DatabaseUtils.h"

namespace PPSDK = SDK::PaymentProcessor;

//---------------------------------------------------------------------------
bool DatabaseUtils::getDeviceParams(const QString & /*aDeviceConfigName*/,
                                    QVariantMap & /*aParameters*/) {
    Q_ASSERT(false);
    return false;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::isDeviceParam_Exists(const QString & /*aDeviceConfigName*/) {
    Q_ASSERT(false);
    return false;
}

//---------------------------------------------------------------------------
QVariant DatabaseUtils::getDeviceParam(const QString &aDeviceConfigName,
                                       const QString &aParamName) {
    QString queryMessage =
        "SELECT `value` FROM `device_param` WHERE `name` = :param_name AND "
        "`fk_device_id` = (SELECT `id` FROM `device` WHERE `name` = :config_name)";

    QScopedPointer<IDatabaseQuery> dbQuery(m_Database.createQuery(queryMessage));
    if (!dbQuery.isNull()) {
        dbQuery->bindValue(":param_name", aParamName);
        dbQuery->bindValue(":config_name", aDeviceConfigName);

        if (dbQuery->exec() && dbQuery->first()) {
            return dbQuery->value(0);
        }
    }

    return {};
}

//---------------------------------------------------------------------------
bool DatabaseUtils::setDeviceParam(const QString &aDeviceConfigName,
                                   const QString &aParamName,
                                   const QVariant &aParamValue) {
    // Не вставляем NULL или невалидные значения - ограничение БД требует NOT NULL.
    if (!aParamValue.isValid() || aParamValue.isNull()) {
        return true; // Успешно пропускаем null/invalid значения
    }

    // Также проверяем пустые строки (QString("") != QString() но может вызвать ошибку БД)
    if (aParamValue.type() == QVariant::String && aParamValue.toString().isEmpty()) {
        return true; // Пропускаем пустые строки
    }

    if (!hasDevice(aDeviceConfigName)) {
        if (!addDevice(aDeviceConfigName)) {
            return false;
        }
    }

    QMutexLocker lock(&m_AccessMutex);

    QString queryMessage =
        QString("INSERT OR REPLACE INTO `device_param` (`name`, `value`, `fk_device_id`) VALUES "
                "(:param_name, "
                ":param_value, (SELECT `id` FROM `device` WHERE `name` = :config_name))");

    QScopedPointer<IDatabaseQuery> query(m_Database.createQuery(queryMessage));
    if (!query) {
        return false;
    }

    query->bindValue(":param_name", aParamName);
    query->bindValue(":param_value", aParamValue);
    query->bindValue(":config_name", aDeviceConfigName);

    return query->exec();
}

//---------------------------------------------------------------------------
bool DatabaseUtils::hasDevice(const QString &aDeviceConfigName) {
    QScopedPointer<IDatabaseQuery> dbQuery(
        m_Database.createQuery("SELECT * FROM `device` WHERE `name` = :config_name"));

    dbQuery->bindValue(":config_name", aDeviceConfigName);

    return (dbQuery->exec() && dbQuery->first());
}

//---------------------------------------------------------------------------
bool DatabaseUtils::addDevice(const QString &aDeviceConfigName) {
    QMutexLocker lock(&m_AccessMutex);

    int deviceType = SDK::Driver::EDeviceType::from_String(aDeviceConfigName.section('.', 2, 2));

    QScopedPointer<IDatabaseQuery> dbQuery(m_Database.createQuery(
        "INSERT OR REPLACE INTO `device` (`name`, `type`) VALUES (:config_name, :device_type)"));

    if (!dbQuery) {
        return false;
    }

    dbQuery->bindValue(":config_name", aDeviceConfigName);
    dbQuery->bindValue(":device_type", deviceType);

    return dbQuery->exec();
}

//---------------------------------------------------------------------------
bool DatabaseUtils::removeDeviceParams(const QString & /*aDeviceConfigName*/) {
    Q_ASSERT(false);
    return false;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::cleanDevicesStatuses() {
    Q_ASSERT(false);
    return false;
}

//---------------------------------------------------------------------------
bool DatabaseUtils::addDeviceStatus(const QString &aDeviceConfigName,
                                    SDK::Driver::EWarningLevel::Enum aErrorLevel,
                                    const QString &aStatusString) {
    if (!hasDevice(aDeviceConfigName)) {
        if (!addDevice(aDeviceConfigName)) {
            return false;
        }
    }

    QMutexLocker lock(&m_AccessMutex);

    // Проверяем последний статус устройства. Если совпадает с текущим - не пишем его.
    QString queryMessage =
        QString("SELECT `level`, `description` FROM `device_status` WHERE `fk_device_id` = (SELECT "
                "`id` FROM `device` WHERE `name` = :config_name)");

    QScopedPointer<IDatabaseQuery> dbQuery(m_Database.createQuery(queryMessage));
    if (!dbQuery) {
        return false;
    }

    dbQuery->bindValue(":config_name", aDeviceConfigName);

    bool is = false;

    if (dbQuery->exec() && dbQuery->last()) {
        if (aErrorLevel ==
                static_cast<SDK::Driver::EWarningLevel::Enum>(dbQuery->value(0).toInt()) &&
            aStatusString == dbQuery->value(1).toString()) {
            is = true;
        }
    }

    if (!is) {
        queryMessage =
            "INSERT INTO `device_status` (`description`, `level`, `create_date`, `fk_device_id`)"
            " VALUES (:statusDescription, :level, :createDate, (SELECT `id` FROM `device` WHERE "
            "`name` = "
            ":config_name))";

        if (!dbQuery->prepare(queryMessage)) {
            return false;
        }

        dbQuery->bindValue(":statusDescription", aStatusString);
        dbQuery->bindValue(":level", aErrorLevel);
        dbQuery->bindValue(":createDate",
                           QDateTime::currentDateTime().toString(CIDatabaseProxy::DateFormat));
        dbQuery->bindValue(":config_name", aDeviceConfigName);

        return dbQuery->exec();
    }
    return true;
}

//---------------------------------------------------------------------------
void DatabaseUtils::removeUnknownDevice(const QStringList &aCurrentDevicesList) {
    auto formatDeviceList = [&]() -> QString {
        QStringList result;
        foreach (auto c, aCurrentDevicesList) {
            result << QString("'%1'").arg(c);
        }

        result << QString("'%1'").arg(QString(PPSDK::CDatabaseConstants::Devices::Terminal));

        return result.join(",");
    };

    QScopedPointer<IDatabaseQuery> dbQuery(m_Database.createQuery(
        QString("DELETE FROM `device` WHERE `name` NOT IN (%1)").arg(formatDeviceList())));

    if (dbQuery) {
        dbQuery->exec();
    }

    long affected = 0;
    m_Database.execDML(
        "DELETE FROM `device_param` WHERE `fk_device_id` NOT IN (select `id` from device)",
        affected);
    m_Database.execDML(
        "DELETE FROM `device_status` WHERE `fk_device_id` NOT IN (select `id` from device)",
        affected);
}

//---------------------------------------------------------------------------

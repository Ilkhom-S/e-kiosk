#pragma once

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QStringDecoder>
#include <QtCore/QThread>
#include <QtCore/QVariantMap>

class System_Info : public QThread {
    Q_OBJECT

private:
    QVariantMap sysInfo;

    void getSystem_Info() {
        // "----------------- OS ----------------- ";
        QVariantMap osInfo =
            getWmicInfo(QString("os get caption, version, csname, csdversion, osarchitecture"));

        // Removed isWinXP check for Qt 6 compatibility

        sysInfo["os"] = osInfo;

        // "----------------- CPU ----------------- ";
        QVariantMap cpuInfo = getWmicInfo("cpu get name, numberofcores, numberoflogicalprocessors");
        sysInfo["cpu"] = cpuInfo;

        // "----------------- Motherboard ----------------- ";
        QVariantMap mboardInfo =
            getWmicInfo("baseboard get product, manufacturer, version, serialnumber");
        sysInfo["mboard"] = mboardInfo;

        // "----------------- RAM ----------------- ";

        QVariantMap ram_Info = getWmicInfo(QString("path win32_physicalmemory get devicelocator, "
                                                  "manufacturer, capacity, speed, memorytype"),
                                          true);

        int ram_CapacityTotal = 0;

        for (auto &r : ram_Info.value("data").toList()) {
            QVariantMap ram = r.toMap();
            QString manufacturer = ram.value("manufacturer").toString();
            QString speed = ram.value("speed").toString();
            int memoryType = ram.value("memorytype").toInt();
            //            int sm_BiosMemorytype =
            //            ram.value("smbiosmemorytype").toInt();

            QString divider = ram_Info.value("manufacturer").toString() != "" ? "|" : "";

            if (!ram_Info.value("manufacturer").toString().contains(manufacturer)) {
                ram_Info["manufacturer"] =
                    ram_Info.value("manufacturer").toString() + divider + manufacturer;
            }

            if (!ram_Info.value("speed").toString().contains(speed)) {
                ram_Info["speed"] = ram_Info.value("speed").toString() + divider + speed;
            }

            int capacity = int(ram.value("capacity").toDouble() / (1024 * 1024));

            ram_Info["memorytype"] = ram_MemoryType(memoryType);

            ram_CapacityTotal += capacity;
        }

        ram_Info["capacity"] = ram_CapacityTotal;

        sysInfo["ram"] = ram_Info;

        // "----------------- HDD ----------------- ";

        QVariantMap diskInfo = getWmicInfo("diskdrive get model, size", true);

        if (diskInfo.value("data").toList().count() > 0) {
            diskInfo = diskInfo.value("data").toList().at(0).toMap();
        }

        diskInfo["serialnumber"] = getWmicInfo("path win32_physicalmedia where "
                                               "tag='\\\\\\\\.\\\\PHYSICALDRIVE0' get serialnumber")
                                       .value("serialnumber")
                                       .toString()
                                       .trimmed();

        // Removed isWinXP serial number processing for Qt 6 compatibility

        QVariantMap lDiskInfo = getWmicInfo("logicaldisk get caption, size, freespace", true);

        QVariantMap system_Disk;
        double freespace = 0;

        for (auto &ld : lDiskInfo.value("data").toList()) {
            QVariantMap lDisk = ld.toMap();
            QString lDiskCaption = lDisk.value("caption").toString();
            auto size = lDisk.value("size").toDouble() / (1024 * 1024 * 1024);
            auto lDiskFreeSpace = lDisk.value("freespace").toDouble() / (1024 * 1024 * 1024);

            QString divider = diskInfo.value("diskCaptions").toString() != "" ? ", " : "";

            diskInfo["diskCaptions"] =
                diskInfo["diskCaptions"].toString() + divider + lDiskCaption.at(0);

            // system disk
            if (lDiskCaption == QDir::rootPath().left(2)) {
                system_Disk["caption"] = lDiskCaption;
                system_Disk["size"] = QString::number(size, 'f', 1);
                system_Disk["freespace"] = QString::number(lDiskFreeSpace, 'f', 1);
            }

            freespace += lDiskFreeSpace;
        }

        diskInfo["freespace"] = QString::number(freespace, 'f', 1);
        diskInfo["size"] = qRound(diskInfo.value("size").toDouble() / (1024 * 1024 * 1024));

        sysInfo["disk"] = diskInfo;
        sysInfo["system_disk"] = system_Disk;
    }

    QVariantMap getWmicInfo(QString cmd, const bool getList = false) {

        QProcess proc;
        QString args = QString("%1 /value").arg(cmd);
        proc.setProcessChannelMode(QProcess::MergedChannels);
        proc.start("wmic", args.split(" "));
        proc.waitForStarted();
        proc.closeWriteChannel();

        QVariantMap info;

        while (proc.waitForReadyRead()) {
            while (proc.canReadLine()) {
                QByteArray output = proc.readAllStandardOutput();
                QStringDecoder decoder("IBM866");
                QString data = decoder(output);

                //            qDebug() << data;

                if (data.trimmed().contains("\r\r\n\r\r\n\r\r\n")) {
                    QStringList dataList = data.trimmed().split("\r\r\n\r\r\n\r\r\n");

                    QVariantList list;

                    for (auto &d : dataList) {
                        list.append(toMap(d));
                    }

                    info["data"] = list;
                } else {
                    if (getList) {
                        info["data"] = QVariantList() << toMap(data.trimmed());
                    } else {
                        info = toMap(data.trimmed());
                    }
                }
            }
        }

        msleep(500);

        return info;
    }

    QVariantMap toMap(QString data) {
        QVariantMap map;

        QStringList dataList = data.trimmed().split("\r\r\n");

        for (auto &d : dataList) {
            QStringList list = d.split("=");
            QString key = list.at(0).toLower();
            QString value = list.length() > 1 ? list.at(1) : "";
            map[key] = value;
        }

        return map;
    }

    QString ram_MemoryType(int type) {
        switch (type) {
        case 20:
            return "DDR";
        case 21:
            return "DDR2";
        case 22:
            return "DDR2 FB-DIMM";
        case 24:
            return "DDR3";
        case 25:
            return "FBD2";
        case 26:
            return "DDR4";
        default:
            return "UNKNOWN";
        }
    }

protected:
    void run() {
        getSystem_Info();

        emit emitSystem_Info(sysInfo);
    }

signals:
    void emitSystem_Info(QVariantMap data);
};

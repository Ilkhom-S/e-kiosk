#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QTextCodec>
#include <QtCore/QThread>
#include <QtCore/QVariantMap>
#include <Common/QtHeadersEnd.h>

class SystemInfo : public QThread {
    Q_OBJECT

  private:
    QVariantMap sysInfo;

    void getSystemInfo() {
        auto isWinXP = QSysInfo::windowsVersion() == QSysInfo::WV_XP;

        // "----------------- OS ----------------- ";
        QVariantMap osInfo = getWmicInfo(
            QString("os get caption, version, csname, csdversion%1").arg(isWinXP ? "" : ", osarchitecture"));

        if (isWinXP) {
            osInfo["osarchitecture"] = "x32";
        }

        sysInfo["os"] = osInfo;

        // "----------------- CPU ----------------- ";
        QVariantMap cpuInfo = getWmicInfo("cpu get name, numberofcores, numberoflogicalprocessors");
        sysInfo["cpu"] = cpuInfo;

        // "----------------- Motherboard ----------------- ";
        QVariantMap mboardInfo = getWmicInfo("baseboard get product, manufacturer, version, serialnumber");
        sysInfo["mboard"] = mboardInfo;

        // "----------------- RAM ----------------- ";

        QVariantMap ramInfo = getWmicInfo(QString("path win32_physicalmemory get devicelocator, "
                                                  "manufacturer, capacity, speed, memorytype"),
                                          true);

        int ramCapacityTotal = 0;

        for (auto &r : ramInfo.value("data").toList()) {
            QVariantMap ram = r.toMap();
            QString manufacturer = ram.value("manufacturer").toString();
            QString speed = ram.value("speed").toString();
            int memoryType = ram.value("memorytype").toInt();
            //            int smBiosMemorytype =
            //            ram.value("smbiosmemorytype").toInt();

            QString divider = ramInfo.value("manufacturer").toString() != "" ? "|" : "";

            if (!ramInfo.value("manufacturer").toString().contains(manufacturer)) {
                ramInfo["manufacturer"] = ramInfo.value("manufacturer").toString() + divider + manufacturer;
            }

            if (!ramInfo.value("speed").toString().contains(speed)) {
                ramInfo["speed"] = ramInfo.value("speed").toString() + divider + speed;
            }

            int capacity = int(ram.value("capacity").toDouble() / (1024 * 1024));

            ramInfo["memorytype"] = ramMemoryType(memoryType);

            ramCapacityTotal += capacity;
        }

        ramInfo["capacity"] = ramCapacityTotal;

        sysInfo["ram"] = ramInfo;

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

        if (isWinXP) {
            QString s = QByteArray::fromHex(diskInfo.value("serialnumber").toString().toLatin1());
            QString sn = "";

            if (s.length() > 2) {
                for (int i = 1; i <= s.length(); i++) {
                    if (i % 2 == 0) {
                        sn = sn + s.at(i - 1) + s.at(i - 2);
                    }
                }
            }

            diskInfo["serialnumber"] = sn.trimmed();
        }

        QVariantMap lDiskInfo = getWmicInfo("logicaldisk get caption, size, freespace", true);

        QVariantMap systemDisk;
        double freespace = 0;

        for (auto &ld : lDiskInfo.value("data").toList()) {
            QVariantMap lDisk = ld.toMap();
            QString lDiskCaption = lDisk.value("caption").toString();
            auto size = lDisk.value("size").toDouble() / (1024 * 1024 * 1024);
            auto lDiskFreeSpace = lDisk.value("freespace").toDouble() / (1024 * 1024 * 1024);

            QString divider = diskInfo.value("diskCaptions").toString() != "" ? ", " : "";

            diskInfo["diskCaptions"] = diskInfo["diskCaptions"].toString() + divider + lDiskCaption.at(0);

            // system disk
            if (lDiskCaption == QDir::rootPath().left(2)) {
                systemDisk["caption"] = lDiskCaption;
                systemDisk["size"] = QString::number(size, 'f', 1);
                systemDisk["freespace"] = QString::number(lDiskFreeSpace, 'f', 1);
            }

            freespace += lDiskFreeSpace;
        }

        diskInfo["freespace"] = QString::number(freespace, 'f', 1);
        diskInfo["size"] = qRound(diskInfo.value("size").toDouble() / (1024 * 1024 * 1024));

        sysInfo["disk"] = diskInfo;
        sysInfo["system_disk"] = systemDisk;
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
                auto codec = QTextCodec::codecForName("IBM866");
                QString data = codec->toUnicode(output);

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

    QString ramMemoryType(int type) {
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
        getSystemInfo();

        emit emitSystemInfo(sysInfo);
    }

  signals:
    void emitSystemInfo(QVariantMap data);
};


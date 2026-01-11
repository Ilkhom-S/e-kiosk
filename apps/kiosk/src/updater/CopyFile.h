#ifndef COPYFILE_H
#define COPYFILE_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMap>
#include <QThread>

class CopyFileQs : public QThread {
    Q_OBJECT

  public:
    CopyFileQs() {}

    bool debugger = false;
    QString senderName = "UPDATER";
    QMap<int, QMap<QString, QString> > copyXmlMap;
    QStringList allFilesList;

    void startToCopy() {
        try {
            QString startDir = "tmp";
            allFilesList.clear();
            allFilesList = getDirFiles(startDir);

            // Список создан делаем готовим к замене файлы
            QVariantMap filesUpdated;
            int idx = 0;

            for (QStringList::Iterator iter = allFilesList.begin(); iter != allFilesList.end();
                 ++iter) {
                QString vrmPath = *iter;
                int compex = vrmPath.indexOf("assets/");
                QString fileFrom = vrmPath.mid(compex);

                QFileInfo infoIn(*iter);
                QFile fileCopy;
                QString fileNameIn = infoIn.fileName();

                // Проверяем есть ли файлы команд
                if (fileNameIn == "Services.xml" || fileNameIn == "Groups.xml" ||
                    fileNameIn == "style.qss") {
                    filesUpdated[fileNameIn] = true;
                }

                if (fileNameIn == "EKiosk.exe") {
                    continue;
                }

                if (fileNameIn == "h-sheller.exe") {
                    if (fileCopy.exists(QDir::currentPath() + "/h-sheller.exe")) {
                        fileCopy.remove(QDir::currentPath() + "/h-sheller.exe");
                    }

                    if (fileCopy.copy("tmp/h-sheller.exe",
                                      QDir::currentPath() + "/h-sheller.exe")) {
                        if (debugger) qDebug() << "--- COPY FILE OK --- sheler";
                        emit emit_Loging(0, senderName,
                                         QString("Файл %1 успешно перемещен в директорию root...")
                                             .arg(infoIn.fileName()));
                    } else {
                        if (debugger) qDebug() << "--- COPY FILE ERROR ---sheler";
                        emit emit_Loging(
                            1, senderName,
                            QString("Файл %1 не удалось переместить в директорию root...")
                                .arg(infoIn.fileName()));
                    }
                    continue;
                }

                copyXmlMap[idx]["pathFrom"] = "tmp/" + fileFrom;
                copyXmlMap[idx]["pathTo"] = fileFrom;

                if (fileCopy.exists(copyXmlMap[idx]["pathFrom"])) {
                    if (debugger) qDebug() << "PRESENT";
                } else {
                    if (debugger) qDebug() << "NOT PRESENT";
                }

                if (debugger) qDebug() << "--- FILE-PATH TO --- " << copyXmlMap[idx]["pathTo"];

                if (fileCopy.exists(copyXmlMap[idx]["pathTo"])) {
                    if (debugger) qDebug() << "PRESENT";
                } else {
                    if (debugger) qDebug() << "NOT PRESENT";
                }

                if (fileCopy.exists(copyXmlMap[idx]["pathTo"])) {
                    fileCopy.remove(copyXmlMap[idx]["pathTo"]);
                }

                if (fileCopy.copy(copyXmlMap[idx]["pathFrom"], copyXmlMap[idx]["pathTo"])) {
                    if (debugger) qDebug() << "--- COPY FILE OK --- " << infoIn.fileName();
                    emit emit_Loging(0, senderName,
                                     QString("Файл %1 успешно перемещен в директорию assets...")
                                         .arg(infoIn.fileName()));
                } else {
                    if (debugger) qDebug() << "--- COPY FILE ERROR --- " << infoIn.fileName();
                    emit emit_Loging(
                        1, senderName,
                        QString("Файл %1 не удалось переместить в директорию assets...")
                            .arg(infoIn.fileName()));
                }

                idx++;
            }

            emit emit_FilesUpdated(filesUpdated);
            return;
        } catch (std::exception& e) {
            if (debugger) qDebug() << "Error recucrciyo " << QString(e.what());
            return;
        }
    }

    QStringList getDirFiles(const QString& dirName) {
        QDir dir(dirName);
        // Проверяем есть ли папка вообще
        if (!dir.exists()) {
            if (debugger) qDebug() << "No such directory : " << dir.dirName();
            return QStringList();
        }

        QStringList fileNames;

        // Берем список файлов из данной директории
        QStringList fileList = dir.entryList(QDir::Files);

        // Пробегаемся по файлам
        for (QStringList::Iterator fit = fileList.begin(); fit != fileList.end(); ++fit) {
            fileNames.append(dir.absolutePath() + "/" + *fit);
        }

        // Берем список каталогов
        QStringList dirList = dir.entryList(QDir::Dirs);

        // Удаляем ненужные item
        dirList.removeAll(".");
        dirList.removeAll("..");

        // Пробегаемся по каталогам
        for (QStringList::Iterator dit = dirList.begin(); dit != dirList.end(); ++dit) {
            // Обявляем новый каталог от текущего
            QDir curDir = dir;

            // Делаем переход
            curDir.cd(*dit);
            QStringList curList = getDirFiles(curDir.absolutePath());

            for (QStringList::Iterator it = curList.begin(); it != curList.end(); ++it) {
                fileNames.append(QFileInfo(*it).absoluteFilePath());
            }
        }

        return fileNames;
    }

    static void msleep(int ms) { QThread::msleep(ms); }

  protected:
    void run() {
        startToCopy();
        msleep(1000);
        emit copyComplite();
        return;
    }

  signals:
    void emit_FilesUpdated(QVariantMap data);
    void copyComplite();
    void emit_Loging(int sts, QString name, QString content);
};

#endif  // COPYFILE_H

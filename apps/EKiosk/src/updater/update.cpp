#include "update.h"

DownloadManager::DownloadManager() : QThread() {
    Debugger = 1;

    this->senderName = "UPDATER";

    bisyNow = false;
    abortTimer = new QTimer(this);
    abortTimer->setSingleShot(true);
    connect(abortTimer, SIGNAL(timeout()), SLOT(abortReply()));

    resendTimer = new QTimer(this);
    resendTimer->setSingleShot(true);
    connect(resendTimer, SIGNAL(timeout()), this, SLOT(startNextDownload()));

    copyFile = new CopyFileQs();
    connect(this->copyFile,
            SIGNAL(emit_Loging(int, QString, QString)),
            this,
            SIGNAL(emit_Loging(int, QString, QString)));
    connect(this->copyFile, SIGNAL(copyComplite()), this, SLOT(afterCopyAllFiles()));
    connect(this->copyFile,
            SIGNAL(emit_FilesUpdated(QVariantMap)),
            this,
            SIGNAL(emit_FilesUpdated(QVariantMap)));

    connect(this, SIGNAL(emit_reDown()), this, SLOT(reDownloadFile()));
    connect(this, SIGNAL(emit_downOneByOne()), SLOT(downloadOneByOneFile()));

    restartTimer = new QTimer(this);
    restartTimer->setInterval(15000);
    connect(this->restartTimer, SIGNAL(timeout()), this, SLOT(toRestartTimer()));
}

void DownloadManager::toRestartTimer() {
    if (!isUpdaterLocked()) {
        // То можно перезагружать ПО
        restartTimer->stop();

        // Завершение обновления
        this->compliteUpdateIn();

        // Тут надо вытащить размер файла EKiosk.exe из базы
        int appSize = this->getAppFileSize();

        QVariantMap data;
        data["app"] = "EKiosk";
        data["app_size"] = appSize;

        /// Замена файла
        emit emit_replaceApp("replace", data);
    }
}

int DownloadManager::getAppFileSize() {
    QSqlQuery selectHash(this->db_);

    QString strQuery = QString("SELECT size FROM local_files WHERE name = \"EKiosk.exe\" LIMIT 1;");

    if (!selectHash.exec(strQuery)) {
        // if(Debugger) qDebug() << "Error Select global_hash";
        return 0;
    }

    QSqlRecord record = selectHash.record();

    if (selectHash.next()) {
        int size = selectHash.value(record.indexOf("size")).toInt();
        return size;
    }
    return 0;
}

bool DownloadManager::isUpdaterLocked() {
    QSettings settings(settingsPath, QSettings::IniFormat);
    return settings.value("updater_lock", true).toBool();
}

void DownloadManager::setUpdatePointName(QString name) {
    FolderName = name;
}

void DownloadManager::setUrlServerIp(QString ip) {
    IpServer = ip;
}

void DownloadManager::setXmlFileName(QString name) {
    XmlName = name;
}

bool DownloadManager::checkHashMonitor(QString hash) {
    bool result = false;
    // Делаем проверку на отличие
    QString oldHash = this->getOldHash();
    if (hash != oldHash) {
        // Тут результат разные
        result = true;
        emit this->emit_Loging(0,
                               this->senderName,
                               QString("Hash_update_interface отличаются(старый - %1, новый - %2)")
                                   .arg(oldHash, hash));
        // Присваиваем хеш
        gblNewHash = hash;
    }

    return result;
}

void DownloadManager::startToUpdate() {
    emit this->emit_Loging(0, this->senderName, QString("Начинаем закачку файлов с сервера..."));

    // Обнуляем число повторений
    count_download = 0;

    // Ставим параметр updater занят
    this->bisyNow = true;

    // Обнуляем счетчик общих закачек
    this->totalCount = 0;

    // Тут надо вначале закачать xml файл со списком файлов
    tmpDirectory = "tmp/";

    // Файл который надо будет качать
    nowDownloadFileName = XmlName;

    // Первоначальное значение размера файла
    nowDownloadFileSize = 0;
    nowDownloadFileHash = "";

    // Проверяем есть ли папка
    //    this->createPathDirIfNotExist(tmpDirectory);

    emit this->emit_Loging(
        0, this->senderName, QString("Начинаем качать XML файл %1").arg(nowDownloadFileName));

    // Тут ставим на закачку
    this->append(QUrl(QString("%1/%2").arg(IpServer, nowDownloadFileName)));
}

void DownloadManager::append(QUrl url) {
    // Глобальный URL
    now_url = url;

    // Начинаем закачку не сразу а через 1 сек
    //    QTimer::singleShot(1, this, SLOT(startNextDownload()));
    resendTimer->start(1);

    // Увеличиваем число общех закачек на один
    totalCount++;
}

void DownloadManager::startNextDownload() {
    // if(Debugger) qDebug() << "-----Start to download----";

    // Проверяем есть ли такой каталог в каталоге TMP
    this->createPathDirIfNotExist(tmpDirectory);
    //    emit this->emit_Loging(0,this->senderName,QString("TMP PATH -
    //    %1").arg(tmpDirectory));

    // Надо проверить есть ли такая папка в assets
    if (this->nowDownloadFilePath != "")
        this->createPathDirIfNotExist(this->nowDownloadFilePath);

    count_download++;
    // if(Debugger) qDebug() << "Count of Download ---- " << count_download;

    // Путь куда надо закачать файл
    QString filename = tmpDirectory + nowDownloadFileName;
    // if(Debugger) qDebug() << "File name ---- " << filename;
    // Флаг записи активен(QIODevice::WriteOnly)
    fileWrite = true;

    // Проверяем на существование файла
    /// Если это не xml Файл со списком файлов для закачки
    if (this->nowDownloadFileName != this->XmlName) {
        if (output.exists(filename)) {
            // if(Debugger) qDebug() << "----- Enter into file exist -----";
            // Объект для проверки файла
            QString hash = fileChecksum(filename, QCryptographicHash::Md5);

            // Если в тмп уже есть файл то даем сигнал что закачка есть
            if (hash != "" && hash == nowDownloadFileHash) {
                this->downloadFileResponse();
                return;
            }

            /// Если размер тмп файла меньше реального размера файла то надо докачать
            if (hash != nowDownloadFileHash) {
                fileWrite = true; // QIODevice::Append;
                                  // if(Debugger) qDebug() << "----- fileWrite = false; -----";
            }
        }
    }

    // Присваиваем наименование дла создание локального файла
    output.setFileName(filename);

    // Готовим заголовок для закачки
    QNetworkRequest request(now_url);

    // Смотрим параметр перезаписи
    if (fileWrite) {
        /// Если надо тупа перезаписать или создать файл открываем с параметром
        /// (WriteOnly)
        if (!output.open(QIODevice::WriteOnly)) {
            fprintf(stderr,
                    "Problem opening save file '%s' for download '%s': %s\n",
                    qPrintable(filename),
                    now_url.toEncoded().constData(),
                    qPrintable(output.errorString()));

            // Не возможно создать файл
            emit this->emit_Loging(2,
                                   this->senderName,
                                   QString("Не возможно создать файл %1").arg(nowDownloadFileName));
            return; // Пропускаем закачку
        } else {
            // if(Debugger) qDebug() << "----- output.open(QIODevice::WriteOnly)
            // -----";
        }
    } else {
        /// Если надо дозаписать файл открываем с параметром (Append)
        if (!output.open(QIODevice::Append)) {
            fprintf(stderr,
                    "Problem opening save file '%s' for download '%s': %s\n",
                    qPrintable(filename),
                    now_url.toEncoded().constData(),
                    qPrintable(output.errorString()));

            emit this->emit_Loging(
                2,
                this->senderName,
                QString("Не возможно открыть файл для до записи %1").arg(nowDownloadFileName));
            return; // Пропускаем закачку
        } else {
            // if(Debugger) qDebug() << "----- output.open(QIODevice::Append) -----";
        }

        /// Готовим параметры для заголовка файла закачки
        QFileInfo vrm_Inf(filename);
        int size = vrm_Inf.size(); // размер частично загруженного файла
        request.setRawHeader(QByteArray("Range"), QString("bytes=%1-").arg(size).toLatin1());
        // if(Debugger) qDebug() << "----- request.setRawHeader ----- size - " <<
        // size;
    }

    // Необходимо создать abort так как закачка может подвиснуть
    /// задаем время
    int timeAbort = 480; // sec
    if (this->nowDownloadFileName == this->XmlName)
        timeAbort = 180; // 3 min
    if (this->nowDownloadFileName == "EKiosk.exe")
        timeAbort = 900; // 8 min

    abortTimer->start(timeAbort * 1000);

    // Даем реплаю параметры для закачки файла
    currentDownload = manager.get(request);

    // Связка хода закачки с отображением
    connect(currentDownload,
            SIGNAL(downloadProgress(qint64, qint64)),
            SLOT(downloadProgress(qint64, qint64)));

    // Связка закачка завершена
    connect(currentDownload, SIGNAL(finished()), SLOT(downloadFinished()));

    // Записываем в файл когда часть файла загруженна
    connect(currentDownload, SIGNAL(readyRead()), SLOT(downloadReadyRead()));

    // Печатаем запрос
    printf("Downloading %s...\n", now_url.toEncoded().constData());
    downloadTime.start();
}

void DownloadManager::downloadFileResponse() {
    emit this->emit_Loging(
        0,
        this->senderName,
        QString("Файл %1 успешно закачен на локальный компьютер...").arg(nowDownloadFileName));
    // if(Debugger) qDebug() << "SERVER RESPONSE FILE >> " +nowDownloadFileName;

    // Даем статус что файл закачен
    this->updateRecordFile(nowDownloadFileId, QString("status"), QString("0"));

    // Качаем следующий файл
    this->downloadOneByOneFile();
}

void DownloadManager::abortReply() {
    if (currentDownload->isFinished()) {
        /// Ничего не делаем
    } else {
        // if(Debugger) qDebug() << "-----Abort download now----";
        this->currentDownload->abort();
        /// Надо повторно отправить закачку
        if (this->count_download < 20)
            resendTimer->start(20000);
        else {
            this->reDownloadFile();
        }
    }
}

void DownloadManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    // Даем параметры для отображения процесса
    progressBar.setStatus(bytesReceived, bytesTotal);

    // Вычитываем скорость закачки
    double speed = bytesReceived * 1000.0 / downloadTime.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024 * 1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024 * 1024;
        unit = "MB/s";
    }

    progressBar.setMessage(QString::fromLatin1("%1 %2").arg(speed, 3, 'f', 1).arg(unit));
    progressBar.update();
}

void DownloadManager::downloadFinished() {
    // Очищаем отображение загрузки
    progressBar.clear();

    // Закрываем записанный файл
    output.close();

    // когда файл не существует на сервере
    if (currentDownload->error() == 203) {
        QFile::remove("tmp/" + nowDownloadFileAll);
        this->downloadFileResponse();
    } else {

        if (currentDownload->error()) {
            /// Если есть ошибка при закачивании файла

            // Печатаем в консоль ошибку
            fprintf(stderr, "Failed: %s\n", qPrintable(currentDownload->errorString()));

            // Можно отрубить чтобы главная прога перезапустила сеть и вновь запустила
            // апдейтер
            if (nowDownloadFileName == this->XmlName) {
                emit this->emit_Loging(1,
                                       this->senderName,
                                       QString("Ошибка при закачки XML файла %1, "
                                               "повторная закачка через 5 мин.")
                                           .arg(nowDownloadFileName));
            }
            /// Если число закачек не превышает 5
            if (count_download < 20) {
                emit this->emit_Loging(0,
                                       this->senderName,
                                       QString("Попытка №%1 для закачки файла %2 "
                                               "(повторная закачка через 20 сек.)")
                                           .arg(count_download)
                                           .arg(nowDownloadFileName));
                // if(Debugger) qDebug() << "----- count_download < 5 -----";
                this->resendTimer->start(20000);
                return;
            } else {
                emit this->emit_Loging(
                    1,
                    this->senderName,
                    QString("Превышено число попыток для закачки файла %1 (повторная "
                            "закачка через 5 мин.)")
                        .arg(nowDownloadFileName));
                // На всякий случай удаляем файл
                /// Надо подумать надо удалять или нет
                //            this->deleteErrorDownloadFile(nowDownloadFileId);
                this->reDownloadFile();

                return;
            }
        } else {

            /// Ошибок типо нет но может и вырубили соединение

            printf("-------Download return.-------\n");

            // Проверяем что
            if (nowDownloadFileName == this->XmlName) {
                this->openDocumentXml();
                return;
            }

            QString filename = tmpDirectory + nowDownloadFileName;

            QString hash = fileChecksum(filename, QCryptographicHash::Md5);

            if (hash != nowDownloadFileHash) {
                if (count_download < 20) {
                    emit this->emit_Loging(
                        0,
                        this->senderName,
                        QString("Хеш файла %1 не совпадает (делаем докачку через 20 сек...)")
                            .arg(nowDownloadFileName));
                    this->resendTimer->start(20000);
                    return;
                } else {
                    this->reDownloadFile();
                    return;
                }
            }
            // if(Debugger) qDebug() << "------------START NEXT Download-------------";
            this->downloadFileResponse();
        }
    }

    currentDownload->deleteLater();
}

void DownloadManager::reDownloadFile() {
    this->bisyNow = false;
    count_download = 0;
    this->abortTimer->stop();
    this->resendTimer->stop();
}

void DownloadManager::downloadReadyRead() {
    output.write(currentDownload->readAll());
}

void DownloadManager::openDocumentXml() {
    // Количество тегов или файлов в xml
    countFileTag = 0;

    // Документ для парсинга xml
    QDom_Document doc;
    QFile file(tmpDirectory + this->XmlName);

    // Проверяем можно ли открыть файл
    if (!file.open(QIODevice::ReadOnly)) {

        /// Файл открыть нельзя удаляем его и начинаем качать заново
        // Удаляем файл
        file.remove(tmpDirectory + this->XmlName);
        // Начинаем закачку через 10 сек.
        if (this->count_download < 5) {
            this->resendTimer->start(10000);
            return;
        } else {
            this->reDownloadFile();
        }
        emit this->emit_Loging(
            1,
            this->senderName,
            QString("Не возможно открыть %1 файл для чтения.").arg(tmpDirectory + this->XmlName));
        return;
    }
    // Файл открыт успешно
    emit this->emit_Loging(
        0, this->senderName, QString("Файл %1 открыт успешно.").arg(this->XmlName));
    qDebug() << QString(" =====> File %1%2 OPENED;").arg(tmpDirectory).arg(this->XmlName);

    // Проверяем на целостность xml файл
    /// Если нет целостности начинаем качать заново

    if (!doc.setContent(&file)) {
        // Закрываем файл
        file.close();
        // Целостность документа нарушена
        // if(Debugger) qDebug() << "--------Parse Error "+this->XmlName+"--------";
        // Удаляем файл
        file.remove(tmpDirectory + this->XmlName);
        // Начинаем закачку через 10 сек.
        if (this->count_download < 5)
            this->resendTimer->start(10000);
        else {
            this->reDownloadFile();
        }
        emit this->emit_Loging(
            1, this->senderName, QString("Не возможно отпарсить %1 файл.").arg(this->XmlName));
        return;
    } else {
        // Xml Файл открыт успешно
        // if(Debugger) qDebug() << this->XmlName << " Opened OK!!!";
        emit this->emit_Loging(
            0, this->senderName, QString("Начинаем парсить %1 файл.").arg(this->XmlName));
        this->ServerXmlMap.clear();
        QDom_Element dom_Element = doc.documentElement();
        traverseNode(dom_Element);
    }

    // Файл отпарсен успешно
    qDebug() << QString(" =====> File %1%2 CLOSED;").arg(tmpDirectory).arg(this->XmlName);
    emit this->emit_Loging(
        0, this->senderName, QString("Файл %1 успешно отпарсен.").arg(this->XmlName));

    // Закрываем файл
    file.close();

    // Удаляем файл
    file.remove(tmpDirectory + this->XmlName);

    qDebug() << QString(" =====> File %1%2 REMOVED;").arg(tmpDirectory).arg(this->XmlName);

    // говорим что все отпарсили
    /// Проверяем есть ли файлы на закачку
    this->addToDatabaseFile();
}

void DownloadManager::run() {
    bool updateFile = false;
    QSqlQuery DataQuery(this->db_);

    QString strQuery = QString("SELECT id,hash,name,status FROM local_files "
                               "WHERE path = \"%1\" AND name = \"%2\" LIMIT 1;");

    int countTag = this->ServerXmlMap.count();
    if (countFileTag != countTag) {
        countFileTag = countTag;
    }

    bool updateSheller = false;

    // Начинаем пробегаться по базе беря параметры путь и имя файла
    for (int i = 1; i <= countFileTag; i++) {
        if (!DataQuery.exec(strQuery.arg(ServerXmlMap[i]["path"], ServerXmlMap[i]["name"]))) {
            // if(Debugger) qDebug() << "Error Select hash for update status";
            // if(Debugger) qDebug() << DataQuery.lastError();
            /// Ошибка выборки данных с базы с файлами
            emit this->emit_Loging(
                2, this->senderName, QString("Ошибка выполнения запроса к базе с файлами."));
            return;
        }

        QSqlRecord record = DataQuery.record();

        if (DataQuery.next()) {
            QString hash = DataQuery.value(record.indexOf("hash")).toString();
            QString name = DataQuery.value(record.indexOf("name")).toString();
            int sts = DataQuery.value(record.indexOf("status")).toInt();

            int id = DataQuery.value(record.indexOf("id")).toInt();

            if (ServerXmlMap[i]["hash"] != hash || sts == 1) {
                // Обновляем строку на статус 1 для дальнейшей закачки
                if (this->updateRecordFile(id, QString("status"), QString("1"))) {
                    /// Если строка состояния обновлена успешно то обновляем остальны
                    /// данные
                    if (this->updateRecordForMore(
                            id, ServerXmlMap[i]["hash"], ServerXmlMap[i]["size"].toInt())) {
                        qDebug() << QString("FILE %1 WENT TO UPDATE;").arg(name);
                        updateFile = true;

                        if (name == "h-sheller.exe") {
                            updateSheller = true;
                        }
                    } else {
                        qDebug() << "ERROR UPDATE RECORD FOR MORE;";
                    }
                } else
                    qDebug() << "FAELD TO UPDATE " + name;
            }
        } else {
            // Создаем новую запись со статусом 1 для дальнейшей закачки
            if (this->insertNewRecordFile(ServerXmlMap[i]["path"],
                                          ServerXmlMap[i]["name"],
                                          ServerXmlMap[i]["size"],
                                          ServerXmlMap[i]["hash"])) {
                qDebug() << QString("ADD NEW FILE %1 FOR DOWNLOAD ").arg(ServerXmlMap[i]["name"]);
                updateFile = true;

                if (ServerXmlMap[i]["name"] == "h-sheller.exe") {
                    updateSheller = true;
                }
            } else {
                qDebug() << QString("FAILED TO ADD NEW RECORD");
            }
        }
    }
    if (updateFile) {
        /// Есть доступные обновления

        if (updateSheller) {
            emit emit_killSheller();

            msleep(3000);
        }

        emit this->emit_downOneByOne();
        return;
    } else {
        /// Нет доступных обновлений
        updateGlobalHash();
        emit emit_Loging(0,
                         this->senderName,
                         QString("Нет новых файлов на сервере (обновляем hash и "
                                 "закрываем UPDATER)"));

        //        this->reDownloadFile();
        emit emit_reDown();

        return;
        /// Exit
    }
    return;
}

void DownloadManager::addToDatabaseFile() {
    this->start();
}

void DownloadManager::downloadOneByOneFile() {

    QSqlQuery DataQuery(this->db_);

    QString strQuery = QString("SELECT * FROM local_files WHERE status = 1 LIMIT 1;");

    if (!DataQuery.exec(strQuery)) {
        // if(Debugger) qDebug() << "Error SELECT record file";
        // if(Debugger) qDebug() << DataQuery.lastError();
        return;
    }

    QSqlRecord record = DataQuery.record();

    if (DataQuery.next()) {
        // Обнуляем количество повторов
        count_download = 0;

        this->nowDownloadFileName = DataQuery.value(record.indexOf("name")).toString();
        this->nowDownloadFileSize = DataQuery.value(record.indexOf("size")).toInt();
        this->nowDownloadFileHash = DataQuery.value(record.indexOf("hash")).toString();
        this->nowDownloadFileId = DataQuery.value(record.indexOf("id")).toInt();
        this->nowDownloadFilePath = DataQuery.value(record.indexOf("path")).toString();

        // Что именно качаем
        nowDownloadFileAll = nowDownloadFilePath + nowDownloadFileName;

        // Путь куда будет идти закачка
        tmpDirectory = "tmp/" + this->nowDownloadFilePath;

        QString vrm_Url =
            QString("%1/%2/%3").arg(this->IpServer, this->FolderName, nowDownloadFileAll);
        // if(Debugger) qDebug() << vrm_Url;
        emit this->emit_Loging(0,
                               this->senderName,
                               QString("Начинаем качать файл %1 объемом %2 Byte в директорию %3")
                                   .arg(nowDownloadFileName)
                                   .arg(nowDownloadFileSize)
                                   .arg(this->tmpDirectory));
        this->append(QUrl(vrm_Url));
        return;
    } else {
        /// Тут надо сделать перемещение файлов из папки tmp в реальную директорию
        this->copyFile->start();

        return;
        /// Exit
    }
}

void DownloadManager::afterCopyAllFiles() {
    //    this->copyFile->terminate();

    QFileInfo infoNamePath;
    infoNamePath.setFile("tmp/EKiosk.exe");

    // Проверяем есть ли файл EKiosk.exe в директории tmp
    if (infoNamePath.exists()) {
        /// Запускаем таймер перезагрузки EKiosk-a
        restartTimer->start();
    } else {
        // Делаем терминейт копира

        /// Удаление папки tmp
        QDir dir;
        if (dir.rename(
                "tmp",
                QString("tmp_%1").arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"))))
            qDebug() << "--- DIRECTORY tmp RENAMED ---";
        else
            qDebug() << "--- DIRECTORY tmp NOT RENAMED ---";

        this->compliteUpdateIn();
    }

    return;
}

void DownloadManager::compliteUpdateIn() {
    /// Удаление папки tmp
    //    this->dirCont.remove("tmp");

    // Обновляем глобальный hash
    this->updateGlobalHash();
    emit this->emit_Loging(0,
                           this->senderName,
                           QString("База не содержит новых файлов (обновляем "
                                   "hash и закрываем UPDATER)"));

    this->reDownloadFile();
}

void DownloadManager::createFileList(QString path) {
    QFileInfo infoIn;
    infoIn.setFile(path);
    if (infoIn.isDir()) {

        QStringList list = this->dirCont.entryList(QDir::AllDirs | QDir::Files);
        for (int i = 0; i < list.count(); i++) {
            qDebug() << "--- LIST --- " << list.at(i);
            this->createFileList(list.at(i));
        }
    } else {
        /// Если это файл берем его путь
        if (infoIn.isFile()) {
            //            count_i_files ++;
            //            this->copyXmlMap[count_i_files]["path"] = path;
            qDebug() << "--- TO MAP --- " << path;
        }
    }
}

void DownloadManager::reCopyAllFiles() {
    try {
        QString startDir = "tmp";
        allFilesList.clear();
        allFilesList = getDirFiles(startDir);

        // Список создан делаем готовим к замене файлы
        int count_i_files = 0;
        for (QStringList::Iterator iter = allFilesList.begin(); iter != allFilesList.end();
             ++iter) {
            //            qDebug() << "--- FILES PATH --- " << *iter;
            QString vrm_Path = *iter;
            int compex = vrm_Path.indexOf("assets/");
            QString fileFrom = vrm_Path.mid(compex);
            //            qDebug() << "--- FILES PATH --- " << fileFrom;
            QFileInfo infoIn(*iter);
            QFile fileCopy;
            if (infoIn.fileName() != "EKiosk.exe" && infoIn.fileName() != "h-sheller.exe") {
                copyXmlMap[count_i_files]["pathFrom"] = "tmp/" + fileFrom;
                copyXmlMap[count_i_files]["pathTo"] = fileFrom;
                qDebug() << "--- FILE-PATH FROM --- " << copyXmlMap[count_i_files]["pathFrom"];

                if (fileCopy.exists(copyXmlMap[count_i_files]["pathFrom"])) {
                    qDebug() << "PRESENT";
                } else {
                    qDebug() << "NOT PRESENT";
                }

                qDebug() << "--- FILE-PATH TO --- " << copyXmlMap[count_i_files]["pathTo"];

                if (fileCopy.exists(copyXmlMap[count_i_files]["pathTo"])) {
                    qDebug() << "PRESENT";
                } else {
                    qDebug() << "NOT PRESENT";
                }

                if (fileCopy.exists(copyXmlMap[count_i_files]["pathTo"])) {
                    fileCopy.remove(copyXmlMap[count_i_files]["pathTo"]);
                }

                if (fileCopy.copy(copyXmlMap[count_i_files]["pathFrom"],
                                  copyXmlMap[count_i_files]["pathTo"])) {
                    qDebug() << "--- COPY FILE OK --- " << infoIn.fileName();
                    emit this->emit_Loging(
                        0,
                        this->senderName,
                        QString("Файл %1 успешно перемещен в директорию assets...")
                            .arg(infoIn.fileName()));
                } else {
                    qDebug() << "--- COPY FILE ERROR --- " << infoIn.fileName();
                    emit this->emit_Loging(
                        1,
                        this->senderName,
                        QString("Файл %1 не удалось переместить в директорию assets...")
                            .arg(infoIn.fileName()));
                }

                count_i_files++;
            }
        }

        return;
    } catch (std::exception &e) {
        if (Debugger)
            qDebug() << "Error recucrciyo " << QString(e.what());
        return;
    }
}

QStringList DownloadManager::getDirFiles(const QString &dirName) {
    QDir dir(dirName);
    // Проверяем есть ли папка вообще
    if (!dir.exists()) {
        qDebug() << "No such directory : " << dir.dirName();
        QStringList lst;
        return lst;
    }
    QStringList fileNames;

    // Берем список файлов из данной директории
    QStringList fileList = dir.entryList(QDir::Files);

    // Пробегаемся по файлам
    for (QStringList::Iterator fit = fileList.begin(); fit != fileList.end(); ++fit)
        fileNames.append(dir.absolutePath() + "/" + *fit);

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
        for (QStringList::Iterator it = curList.begin(); it != curList.end(); ++it)
            fileNames.append(QFileInfo(*it).absoluteFilePath());
    }
    return fileNames;
}

void DownloadManager::updateGlobalHash() {
    QSqlQuery insertQuery(this->db_);

    QString strUpdate =
        QString("UPDATE global_hash SET hash = \"%1\" WHERE id = 1").arg(this->gblNewHash);

    if (!insertQuery.exec(strUpdate)) {
        // if(Debugger) qDebug() << "Error update global hash";
        // if(Debugger) qDebug() << insertQuery.lastError();
        return;
    }
    return;
}

bool DownloadManager::updateRecordFile(int id, QString fieldName, QString value) {
    QSqlQuery updateQuery(this->db_);

    QString strUpdate = QString("UPDATE local_files SET %1 = \"%2\" WHERE id = %3")
                            .arg(fieldName, value, QString("%1").arg(id));

    if (!updateQuery.exec(strUpdate)) {
        // if(Debugger) qDebug() << "Error Update record file";
        // if(Debugger) qDebug() << updateQuery.lastError();
        return false;
    }
    return true;
}

bool DownloadManager::updateRecordForMore(int id, QString hash, int size) {
    QSqlQuery updateQuery(this->db_);

    QString strUpdate = QString("UPDATE local_files SET hash = \"%1\", size = %2 WHERE id = %3")
                            .arg(hash)
                            .arg(size)
                            .arg(id);

    if (!updateQuery.exec(strUpdate)) {
        // if(Debugger) qDebug() << "Error Update record file";
        // if(Debugger) qDebug() << updateQuery.lastError();
        return false;
    }
    return true;
}

bool DownloadManager::insertNewRecordFile(QString path, QString name, QString size, QString hash) {
    QSqlQuery insertQuery(this->db_);

    QString strUpdate = QString("INSERT INTO local_files (id,path,name,size,hash,status)"
                                "VALUES (NULL,\"%1\",\"%2\",%3,\"%4\",1);")
                            .arg(path, name, size, hash);

    if (!insertQuery.exec(strUpdate)) {
        // if(Debugger) qDebug() << "Error insert record file";
        // if(Debugger) qDebug() << insertQuery.lastError();
        return false;
    }
    return true;
}

void DownloadManager::traverseNode(const QDom_Node &node) {
    QDom_Node dom_Node = node.firstChild();

    while (!dom_Node.isNull()) {
        if (dom_Node.isElement()) {
            QDom_Element dom_Element = dom_Node.toElement();
            QString strTag = dom_Element.tagName();

            if (strTag == "file") {
                countFileTag++;

                ServerXmlMap[countFileTag]["path"] =
                    dom_Element.attribute("path", "")
                        .right(dom_Element.attribute("path", "").length() - 1);
                ServerXmlMap[countFileTag]["name"] = dom_Element.attribute("name", "");
                ServerXmlMap[countFileTag]["size"] = dom_Element.attribute("size", "");
                ServerXmlMap[countFileTag]["hash"] = dom_Element.text();
            }
        }
        traverseNode(dom_Node);
        dom_Node = dom_Node.nextSibling();
    }
}

void DownloadManager::createDirIfNotExist(QString dirPath) {
    QFile info;
    if (!info.exists(dirPath)) {
        // if(Debugger) qDebug() << "isNotDir";
        // Тут надо создать папку
        this->dirCont.mkdir(dirPath);
    }
}

void DownloadManager::createPathDirIfNotExist(QString dirPath) {
    QFile info;
    if (!info.exists(dirPath)) {
        // if(Debugger) qDebug() << "isNotPathDir";
        // Тут надо создать папку
        this->dirCont.mkpath(dirPath);
    }
}

void DownloadManager::setDbName(QSqlDatabase &db) {
    this->db_ = db;
}

QString DownloadManager::getOldHash() {
    QSqlQuery selectHash(this->db_);

    QString strQuery = QString("SELECT hash FROM global_hash WHERE id = 1 LIMIT 1;");

    if (!selectHash.exec(strQuery)) {
        // if(Debugger) qDebug() << "Error Select global_hash";
        return "";
    }

    QSqlRecord record = selectHash.record();

    if (selectHash.next()) {
        QString global_hash = selectHash.value(record.indexOf("hash")).toString();
        // if(Debugger) qDebug() << "===================================";
        // if(Debugger) qDebug() << QString("global_hash = %1").arg(global_hash);
        // if(Debugger) qDebug() << "===================================";
        return global_hash;
    }
    return "";
}

QString DownloadManager::fileChecksum(const QString &fileName,
                                      QCryptographicHash::Algorithm hashAlgorithm) {
    QFile sourceFile(fileName);
    qint64 fileSize = sourceFile.size();
    const qint64 bufferSize = 10240;

    if (sourceFile.open(QIODevice::ReadOnly)) {
        char buffer[bufferSize];
        int bytesRead;
        int readSize = qMin(fileSize, bufferSize);

        QCryptographicHash hash(hashAlgorithm);
        while (readSize > 0 && (bytesRead = sourceFile.read(buffer, readSize)) > 0) {
            fileSize -= bytesRead;
            hash.addData(buffer, bytesRead);
            readSize = qMin(fileSize, bufferSize);
        }

        sourceFile.close();
        return QString(hash.result().toHex());
    }
    return QString();
}

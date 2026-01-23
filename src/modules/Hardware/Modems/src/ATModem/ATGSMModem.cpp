/* @file AT-совместимый модем. */

// STL
#include <algorithm>
#include <cmath>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QElapsedTimer>
#include <QtCore/QRegularExpression>
#include <Common/QtHeadersEnd.h>

// System
#include "Hardware/Modems/ModemStatusesDescriptions.h"

// Project
#include "ATGSMModem.h"
#include "smspdudecoder.h"
#include "smspduencoder.h"

using namespace SDK::Driver;

//--------------------------------------------------------------------------------
ATGSMModem::ATGSMModem() {
    mGsmDialect = AT::EModemDialect::DefaultAtGsm;
    mStatusCodesSpecification = DeviceStatusCode::PSpecifications(new ModemStatusCode::CSpecifications());

    mDeviceName = CATGSMModem::DefaultName;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getOperator(QString &aOperator) {
    if (!checkConnectionAbility()) {
        return false;
    }

    enableLocalEcho(false);
    processCommand(AT::Commands::CopsMode);

    QByteArray answer;
    QRegularExpression regExp("\".+\"");
    bool result = false;

    QRegularExpressionMatch match = regExp.match(answer);
    if (processCommand(AT::Commands::COPS, answer) && match.hasMatch()) {
        // Парсим имя оператора.
        aOperator = match.captured(0).remove("\"");

        toLog(LogLevel::Normal, QString("Operator name: %1.").arg(aOperator));
        result = true;
    }

    mIOPort->close();

    return result;
}

//--------------------------------------------------------------------------------
void ATGSMModem::setDeviceConfiguration(const QVariantMap &aConfiguration) {
    ATModemBase::setDeviceConfiguration(aConfiguration);

    if (aConfiguration.contains(CHardwareSDK::ModelName)) {
        setDeviceName(aConfiguration.value(CHardwareSDK::ModelName).toString().toLatin1());
    }
}

//--------------------------------------------------------------------------------
void ATGSMModem::setDeviceName(const QByteArray &aFullName) {
    ATModemBase::setDeviceName(aFullName);

    if (mDeviceName.contains("Cinterion", Qt::CaseInsensitive) ||
        mDeviceName.contains("SIEMENS", Qt::CaseInsensitive)) {
        mGsmDialect = AT::EModemDialect::Siemens;
    } else if (mDeviceName.contains("SIMCOM", Qt::CaseInsensitive)) {
        mGsmDialect = AT::EModemDialect::SimCom;

        QRegularExpression revisionRegex("(\\s*Revision.*)");
        mDeviceName.remove(revisionRegex);
        mModemConfigTimeout = CATGSMModem::Timeouts::SimCom::Config;
    } else if (mDeviceName.contains("huawei", Qt::CaseInsensitive)) {
        mGsmDialect = AT::EModemDialect::Huawei;

        QString value;

        if (parseFieldInternal(aFullName, "Manufacturer", value)) {
            mDeviceName = value;
        }

        if (parseFieldInternal(aFullName, "Model", value)) {
            mDeviceName += " " + value;
        }
    } else if (mDeviceName.contains(QRegularExpression("MF\\d{3}", QRegularExpression::CaseInsensitiveOption)) ||
               mDeviceName.contains("ZTE", Qt::CaseInsensitive)) {
        mGsmDialect = AT::EModemDialect::ZTE;

        QString value;

        if (parseFieldInternal(aFullName, "Manufacturer", value)) {
            mDeviceName = value;

            if (parseFieldInternal(aFullName, "Model", value)) {
                mDeviceName += " " + value;
            }
        } else {
            QRegularExpression zteRegex("MF\\d{3}", QRegularExpression::CaseInsensitiveOption);
            QRegularExpressionMatch match = zteRegex.match(mDeviceName);
            if (match.hasMatch()) {
                mDeviceName = "ZTE " + mDeviceName.mid(match.capturedStart(), 5);
            }
        }
    }
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getInfo(QString &aInfo) {
    toLog(LogLevel::Normal, "Retrieve modem info.");

    if (!checkConnectionAbility()) {
        return false;
    }

    enableLocalEcho(false);

    getSIMData(AT::Commands::IMEI);
    getSIMData(AT::Commands::IMSI);
    getSIMData(AT::Commands::CNUM);

    QByteArray answer;
    QString value;

    if (processCommand(AT::Commands::Revision, answer)) {
        QString data = parseFieldInternal(answer, "Revision", value) ? value : answer;
        setDeviceParameter(CDeviceData::Revision, data);
    }

    switch (mGsmDialect) {
        case AT::EModemDialect::Siemens: {
            getSIMData(AT::Commands::Siemens::SIMID);

            if (getSiemensCellList(value)) {
                setDeviceParameter(CDeviceData::Modems::GSMCells, value);
            }

            break;
        }
        //--------------------------------------------------------------------------------
        case AT::EModemDialect::Huawei: {
            getSIMData(AT::Commands::Huawei::SIMID);

            break;
        }
        //--------------------------------------------------------------------------------
        case AT::EModemDialect::ZTE: {
            getSIMData(AT::Commands::ZTE::SIMID);

            break;
        }
        //--------------------------------------------------------------------------------
        case AT::EModemDialect::SimCom: {
            if (getSimCOMCellList(value)) {
                setDeviceParameter(CDeviceData::Modems::GSMCells, value);
            }

            break;
        }
        default:
            break;
    }

    mIOPort->close();

    aInfo.clear();

    foreach (const QString &aKey, CATGSMModem::SIMRequestInfo.getDeviceDataKeys()) {
        aInfo += QString("\n%1: %2").arg(aKey).arg(getDeviceParameter(aKey).toString());
    }

    return !aInfo.isEmpty();
}

//--------------------------------------------------------------------------------
void ATGSMModem::getSIMData(const QByteArray &aCommand) {
    QByteArray answer;
    processCommand(aCommand, answer);

    CATGSMModem::SSIMRequestInfo SIMRequestInfo = CATGSMModem::SIMRequestInfo[aCommand];

    // 1. Так как regexpData уже QString, передаем её напрямую в конструктор.
    QRegularExpression regExp(SIMRequestInfo.regexpData);

    // 2. Ответ от модема (QByteArray) конвертируем в QString для поиска.
    // Используем перегрузку, которая принимает QByteArray целиком.
    QString answerStr = QString::fromUtf8(answer);

    // 3. Выполняем поиск
    QRegularExpressionMatch match = regExp.match(answerStr);

    if (match.hasMatch()) {
        // Получаем захваченную группу (0 — всё совпадение целиком)
        QString result = match.captured(0);

        if (SIMRequestInfo.swapCharPair) {
            for (int i = 0; i + 1 < result.size(); i += 2) {
                QChar a = result[i];
                result[i] = result[i + 1];
                result[i + 1] = a;
            }
        }

        setDeviceParameter(SIMRequestInfo.name, result);
    }
}

//--------------------------------------------------------------------------------
bool ATGSMModem::parseFieldInternal(const QByteArray &aBuffer, const QString &aFieldName, QString &aValue) {
    // 1. Создаем регулярное выражение с опцией игнорирования регистра.
    // Экранируем aFieldName на случай спецсимволов в имени поля с помощью escape().
    QRegularExpression rx(QRegularExpression::escape(aFieldName) + "[:\\s]+([^\\n\\r]+)",
                          QRegularExpression::CaseInsensitiveOption);

    // 2. Преобразуем буфер в строку (Latin1 обычно достаточно для заголовков AT-команд).
    // Выполняем поиск.
    QRegularExpressionMatch match = rx.match(QString::fromLatin1(aBuffer).trimmed());

    // 3. Проверяем наличие совпадения
    if (match.hasMatch()) {
        // 4. Извлекаем первую захваченную группу (текст после двоеточия/пробелов)
        aValue = match.captured(1).trimmed();

        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getSimCOMCellList(QString &aValue) {
    QByteArray answer;
    // 1. Используем QRegularExpression.
    // В Qt 6 регулярные выражения компилируются один раз, что эффективнее.
    QRegularExpression regExp("(\\d+),(\\d+),\"([\\d\\w]+)\",\"([\\d\\w]+)\"");
    QByteArray CGREG = AT::Commands::SimCom::CGREG;

    // 2. Выполняем команды.
    // Для проверки регулярного выражения сначала конвертируем ответ в строку.
    if (!processCommand(CGREG + "=2", CATGSMModem::Timeouts::CellInfo) ||
        !processCommand(CGREG + "?", answer, CATGSMModem::Timeouts::CellInfo)) {
        return false;
    }

    // 3. Выполняем поиск и сохраняем результат в объект Match
    QRegularExpressionMatch match = regExp.match(QString::fromUtf8(answer));

    if (!match.hasMatch()) {
        return false;
    }

    // 4. Используем match.captured(N) вместо cap(N).
    // Помним, что для Hex (16-ричных) значений мы используем captured().
    QStringList info = QStringList() << ""
                                     << "" << QString::number(match.captured(4).toInt(nullptr, 16))
                                     << QString::number(match.captured(3).toInt(nullptr, 16)) << ""
                                     << "";

    aValue = info.join(",");

    return true;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getSiemensCellList(QString &aValue) {
    QByteArray answer;

    // Проверка выполнения команды и наличия специфического ответа модема
    if (!processCommand(AT::Commands::Siemens::GetCellList, answer, CATGSMModem::Timeouts::Siemens::CellInfo) ||
        answer.contains("^SMONC:")) {
        return false;
    }

    QList<QString> cellList;
    QMap<QString, QStringList> cellsData;

    // MCC, MNC, LAC, cell, BSIC, channel, RSSI, C1, C2
    // Используем QStringLiteral для совместимости и производительности
    QStringList items = QString::fromLatin1(answer).section(':', 1, 1).split(QStringLiteral(","));
    bool result = false;

    for (int i = 0; i < items.size(); i += 9) {
        // Проверяем, что индекс MCC больше 0 и хватает данных для RSSI (i + 6)
        if (items[i].trimmed().toInt() > 0 && (i + 6) < items.size()) {
            QStringList info;
            // Конвертируем HEX значения в десятичные (nullptr для отсутствия флага ошибки)
            info << items[i].trimmed() << items[i + 1].trimmed()
                 << QString::number(items[i + 3].toInt(nullptr, 16)).trimmed()
                 << QString::number(items[i + 2].toInt(nullptr, 16)).trimmed() << items[i + 6].trimmed();

            QString channelKey = items[i + 5];
            cellList.push_back(channelKey);
            cellsData[channelKey] = info;

            result = true;
        }
    }

    QList<QByteArray> commands;
    commands << AT::Commands::Siemens::ActiveCellInfo << AT::Commands::Siemens::InactiveCellInfo;

    // Используем QRegularExpression — стандарт для Qt 5.15 и Qt 6
    QRegularExpression whiteSpaceRegex(QStringLiteral("\\s+"));

    for (const QByteArray &command : commands) {
        if (processCommand(command, answer, CATGSMModem::Timeouts::CellInfo)) {
            QStringList lines = QString::fromLatin1(answer).trimmed().split(QStringLiteral("\n"));

            for (const QString &line : lines) {
                // Qt::SkipEmptyParts — универсальный флаг для Qt 5.14+ и 6.x
                QStringList columns = line.split(whiteSpaceRegex, Qt::SkipEmptyParts);

                if (!columns.isEmpty()) {
                    QString channel = columns[0].trimmed();

                    if (channel.toInt() > 0 && cellsData.contains(channel)) {
                        // Добавляем данные, если в колонках есть нужный параметр
                        if (columns.size() > 2) {
                            cellsData[channel] << columns[2];
                            result = true;
                        }
                    }
                }
            }
        }
    }

    QStringList resultValueList;
    for (const QString &channel : cellList) {
        resultValueList << cellsData[channel].join(QStringLiteral(","));
    }

    aValue = resultValueList.join(QStringLiteral(";"));

    return result;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::getSignalQuality(int &aQuality) {
    /* Signal quality: +CSQ
       +CSQ: [rssi],[ber]
    */
    if (!checkConnectionAbility()) {
        return false;
    }

    enableLocalEcho(false);

    QByteArray answer;
    processCommand(AT::Commands::CSQ, answer);

    // 1. Используем QRegularExpression. Для повышения производительности в Qt 6
    // можно использовать префикс u"" и литерал _s, но для совместимости с 5.15
    // используем QStringLiteral.
    QRegularExpression signalRegex(QStringLiteral("(\\d+),(\\d+)"));

    // 2. Конвертируем ответ в строку для поиска
    QRegularExpressionMatch match = signalRegex.match(QString::fromUtf8(answer));
    bool result = false;

    // 3. Используем hasMatch() вместо indexIn() != -1
    if (match.hasMatch()) {
        result = true;

        // 4. Используем captured(N) вместо cap(N).
        // toInt(nullptr) — стандартный способ в Qt 5/6, когда не нужен указатель на bool ok.
        aQuality = match.captured(1).toInt(nullptr);
        int bitErrorRate = match.captured(2).toInt(nullptr); // Переименовано ber -> bitErrorRate для чистоты кода

        toLog(LogLevel::Normal, QStringLiteral("Signal quality (rssi, ber):(%1, %2).").arg(aQuality).arg(bitErrorRate));
    }

    mIOPort->close();

    return result;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::reset() {
    if (!checkConnectionAbility()) {
        return false;
    }

    enableLocalEcho(false);

    // Сбрасываем модем.
    toLog(LogLevel::Normal, "Resetting modem to factory defaults...");

    switch (mGsmDialect) {
        case AT::EModemDialect::Siemens: {
            toLog(LogLevel::Normal, "Restart Siemens modem...");
            processCommand(AT::Commands::Siemens::Restart);

            break;
        }
        case AT::EModemDialect::ZTE: {
            toLog(LogLevel::Normal, "Power on ZTE modem...");
            processCommand(AT::Commands::ZTE::PowerOn);

            break;
        }
        case AT::EModemDialect::SimCom: {
            toLog(LogLevel::Normal, "Restart SimCOM modem...");
            processCommand(AT::Commands::SimCom::Restart);

            break;
        }
        default: {
            toLog(LogLevel::Normal, "Restart modem does not supported for this dialect.");

            break;
        }
    }

    // Проверяем, откликается ли модем
    if (!checkAT(CATGSMModem::Timeouts::ResetConnection)) {
        mIOPort->close();
        return false;
    }

    bool result = processCommand(AT::Commands::ATandF0);

    toLog(LogLevel::Normal, "Wait GSM network accessability...");

    if (waitNetworkAccessability(CATGSMModem::Timeouts::Connection)) {
        toLog(LogLevel::Normal, "GSM network available.");
    } else {
        toLog(LogLevel::Warning, "GSM network still waiting...");
    }

    onPoll();

    mIOPort->close();

    return result;
}

//---------------------------------------------------------------------------------
bool ATGSMModem::getStatus(TStatusCodes &aStatuses) {
    if (!checkConnectionAbility()) {
        return false;
    }

    enableLocalEcho(false);

    if (!processCommand(AT::Commands::AT, CATGSMModem::Timeouts::ResetConnection)) {
        return false;
    }

    QByteArray answer;

    if (!processCommand(AT::Commands::CPIN, answer)) {
        aStatuses.insert(ModemStatusCode::Error::SIMError);
        toLog(LogLevel::Error, QString("SIM card error, modem answer '%1'.").arg(answer.data()));
    } else {
        ENetworkAccessability::Enum networkAccessability;

        if (!getNetworkAccessability(networkAccessability) ||
            (networkAccessability != ENetworkAccessability::RegisteredHomeNetwork &&
             networkAccessability != ENetworkAccessability::SearchingOperator)) {
            aStatuses.insert(ModemStatusCode::Error::NoNetwork);
            toLog(LogLevel::Error, QString("Network is not available, modem answer '%1'.").arg(answer.data()));
        }
    }

    // Закрываем порт. TODO: найти более подходящее место для этого.
    mIOPort->close();

    return true;
}

//-------------------------------------------------------------------------------
bool ATGSMModem::getNetworkAccessability(ENetworkAccessability::Enum &aNetworkAccessability) {
    aNetworkAccessability = ENetworkAccessability::Unknown;

    QByteArray answer;
    processCommand(AT::Commands::CREG, answer);

    // 1. В QRegularExpression вместо setMinimal(true) используется квантификатор '?' после '+'.
    // Однако в данном конкретном выражении квантификатор не требуется,
    // так как мы ищем конкретные одиночные цифры.
    QRegularExpression regExp(QStringLiteral("\\+CREG:\\s+\\d,(\\d)"));

    // 2. Выполняем поиск и получаем объект совпадения.
    QRegularExpressionMatch match = regExp.match(QString::fromUtf8(answer));

    // 3. Проверяем успех с помощью hasMatch()
    if (!match.hasMatch()) {
        return false;
    }

    // 4. Используем captured(1) для получения группы.
    // toInt(nullptr) полностью совместим с Qt 5.15 и Qt 6.
    int statusValue = match.captured(1).toInt(nullptr);
    aNetworkAccessability = static_cast<ENetworkAccessability::Enum>(statusValue);

    return true;
}

//-------------------------------------------------------------------------------
bool ATGSMModem::waitNetworkAccessability(int aTimeout) {
    ENetworkAccessability::Enum networkAccessability;

    // 1. Используем QElapsedTimer вместо QTime для измерения прошедшего времени.
    // Это стандарт для Qt 6, так как QTime::start() больше не существует.
    QElapsedTimer timer;
    timer.start();

    do {
        // Вызываем ранее рефакторенный метод (уже использующий QRegularExpression)
        if (getNetworkAccessability(networkAccessability) &&
            (networkAccessability == ENetworkAccessability::RegisteredHomeNetwork ||
             networkAccessability == ENetworkAccessability::RegisteredRoaming)) {
            return true;
        }

        // 2. Небольшая пауза (опционально), чтобы не перегружать процессор и порт в цикле
        // QThread::msleep(100);

    } while (timer.elapsed() < aTimeout); // elapsed() возвращает qint64 миллисекунд

    return false;
}

//-------------------------------------------------------------------------------
bool ATGSMModem::getCUSDMessage(const QByteArray &aBuffer, QString &aMessage) {
    QString str;

    if (mGsmDialect == AT::EModemDialect::SimCom) {
        str.reserve(aBuffer.size());
        for (char character : aBuffer) {
            if (character != '\0') {
                str.append(QLatin1Char(character));
            }
        }
    } else {
        str = QString::fromLatin1(aBuffer).simplified();
    }

    // Регулярное выражение с анкорами ^ и $ для замены exactMatch
    QRegularExpression cusdRegex(QStringLiteral("^.*\\+CUSD: ?(\\d)(?:,\"(.*)\",(\\d+))?.*$"));
    QRegularExpressionMatch match = cusdRegex.match(str);

    if (!match.hasMatch()) {
        return false;
    }

    QString messageContent = match.captured(2);
    QString dataCodingScheme = match.captured(3);

    if (dataCodingScheme.toInt(nullptr) == 72) {
        QRegularExpression hexRegex(QStringLiteral("^[0-9A-Fa-f]+$"));

        if (hexRegex.match(messageContent).hasMatch()) {
            QByteArray hexBuffer;
            for (QChar aChar : messageContent) {
                hexBuffer.append(static_cast<char>(aChar.toLatin1()));

                if (hexBuffer.size() == 4) {
                    // Явное приведение к ushort для QChar конструктора
                    aMessage.push_back(QChar(static_cast<ushort>(hexBuffer.toInt(nullptr, 16))));
                    hexBuffer.clear();
                }
            }
        } else {
            // Устранение Deprecated Warning: используем char16_t
            QByteArray rawBytes = messageContent.toLatin1();
            aMessage =
                QString::fromUtf16(reinterpret_cast<const char16_t *>(rawBytes.constData()), rawBytes.size() / 2);
        }
    } else {
        aMessage = messageContent;
    }

    return true;
}

//-------------------------------------------------------------------------------
bool ATGSMModem::processUSSD(const QString &aMessage, QString &aAnswer) {
    if (!checkConnectionAbility()) {
        return false;
    }

    enableLocalEcho(false);

    QByteArray command;
    int commandTimeout = CATGSMModem::Timeouts::Default;

    toLog(LogLevel::Normal, QStringLiteral("ATGSMModem: sending USSD '%1'.").arg(aMessage));

    command.append(AT::Commands::CUSD);
    command.append(",\"");

    switch (mGsmDialect) {
        case AT::EModemDialect::Huawei: {
            // В Qt 6 toHex() возвращает QByteArray, что эффективно
            command.append(SmsPduEncoder::encode(aMessage.toLatin1()).toHex());
            break;
        }
        case AT::EModemDialect::SimCom: {
            commandTimeout = CATGSMModem::Timeouts::SimCom::USSD;
            // Переключаем кодировку USSD
            processCommand(AT::Commands::SimCom::CSCS);
            // fall through
            //[[fallthrough]]; // Явное указание перехода для компилятора (C++17)
        }
        default: {
            command.append(aMessage.toUtf8()); // Рекомендуется использовать Utf8 для AT-команд
            break;
        }
    }

    command.append("\",15");

    QByteArray answer;

    if (!processCommand(command, answer, commandTimeout)) {
        // В Qt 6/5.15 используем QByteArray::contains() напрямую
        if ((mGsmDialect != AT::EModemDialect::ZTE) || !answer.contains("Unexpected Data Value")) {
            mIOPort->close();
            return false;
        }

        // Повторяем команду для ZTE модемов
        SleepHelper::msleep(CATGSMModem::Pauses::ZTE::USSDAttempt);
        toLog(LogLevel::Normal, QStringLiteral("Retry send USSD for ZTE modems"));

        if (!processCommand(command, answer, commandTimeout)) {
            mIOPort->close();
            return false;
        }
    }

    for (int attempt = 0; attempt < CATGSMModem::BalanceAttempts; ++attempt) {
        toLog(LogLevel::Normal, QStringLiteral("Waiting for USSD answer, attempt %1.").arg(attempt + 1));

        SleepHelper::msleep(CATGSMModem::Pauses::BalanceAttempt);
        QByteArray data;

        // Несколько попыток прочитать данные из порта
        for (int i = 0; i < 4 && mIOPort->read(data); ++i) {
            answer.append(data);

            toLog(LogLevel::Normal, QStringLiteral("Total received string: %1").arg(QString::fromLatin1(answer)));

            // Вызываем обновленный нами ранее метод на базе QRegularExpression
            if (getCUSDMessage(answer, aAnswer)) {
                toLog(LogLevel::Normal, QStringLiteral("USSD answer: %1").arg(aAnswer));
                mIOPort->close();
                return true;
            }

            // В Qt 6/5.15 indexOf для байтовых строк работает очень быстро
            if (answer.indexOf("ERROR") != -1) {
                toLog(LogLevel::Warning, QStringLiteral("USSD request failed: %1").arg(QString::fromLatin1(answer)));
                mIOPort->close();
                return false;
            }

            SleepHelper::msleep(CATGSMModem::Pauses::BalanceData);
        }
    }

    mIOPort->close();
    return false;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::sendMessage(const QString &aPhone, const QString &aMessage) {
    // TODO: учитывать таймауты.
    if (!checkConnectionAbility()) {
        return false;
    }

    enableLocalEcho(false);

    toLog(LogLevel::Normal, QString("ATGSMModem: sending SMS to number %1.").arg(aPhone));
    processCommand(AT::Commands::SetTextMode);

    QByteArray command = AT::Commands::SendSMS + aPhone.toLatin1();
    QByteArray answer;
    processCommand(command, answer);

    bool result = false;

    if (answer.indexOf(">") > -1) {
        command = aMessage.toLatin1() + AT::Commands::StrgZ;

        if (mIOPort->write(command)) {
            // Ожидаем прихода уведомления об отправке сообщения.
            SleepHelper::msleep(CATGSMModem::Pauses::Message);

            result = mIOPort->read(answer) && (answer.indexOf("OK") > -1);
        }
    }

    mIOPort->close();

    return result;
}

//--------------------------------------------------------------------------------
bool ATGSMModem::takeMessages(TMessages &aMessages) {
    if (!checkConnectionAbility()) {
        return false;
    }

    enableLocalEcho(false);

    toLog(LogLevel::Normal, QStringLiteral("Read SMS from the modem"));
    processCommand(AT::Commands::SetPDUMode);

    QByteArray answer;
    processCommand(AT::Commands::ListSMS, answer, CATGSMModem::Timeouts::SMS);

    QString answerData = QString::fromLatin1(answer);

    QList<int> messageIds;
    QList<SmsPart> parts;

    // 1. Создаем регулярное выражение один раз.
    // Используем [\\r\\n]+ для обработки разных окончаний строк.
    QRegularExpression messageRegex(QStringLiteral("\\+CMGL:\\s+(\\d+),.*[\\r\\n]+([0-9A-Fa-f]+)[\\r\\n]+"));

    // 2. Используем итератор для эффективного поиска всех совпадений в строке.
    QRegularExpressionMatchIterator iterator = messageRegex.globalMatch(answerData);

    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();

        int messageId = match.captured(1).toInt(nullptr);
        QString pduData = match.captured(2);

        toLog(LogLevel::Debug, QStringLiteral("SMS %1: %2").arg(messageId).arg(pduData));

        messageIds << messageId;
        parts << Sms::decode(pduData);
    }

    toLog(LogLevel::Normal, QStringLiteral("Read %1 SMS parts").arg(parts.size()));

    QList<Sms> messages = Sms::join(parts);
    toLog(LogLevel::Normal, QStringLiteral("SMS parts joined to %1 SMS").arg(messages.size()));

    // 3. Заменяем foreach на стандартный цикл C++ (совместимо с C++14)
    for (const auto &message : messages) {
        if (message.isValid()) {
            GSM::SSMS sms;

            sms.date = message.getDateTime().toLocalTime();
            sms.from = message.getSenderAddress();
            sms.text = message.getText();

            aMessages << sms;
        } else {
            toLog(LogLevel::Warning, message.getErrorString());
        }
    }

    // 4. qStableSort устарел. В Qt 5.15/6 используем std::stable_sort.
    // Требует #include <algorithm>
    std::stable_sort(aMessages.begin(), aMessages.end(),
                     [](const SDK::Driver::GSM::SSMS &aSms1, const SDK::Driver::GSM::SSMS &aSms2) -> bool {
                         return aSms1.date < aSms2.date;
                     });

    // 5. Удаляем из модема все прочитанные сообщения
    for (int partId : messageIds) {
        QByteArray command = AT::Commands::DeleteSMS + QByteArray::number(partId);
        processCommand(command);
    }

    return !aMessages.isEmpty();
}
//--------------------------------------------------------------------------------

/* @file Вспомогательные функции для протоколов. */

#include <QtCore/QFile>
#include <QtCore/QRegularExpression>
#include <QtCore/QTextStream>

#include <Hardware/Protocols/Common/ProtocolUtils.h>

bool ProtocolUtils::getBit(const QByteArray &aBuffer, int aShift, bool invert) {
    int byteNumber = aShift / 8;

    if (byteNumber > aBuffer.size()) {
        return false;
    }

    if (invert) {
        byteNumber = aBuffer.size() - byteNumber - 1;
    }

    int bitNumber = aShift - (byteNumber * 8);

    if ((byteNumber + 1) <= aBuffer.size()) {
        return ((aBuffer[byteNumber] >> bitNumber) & 1) != 0;
    }

    return false;
}

//--------------------------------------------------------------------------------
bool ProtocolUtils::checkBufferString(QString aData, QString *aLog) {
    aData = aData.replace("0x", "").replace(" ", "");
    auto makeResult = [&aLog](const QString &aLogData) -> bool {
        if (aLog) {
            *aLog = "Failed to check buffer string due to " + aLogData;
        }
        return false;
    };

    int size = aData.size();

    if ((size % 2) != 0) {
        return makeResult("size = " + QString::number(size));
    }

    for (int i = 0; i < size / 2; ++i) {
        bool ok = false;
        QString data = aData.mid(i * 2, 2);
        data.toUShort(&ok, 16);

        if (!ok) {
            return makeResult(QString("data #%1 = %2").arg(i).arg(data));
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
QByteArray ProtocolUtils::getBufferFrom_String(QString aData) {
    aData = aData.replace("0x", "").replace(" ", "");
    QByteArray result;

    if ((aData.size() % 2) != 0) {
        aData = "0" + aData;
    }

    for (int i = 0; i < aData.size() / 2; ++i) {
        result += uchar(aData.mid(i * 2, 2).toUShort(nullptr, 16));
    }

    return result;
}

//--------------------------------------------------------------------------------
ProtocolUtils::TBufferList ProtocolUtils::getBufferListFrom_Strings(QStringList aDataList) {
    ProtocolUtils::TBufferList result;

    aDataList.removeAll("");
    QRegularExpression regex(CProtocolUtils::LogRexExp);

    for (const auto &i : aDataList) {

        QRegularExpressionMatch match = regex.match(i);
        if (match.hasMatch() && (match.capturedTexts()[1].size() > 4)) {
            QString lineData = match.capturedTexts()[1];
            result << ProtocolUtils::getBufferFrom_String(lineData);
        }
    }

    return result;
}

//--------------------------------------------------------------------------------
ProtocolUtils::TBufferList ProtocolUtils::getBufferListFrom_Log(const QString &aData) {
    return getBufferListFrom_Strings(aData.split("\t"));
}

//--------------------------------------------------------------------------------
ProtocolUtils::TBufferList ProtocolUtils::getBufferListFrom_File(const QString &aFileName) {
    ProtocolUtils::TBufferList result;

    QFile file(aFileName);

    if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QTextStream ts(&file);
    QString data = ts.readAll();
    file.close();

    return getBufferListFrom_Strings(data.split("\r\n"));
}

//--------------------------------------------------------------------------------
char ProtocolUtils::mask(char aData, const QString &aMask) {
    for (int i = 0; i < 8; ++i) {
        int bit = aMask[i].digitValue();
        char mask = 1 << (7 - i);

        if (bit != -1) {
            if (bit != 0) {
                aData |= mask;
            } else {
                aData &= ~mask;
            }
        }
    }

    return aData;
}

//--------------------------------------------------------------------------------
QString ProtocolUtils::hexToBCD(const QByteArray &aBuffer, char filler) {
    QString result;

    for (char i : aBuffer) {
        result += QString("%1").arg(uchar(i), 2, 10, QChar(filler));
    }

    return result;
}

//--------------------------------------------------------------------------------
QByteArray ProtocolUtils::getHexReverted(double aValue, int aSize, int aPrecision) {
    qint64 value = qRound64(aValue * pow(10.0, abs(aPrecision)));
    QString stringValue = QString("%1").arg(value, aSize * 2, 16, QChar(ASCII::Zero));
    QByteArray result;

    for (int i = 0; i < aSize; ++i) {
        result.append(uchar(stringValue.mid(2 * (aSize - i - 1), 2).toInt(nullptr, 16)));
    }

    return result;
}

//--------------------------------------------------------------------------------

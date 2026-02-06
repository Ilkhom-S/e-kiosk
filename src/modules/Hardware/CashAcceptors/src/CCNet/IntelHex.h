/* @file Парсинг Intel-HEX файлов прошивок. */

#pragma once

#include <QtCore/QByteArray>
#include <QtCore/QPair>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>

#include <numeric>

#include "Hardware/Common/ASCII.h"

//--------------------------------------------------------------------------------
namespace CIntelHex {
/// Разделители записей в ssf-файле.
const char Separator1[] = "\r\n";
const char Separator2[] = "\n";
const char Separator3[] = "\r";

/// Префикс записи.
const char Prefix = ASCII::Colon;

/// Минимальная длина записи.
const int MinRecordLength = 11;
} // namespace CIntelHex

//--------------------------------------------------------------------------------
namespace IntelHex {
namespace EType {
enum Enum {
    NoType = -1,
    DataBlock = 0,
    EndOfFile,
    SegmentAddress,
    StartSegmentAddress,
    ExtendedAddress,
    StartLinearAddress
};
} // namespace EType

struct SRecordData {
    EType::Enum type;
    ushort address;
    QByteArray data;

    SRecordData() : type(EType::NoType), address(0) {}
    SRecordData(EType::Enum aType, ushort aAddress, const QByteArray &aData)
        : type(aType), address(aAddress), data(aData) {}
};

typedef QPair<ushort, QByteArray> TAddressedBlock;
typedef QList<TAddressedBlock> TAddressedBlockList;

bool parseRecord(const QString &aRecord, SRecordData &aData, QString &aErrorDescription);
bool parseRecords(const QStringList &aRecords,
                  TAddressedBlockList &aAddressedBlockList,
                  int aBlockSize,
                  QString &aErrorDescription);
} // namespace IntelHex

//--------------------------------------------------------------------------------

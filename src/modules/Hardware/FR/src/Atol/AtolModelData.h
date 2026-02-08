/* @file Данные моделей ФР на протоколе АТОЛ. */

#pragma once

#include <QtCore/QSet>
#include <QtCore/QStringList>

#include "AtolDataTypes.h"
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
/// Константы, команды и коды состояний устройств на протоколе ATOL.
namespace CAtolFR {
/// Признак наличия ЭКЛЗ по имени устройства.
const char EKLZPostfix = 'K';

/// Признаки наличия ЭКЛЗ в имени устройства в ответе устройства.
typedef QSet<char> TEKLZPostfixes;
const TEKLZPostfixes EKLZPostfixes = TEKLZPostfixes() << EKLZPostfix << 'k' << '\x8A' << '\xAA';

/// Актуальная версия прошивки для торговых ФР.
const int OnlineTradeBuild = 5199;

/// Актуальная версия прошивки для терминальных ФР.
const int OnlineTerminalBuild = 5652;

namespace Models {
extern const char Trium_F[];
extern const char FelixRF[];
extern const char Felix02K[];
extern const char Mercury140F[];
extern const char Tornado[];
extern const char Mercury130[];
extern const char MercuryMSK[];
extern const char FelixRK[];
extern const char Felix3SK[];
extern const char FPrint01K[];
extern const char FPrint02K[];
extern const char FPrint03K[];
extern const char FPrint88K[];
extern const char BIXOLON01K[];
extern const char MicroFR01K[];
extern const char FPrint5200K[];
extern const char Flaton11K[];

extern const char PayVKP80K[];
extern const char PayPPU700K[];
extern const char PayCTS2000K[];

extern const char FPrint55K[];
extern const char FPrint11PTK[];
extern const char FPrint22K[];
extern const char FPrint77PTK[];

extern const char Atol11F[];
extern const char Atol15F[];
extern const char Atol25F[];
extern const char Atol30F[];
extern const char Atol42FC[];
extern const char Atol52F[];
extern const char Atol55F[];
extern const char Atol77F[];
extern const char Atol91F[];

extern const char Paymaster[];
extern const char FPrint22PTK[];
} // namespace Models

/// Данные моделей.
typedef QPair<int, EFRType::Enum> TModelKey;

class CModelData : public CSpecification<TModelKey, SModelData> {
public:
    CModelData();

    /// Получить список моделей.
    QStringList getModelList(EFRType::Enum aFRType, bool aCanBeDP);

private:
    /// Добавить кассовый ФР старых серий.
    void addOldTrade(int aModelId,
                     int aMaxStringSize,
                     QString aName,
                     bool aCutter,
                     bool aLineSpacing,
                     bool aVerified = false,
                     int aFeedingAmount = 0);

    /// Добавить кассовый ФР серии FPrint и/или следующих.
    void addTrade(int aModelId,
                  int aMaxStringSize,
                  QString aName,
                  bool aCutter = true,
                  bool aVerified = false,
                  int aFeedingAmount = 0);

    /// Добавить терминальный ФР.
    void addTerminal(int aModelId,
                     int aMaxStringSize,
                     const QString &aName,
                     int aBuild,
                     bool aEjector,
                     int aFeedingAmount = 0,
                     int aZBufferSize = 0);

    /// Добавить онлайновый кассовый ФР.
    void addOnlineTrade(int aModelId,
                        int aMaxStringSize,
                        QString aName,
                        bool aCutter,
                        int aFeedingAmount = 0,
                        int aBuild = OnlineTradeBuild,
                        bool aVerified = true);

    /// Добавить онлайновый терминальный ФР.
    void addOnlineTerminal(int aModelId,
                           int aMaxStringSize,
                           const QString &aName,
                           bool aEjector,
                           int aFeedingAmount,
                           int aZBufferSize = 0,
                           int aBuild = OnlineTerminalBuild,
                           bool aVerified = true);
};
} // namespace CAtolFR

//--------------------------------------------------------------------------------

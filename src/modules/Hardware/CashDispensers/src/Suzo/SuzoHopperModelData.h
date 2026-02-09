/* @file Данные моделей хопперов на протоколе ccTalk. */

#pragma once

#include "Hardware/CashDevices/CCTalkDeviceConstants.h"
#include "Hardware/CashDevices/CCTalkModelData.h"

//--------------------------------------------------------------------------------
namespace CCCTalk {
namespace Dispenser {
/// Параметры моделей.
namespace Models {
extern const char Default[];
} // namespace Models

class CModelData : public CModelDataBase {
public:
    CModelData() {
        add("SUZO INT (NL)", "SCH2", "CUBE MKII");
        data()["SUZO INT (NL)"]["SCH2"].unsupported =
            CCCTalk::TUnsupported()
            << CCCTalk::Command::Reset // должна работать, но ответа нет и внешних признаков также
                                       // нет
            << CCCTalk::Command::DBVersion << CCCTalk::Command::BaseYear
            << CCCTalk::Command::CreationDate << CCCTalk::Command::SoftLastDate;
    }
};
} // namespace Dispenser
} // namespace CCCTalk

//--------------------------------------------------------------------------------

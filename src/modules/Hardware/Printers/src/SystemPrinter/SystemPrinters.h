/* @file Системные принтеры без авто поиска. */

#pragma once

#include "System_Printer.h"

//--------------------------------------------------------------------------------
class SunphorPOS58IV : public System_Printer {
    SET_SERIES("Sunphor")
    SET_SUBSERIES("POS58IV")

public:
    SunphorPOS58IV() {
        m_DeviceName = "Sunphor Printer";
        m_LineSize = 30;
        m_SideMargin = 4.0;
        m_AutoDetectable = false;
    }
};

//--------------------------------------------------------------------------------
class ICTGP83 : public System_Printer {
    SET_SERIES("ICT")
    SET_SUBSERIES("GP83")

public:
    ICTGP83() {
        m_DeviceName = "ICT GP83";
        m_SideMargin = 3.0;
        m_AutoDetectable = false;
    }
};

//--------------------------------------------------------------------------------

/* @file Базовый принтер Citizen. */

#pragma once

#include "Hardware/Printers/PortPOSPrinters.h"

//--------------------------------------------------------------------------------
template <class T> class CitizenBase : public T {
public:
    CitizenBase() {
        this->m_RussianCodePage = '\x07';

        // теги
        this->m_Parameters = POSPrinters::CommonParameters;
        this->m_Parameters.tagEngine.appendCommon(Tags::Type::DoubleWidth, "\x1B\x21", "\x20");
        this->m_Parameters.tagEngine.appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");
    }
};

//--------------------------------------------------------------------------------

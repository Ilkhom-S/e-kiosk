/* @file Базовый принтер Citizen. */

#pragma once

#include "Hardware/Printers/PortPOSPrinters.h"

//--------------------------------------------------------------------------------
template <class T> class CitizenBase : public T
{
  public:
    CitizenBase()
    {
        this->mRussianCodePage = '\x07';

        // теги
        this->mParameters = POSPrinters::CommonParameters;
        this->mParameters.tagEngine.appendCommon(Tags::Type::DoubleWidth, "\x1B\x21", "\x20");
        this->mParameters.tagEngine.appendCommon(Tags::Type::DoubleHeight, "\x1B\x21", "\x10");
    }
};

//--------------------------------------------------------------------------------

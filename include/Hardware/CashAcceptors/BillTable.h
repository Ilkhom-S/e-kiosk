/* @file Описатель таблицы номиналов. */

#pragma once

#include <SDK/Drivers/CashAcceptor/Par.h>

#include "Hardware/CashAcceptors/CurrencyDescriptions.h"
#include "Hardware/Common/Specifications.h"

//--------------------------------------------------------------------------------
class CBillTable : public CSpecification<int, SDK::Driver::SPar> {
public:
    void add(int aEscrow, const SDK::Driver::SPar &aPar) {
        SDK::Driver::SPar par(aPar);

        if (!CurrencyCodes.data().contains(par.currency) &&
            CurrencyCodes.data().values().contains(par.currencyId)) {
            par.currency = CurrencyCodes.key(par.currencyId);
        }

        append(aEscrow, par);
    }
};

//--------------------------------------------------------------------------------

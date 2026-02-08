/* @file ФР ПРИМ c презентером. */

#pragma once

#include "../Online/Prim_OnlineFRBase.h"
#include "../Prim_FRBase.h"

//--------------------------------------------------------------------------------
template <class T> class Prim_PresenterFR : public T {
    SET_SUBSERIES("Presenter")

public:
    Prim_PresenterFR();

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList();

protected:
    /// Напечатать [и выдать] чек.
    virtual bool perform_Receipt(const QStringList &aReceipt, bool aProcessing = true);

    typedef QSharedPointer<TSerialPrinterBase> PPrinter;
    PPrinter m_Printer;
};

//--------------------------------------------------------------------------------
typedef Prim_PresenterFR<Prim_FRBase> Prim_PresenterFRBase;

//--------------------------------------------------------------------------------

/* @file Принтер Custom TG2480H. */

#pragma once

#include "Custom_Printers.h"

//--------------------------------------------------------------------------------
template <class T> class Custom_TG2480H : public Custom_Printer<T> {
    SET_SUBSERIES("Custom_TG2480H")

public:
    Custom_TG2480H() {
        this->m_Parameters.errors[20][4].insert('\x08', PrinterStatusCode::OK::MotorMotion);
        this->m_Parameters.errors[20][5].insert('\x02', PrinterStatusCode::Error::Presenter);

        this->m_DeviceName = CCustom_Printer::Models::TG2480H;
        this->m_ModelID = '\xA8';

        this->m_ModelData.data().clear();
        this->m_ModelData.add(this->m_ModelID, true, CCustom_Printer::Models::TG2480H);
        this->setConfigParameter(CHardwareSDK::Printer::LineSize, 44);
    }

protected:
    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes) {
        if (!T::getStatus(aStatusCodes)) {
            return false;
        }

        if (aStatusCodes.contains(PrinterStatusCode::Error::Presenter)) {
            aStatusCodes.remove(PrinterStatusCode::Error::Presenter);

            this->cut();
            this->cut();
        }

        return true;
    }
};

//--------------------------------------------------------------------------------
class LibUSBCustom_TG2480H : public Custom_TG2480H<TLibUSBPrinterBase> {
public:
    LibUSBCustom_TG2480H() {
        this->m_DetectingData->set(CUSBVendors::Custom, this->m_DeviceName, 0x01a8);
        this->setConfigParameter(CHardwareSDK::Printer::LineSize, 44);
    }
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<Custom_TG2480H<TSerialPrinterBase>> SerialCustom_TG2480H;

//--------------------------------------------------------------------------------

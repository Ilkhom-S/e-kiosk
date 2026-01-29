/* @file Принтер Custom TG2480H. */

#pragma once

#include "CustomPrinters.h"

//--------------------------------------------------------------------------------
template <class T> class CustomTG2480H : public CustomPrinter<T>
{
    SET_SUBSERIES("CustomTG2480H")

  public:
    CustomTG2480H()
    {
        this->mParameters.errors[20][4].insert('\x08', PrinterStatusCode::OK::MotorMotion);
        this->mParameters.errors[20][5].insert('\x02', PrinterStatusCode::Error::Presenter);

        this->mDeviceName = CCustomPrinter::Models::TG2480H;
        this->mModelID = '\xA8';

        this->mModelData.data().clear();
        this->mModelData.add(this->mModelID, true, CCustomPrinter::Models::TG2480H);
        this->setConfigParameter(CHardwareSDK::Printer::LineSize, 44);
    }

  protected:
    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes)
    {
        if (!T::getStatus(aStatusCodes))
        {
            return false;
        }

        if (aStatusCodes.contains(PrinterStatusCode::Error::Presenter))
        {
            aStatusCodes.remove(PrinterStatusCode::Error::Presenter);

            this->cut();
            this->cut();
        }

        return true;
    }
};

//--------------------------------------------------------------------------------
class LibUSBCustomTG2480H : public CustomTG2480H<TLibUSBPrinterBase>
{
  public:
    LibUSBCustomTG2480H()
    {
        this->mDetectingData->set(CUSBVendors::Custom, this->mDeviceName, 0x01a8);
        this->setConfigParameter(CHardwareSDK::Printer::LineSize, 44);
    }
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<CustomTG2480H<TSerialPrinterBase>> SerialCustomTG2480H;

//--------------------------------------------------------------------------------

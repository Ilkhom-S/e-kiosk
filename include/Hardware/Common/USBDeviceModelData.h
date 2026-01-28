/* @file Спецификация данных моделей USB-устройств для авто поиска. */
#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QSharedPointer>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Hardware/Common/Specifications.h"
#include "Hardware/Common/USBDeviceModelDataTypes.h"
#include "Hardware/Common/USBDeviceVendors.h"

//--------------------------------------------------------------------------------

namespace CUSBDevice
{
    /// Данные моделей по PID-ам.
    template <class T> class ProductDataBase : public CSpecification<quint16, T>
    {
      public:
        // Возвращает список названий моделей для указанного производителя
        QStringList getModelList(const QString &aVendor)
        {
            QStringList result;
            result.reserve(this->mBuffer.size());

            // Используем range-based for (C++11/14) вместо устаревшего foreach
            for (const auto &data : this->mBuffer)
            {
                // QStringLiteral оптимизирует память на Windows 7
                result << aVendor + QStringLiteral(" ") + data.model;
            }
            return result;
        }

        // Устанавливает модель по умолчанию
        void setDefaultModel(const QString &aModel)
        {
            // this-> обязателен в шаблонах для обращения к mBuffer базового класса
            for (auto it = this->mBuffer.begin(); it != this->mBuffer.end(); ++it)
            {
                if (it.value().model == aModel)
                {
                    this->setDefault(it.value());
                    break;
                }
            }
        }

        TProductData getProductData() const
        {
            return mProductData;
        }

      protected:
        TProductData mProductData;
    };

    //--------------------------------------------------------------------------------
    class ProductData : public ProductDataBase<SProductData>
    {
      public:
        void add(quint16 aPID, const QString &aModel, bool aVerified = false)
        {
            // Метод append наследуется от CSpecification
            this->append(aPID, SProductData(aModel, aVerified));
        }
    };

    //--------------------------------------------------------------------------------
    /// Данные моделей по VID-ам.
    class DetectingData : public CSpecification<quint16, ProductData>
    {
      public:
        // Реализации этих методов (не шаблонных) можно оставить в .cpp
        void set(const QString &aVendor, quint16 aPID, const QString &aModel, bool aVerified = false);
        void set(const QString &aVendor, const QString &aDeviceName, quint16 aPID);
        void set(const SDetectingData &aDetectingData);
        void set(const QString &aVendor, const TProductData &aProductData);

      protected:
        // static члены определяются в .cpp
        static CUSBVendors::Data mVendorData;
    };

    typedef QSharedPointer<DetectingData> PDetectingData;
} // namespace CUSBDevice

//--------------------------------------------------------------------------------

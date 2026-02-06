/* @file Спецификация данных моделей USB-устройств для авто поиска. */

#include "USBDeviceModelData.h"

#include "Hardware/CardReaders/MifareReaderModelDataTypes.h"
#include "Hardware/Common/ASCII.h"

namespace CUSBDevice {

//--------------------------------------------------------------------------------
template class ProductDataBase<SProductData>;
template class ProductDataBase<CMifareReader::SModelData>;

CUSBVendors::Data DetectingData::mVendorData;

// //--------------------------------------------------------------------------------
// template <class T> QStringList ProductDataBase<T>::getModelList(const QString &aVendor)
// {
//     QStringList result;

//     // Резервируем память, чтобы избежать множественных переаллокаций при заполнении списка
//     result.reserve(this->mBuffer.size());

//     // Qt 6 удалил макрос foreach. Используем стандартный range-based for (C++11/14).
//     // Используем const auto & для предотвращения лишнего копирования объектов данных.
//     for (const auto &data : this->mBuffer)
//     {
//         // Использование оператора % (через QStringBuilder) или QStringLiteral
//         // более эффективно, чем оператор +, так как выделяет память один раз.
//         result << aVendor + QStringLiteral(" ") + data.model;
//     }

//     return result;
// }

// //--------------------------------------------------------------------------------
// template <class T> void ProductDataBase<T>::setDefaultModel(const QString &aModel)
// {
//     T data;
//     data.model = aModel;
//     data.verified = false;

//     setDefault(data);
// }

// //--------------------------------------------------------------------------------
// template <class T> TProductData ProductDataBase<T>::getProductData()
// {
//     return mProductData;
// }

// //--------------------------------------------------------------------------------
// void DetectingData::set(const QString &aVendor, quint16 aPID, const QString &aModel, bool
// aVerified)
// {
//     data().clear();

//     quint16 VID = mVendorData[aVendor];
//     data()[VID].append(aPID, SProductData(aVendor + " " + aModel, aVerified));
// }

//--------------------------------------------------------------------------------
void DetectingData::set(const QString &aVendor, const QString &aDeviceName, quint16 aPID) {
    data().clear();

    quint16 VID = mVendorData[aVendor];
    data()[VID].append(aPID, SProductData(aDeviceName, true));
}

//--------------------------------------------------------------------------------
void DetectingData::set(const SDetectingData &aDetectingData) {
    data().clear();

    quint16 VID = mVendorData[aDetectingData.vendor];
    data()[VID].append(aDetectingData.PID,
                       SProductData(aDetectingData.vendor + " " + aDetectingData.model, true));
}

//--------------------------------------------------------------------------------
void DetectingData::set(const QString &aVendor, const TProductData &aProductData) {
    data().clear();

    quint16 VID = mVendorData[aVendor];
    TProductData &productData = data()[VID].data();

    for (auto it = aProductData.begin(); it != aProductData.end(); ++it) {
        productData.insert(it.key(), SProductData(aVendor + " " + it->model, it->verified));
    }
}

} // namespace CUSBDevice

//--------------------------------------------------------------------------------

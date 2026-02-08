/* @file ФР ПРИМ c презентером. */

#include "Prim_PresenterFR.h"

#include "../Prim_ModelData.h"
#include "Hardware/Printers/EpsonEUT400.h"

//--------------------------------------------------------------------------------
template class Prim_PresenterFR<Prim_FRBase>;
template class Prim_PresenterFR<Prim_OnlineFRBase>;

//--------------------------------------------------------------------------------
// Получить модели данной реализации.
namespace CPrim_FR {
inline TModels PresenterModels() {
    return TModels() << CPrim_FR::Models::PRIM_21K_01 << CPrim_FR::Models::PRIM_21K_02;
}
} // namespace CPrim_FR

//--------------------------------------------------------------------------------
template <class T> Prim_PresenterFR<T>::Prim_PresenterFR() {
    // данные устройства
    setConfigParameter(CHardware::Printer::PresenterEnable, true);
    m_Printer = PPrinter(new EpsonEUT400());
    m_DeviceName = CPrim_FR::ModelData[CPrim_FR::Models::PRIM_21K_02].name;
    m_Models = CPrim_FR::PresenterModels();
}

//--------------------------------------------------------------------------------
template <class T> QStringList Prim_PresenterFR<T>::getModelList() {
    return CPrim_FR::getModelList(CPrim_FR::PresenterModels());
}

//--------------------------------------------------------------------------------
template <class T>
bool Prim_PresenterFR<T>::perform_Receipt(const QStringList &aReceipt, bool aProcessing) {
    using namespace CHardware::Printer;

    QVariantMap config = getDeviceConfiguration();
    config.insert(OutCall, true);

    // иначе прошивка ФР может воспринять это как DLE-команду
    int presentationLength = config.value(Settings::PresentationLength).toInt();

    if (presentationLength == int(ASCII::DLE)) {
        config.insert(Settings::PresentationLength, ++presentationLength);
    }

    SPrintingOutData printingOutData(m_Log, aProcessing, m_IOMessageLogging, config, aReceipt);

    return m_Printer->printOut(printingOutData);
}

//--------------------------------------------------------------------------------

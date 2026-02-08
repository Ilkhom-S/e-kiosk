/* @file Базовый класс устройств на TCP-порту. */

#include "TCPDeviceBase.h"

#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/FR/ProtoFR.h"
#include "Hardware/IOPorts/TCPPort.h"

using namespace SDK::Driver;

//-------------------------------------------------------------------------------
template class TCPDeviceBase<PortPollingDeviceBase<ProtoFR>>;

//--------------------------------------------------------------------------------
template <class T> TCPDeviceBase<T>::TCPDeviceBase() {}

//--------------------------------------------------------------------------------
template <class T> bool TCPDeviceBase<T>::checkConnectionAbility() {
    // В шаблонах C++ обращение к методам и членам базового класса через this-> обязательно.
    // Это исправляет ошибку "explicit qualification required".
    if (!this->checkError(
            IOPortStatusCode::Error::NotSet,
            [this]() { return this->m_IOPort != nullptr; },
            "IO port is not set")) {
        return false;
    }

    // dynamic_cast обеспечивает безопасность типов при передаче в QVariant.
    // Если IDevice наследует QObject, можно использовать qobject_cast.
    IDevice *deviceInterface = dynamic_cast<IDevice *>(this->m_IOPort);
    this->setConfigParameter(CHardwareSDK::RequiredDevice, QVariant::from_Value(deviceInterface));

    // Использование .value() вместо [] предотвращает создание пустых записей в карте конфигурации.
    // toString() корректно работает во всех версиях Qt 5.15 и 6.x.
    const QString address =
        this->m_IOPort->getDeviceConfiguration().value(CHardwareSDK::Port::TCP::IP).toString();

    return this->checkError(
               IOPortStatusCode::Error::NotConnected,
               [this]() { return this->m_IOPort->isExist() || this->m_IOPort->deviceConnected(); },
               "IO port is not connected") &&
           this->checkError(
               IOPortStatusCode::Error::NotConfigured,
               [&address]() { return !address.isEmpty(); },
               "IO port is not set correctly") &&
           this->checkError(
               IOPortStatusCode::Error::Busy,
               [this]() { return this->m_IOPort->open(); },
               "device cannot open port");
}

//--------------------------------------------------------------------------------
template <class T> bool TCPDeviceBase<T>::checkPort() {
    // В шаблонных классах необходимо использовать this-> для доступа к членам,
    // чтобы компилятор мог найти их в зависимом базовом классе на этапе инстанцирования.
    if (this->m_IOPort->isExist()) {
        return true;
    }

    // Если порт физически (или логически) не существует, проверяем активное соединение.
    if (!this->m_IOPort->deviceConnected()) {
        return false;
    }

    // Вызов виртуального или базового метода проверки существования.
    // Явное указание this-> предотвращает ошибку "explicit qualification required".
    return this->checkExistence();
}

// Исправленный макрос: используем QStringLiteral для оптимизации и явный this-> для логов
#define MAKE_TCP_PORT_PARAMETER(aName, aType)                                                      \
    TTCPDevicePortParameter aName = this->m_PortParameters.value(CHardwareSDK::Port::TCP::aType);  \
    if (aName.isEmpty()) {                                                                         \
        this->toLog(LogLevel::Error,                                                               \
                    this->m_DeviceName + QStringLiteral(": %1 are empty").arg(#aName));            \
        return false;                                                                              \
    }

//--------------------------------------------------------------------------------
template <class T> bool TCPDeviceBase<T>::makeSearchingList() {
    // В шаблонах C++ обращение к членам базового класса m_PortParameters требует this->
    MAKE_TCP_PORT_PARAMETER(IPs, IP);
    MAKE_TCP_PORT_PARAMETER(portNumbers, Number);

    // Qt 6 удалил макрос foreach. Используем стандартный range-based for (C++11/14),
    // который эффективнее и поддерживается всеми компиляторами в 2026 году.
    for (const QVariant &IP : IPs) {
        for (const QVariant &portNumber : portNumbers) {
            STCPPortParameters params(IP, portNumber);
            this->m_SearchingPortParameters.append(params);
        }
    }

    return true;
}

//--------------------------------------------------------------------------------
template <class T> IDevice::IDetectingIterator *TCPDeviceBase<T>::getDetectingIterator() {
    if (!this->m_AutoDetectable) {
        return nullptr;
    }

    this->m_SearchingPortParameters.clear();
    // makeSearchingList() — зависимое имя, вызываем через this->
    if (!this->makeSearchingList()) {
        return nullptr;
    }

    this->m_NextParameterIterator = this->m_SearchingPortParameters.begin();

    return this;
}

//--------------------------------------------------------------------------------
template <class T> bool TCPDeviceBase<T>::find() {
    QVariantMap portParameters;
    portParameters.insert(CHardwareSDK::Port::TCP::IP, this->m_CurrentParameter.IP);
    portParameters.insert(CHardwareSDK::Port::TCP::Number, this->m_CurrentParameter.number);

    // Прямая настройка IO порта для следующей итерации поиска
    this->m_IOPort->setDeviceConfiguration(portParameters);

    // Вызов статического или базового метода find из параметра шаблона
    return T::find();
}

//--------------------------------------------------------------------------------
template <class T> bool TCPDeviceBase<T>::moveNext() {
    // Безопасное сравнение итераторов (совместимо с Qt 5 и 6)
    if (this->m_NextParameterIterator == this->m_SearchingPortParameters.end()) {
        return false;
    }

    this->m_CurrentParameter = *(this->m_NextParameterIterator);
    this->m_NextParameterIterator++;

    return true;
}

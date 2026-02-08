/* @file Кардридер IDTech. */
#pragma once

// IDTech SDK
#pragma warning(push, 1)
#include "libIDT_Device.h"
#pragma warning(pop)

#include "Hardware/Common/PollingDeviceBase.h"
#include "Hardware/HID/ProtoHID.h"

namespace CIDTechReader {
// Имя dll SDK.
extern const char DLLSDKName[];
} // namespace CIDTechReader

//------------------------------------------------------------------------------
class IDTechReader : public PollingDeviceBase<ProtoHID> {
    SET_INTERACTION_TYPE(External)
    SET_SERIES("IDTech")

public:
    IDTechReader();
    virtual ~IDTechReader();

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList();

#pragma region SDK::Driver::IHID
    /// Включает/выключает устройство на чтение штрих-кодов. Пикать все равно будет.
    virtual bool enable(bool aEnabled);

    /// Готов ли к работе (инициализировался успешно, ошибок нет).
    virtual bool isDeviceReady();
#pragma endregion

    /// Получить результат чтения и данные MSR-карты.
    void getMSRCardData(int aType, IDTMSRData *aCardData1);

protected:
    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Инициализировать библиотеки.
    bool initializeLibraries();

    /// Проверить наличие и функциональность библиотеки - получить её версию.
    template <class T>
    bool
    checkLibrary(const char *aName, const char *aFunctionName, std::function<QString(T)> aFunction);

    /// Зарегистрировать callback.
    template <class T>
    bool registerCallback(HMODULE aHandle, const char *aFunctionName, T aFunction);

    /// Установить callback.
    template <class T> bool setCallback(HMODULE aHandle, const char *aFunctionName, T aFunction);

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Флаг корректной инициализации библиотек.
    bool m_LibrariesInitialized;

    /// Id устройства.
    int m_Id;

private:
    /// Статический экземпляр класса.
    static IDTechReader *m_Instance;

    /// Счетчик экземпляров класса.
    static int m_InstanceCounter;
};

//------------------------------------------------------------------------------

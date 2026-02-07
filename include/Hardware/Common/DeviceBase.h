/* @file Базовый класс устройств. */

#pragma once

#include <QtCore/QSet>

#include "Common/ExitAction.h"
#include "Hardware/Common/BaseStatusDescriptions.h"
#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/HystoryList.h"
#include "Hardware/Common/MetaDevice.h"
#include "Hardware/Common/PollingExpector.h"

//--------------------------------------------------------------------------------
typedef QList<int> TStatusCodesBuffer;

//--------------------------------------------------------------------------------
/// Общие константы устройств.
namespace CDevice {
/// Размер истории статусов.
const int StatusCollectionHistoryCount = 10;

/// Имя устройства по умолчанию.
extern const char DefaultName[];

/// Разделитель статусов.
extern const char StatusSeparator[];
} // namespace CDevice

//--------------------------------------------------------------------------------
#define START_IN_WORKING_THREAD(aFunction)                                                         \
    if ((!this->mOperatorPresence || (this->getConfigParameter(CHardware::CallingType) !=          \
                                      CHardware::CallingTypes::Internal)) &&                       \
        !this->isWorkingThread()) {                                                                \
        if (this->mThread.isRunning()) {                                                           \
            QMetaObject::invokeMethod(this, #aFunction, Qt::QueuedConnection);                     \
        } else {                                                                                   \
            this->connect(                                                                         \
                &this->mThread, SIGNAL(started()), this, SLOT(aFunction()), Qt::UniqueConnection); \
            this->mThread.start();                                                                 \
        }                                                                                          \
        return;                                                                                    \
    }

//--------------------------------------------------------------------------------
template <class T> class DeviceBase : public T {
public:
    DeviceBase();

#pragma region SDK::Driver::IDevice interface
    /// Подключает и инициализирует устройство. Обертка для вызова функционала в рабочем потоке.
    virtual void initialize();

    /// Освобождает ресурсы, связанные с устройством, возвращается в состояние до вызова
    /// initialize().
    virtual bool release();

    /// Соединяет сигнал данного интерфейса со слотом приёмника.
    virtual bool subscribe(const char *aSignal, QObject *aReceiver, const char *aSlot);

    /// Отсоединяет сигнал данного интерфейса от слота приёмника.
    virtual bool unsubscribe(const char *aSignal, QObject *aReceiver);
#pragma endregion

#pragma region SDK::Driver::IDetectingIterator interface
    /// Поиск устройства на текущих параметрах.
    virtual bool find();
#pragma endregion

    /// Полл без пост-действий.
    void simplePoll();

protected:
    /// Идентификация.
    virtual bool checkExistence();

    /// Обработчик сигнала полла.
    virtual void onPoll();

    /// Полл.
    virtual void doPoll(TStatusCodes &aStatusCodes);

    /// Получить и обработать статус.
    virtual bool processStatus(TStatusCodes &aStatusCodes);

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Применение буфера статусов для блокирования "мигающих" ошибок.
    void applyStatusBuffer(TStatusCodes &aStatusCodes);

    /// Анализирует коды статусов устройства и фильтрует несуществующие статусы для нижней логики.
    virtual void cleanStatusCodes(TStatusCodes &aStatusCodes);

    /// Есть ли ошибка инициализации при фильтрации статусов.
    bool isInitializationError(TStatusCodes &aStatusCodes);

    /// Исправить исправимые ошибки.
    void recoverErrors(TStatusCodes &aStatusCodes);

    /// Получить переводы новых статус-кодов для последующей их обработки.
    virtual QString getTrOfNewProcessed(const TStatusCollection &aStatusCollection,
                                        SDK::Driver::EWarningLevel::Enum aWarningLevel);

    /// Посылка статусов, если необходимо, в зависимости от типа устройства.
    void processStatusCodes(const TStatusCodes &aStatusCodes);

    /// Фоновая логика при появлении определенных состояний устройства.
    virtual void postPollingAction(const TStatusCollection &aNewStatusCollection,
                                   const TStatusCollection &aOldStatusCollection);

    /// Пере инициализация в рамках фоновой логики пост-поллинга.
    virtual void reInitialize();

    /// Отправка статусов.
    virtual void sendStatuses(const TStatusCollection &aNewStatusCollection,
                              const TStatusCollection &aOldStatusCollection);

    /// Инициализация устройства.
    virtual bool updateParameters();

    /// Установить начальные параметры.
    virtual void setInitialData();

    /// Завершение инициализации.
    virtual void finalizeInitialization();

    /// Идентифицирует устройство.
    virtual bool isConnected();

    /// Проверка возможности выполнения функционала, предполагающего связь с устройством.
    virtual bool checkConnectionAbility();

    /// Проверка возможности применения буфера статусов.
    virtual bool isStatusesReplaceable(TStatusCodes &aStatusCodes);

    /// Проверка возможности применения буфера статусов.
    virtual bool canApplyStatusBuffer();

    /// Есть ли несовпадение плагина и ПП.
    bool isPluginMismatch();

    /// Получить переводы статусов.
    QString getStatusTranslations(const TStatusCodes &aStatusCodes, bool aLocale) const;

    /// Получить спецификацию статуса.
    virtual SStatusCodeSpecification getStatusCodeSpecification(int aStatusCode) const;

    /// Получить статус-коды.
    TStatusCodes getStatusCodes(const TStatusCollection &aStatusCollection);

    /// Получить коллекцию статусов.
    TStatusCollection
    getStatusCollection(const TStatusCodes &aStatusCodes,
                        TStatusCodeSpecification *aStatusCodeSpecification = nullptr);

    /// Получить уровень лога.
    LogLevel::Enum getLogLevel(SDK::Driver::EWarningLevel::Enum aLevel);

    /// Отправить статус-коды.
    void emitStatusCode(int aStatusCode, int aExtendedStatus = SDK::Driver::EStatus::Actual);
    virtual void emitStatusCodes(TStatusCollection &aStatusCollection,
                                 int aExtendedStatus = SDK::Driver::EStatus::Actual);

    /// Устройство было перезагружено по питанию?
    bool isPowerReboot();

    /// Состояние окружения устройства изменилось.
    virtual bool environmentChanged();

    /// Подождать готовность.
    bool waitReady(const SWaitingData &aWaitingData, bool aReady = true);

    /// Получить уровень тревожности по буферу статус-кодов.
    virtual SDK::Driver::EWarningLevel::Enum
    getWarningLevel(const TStatusCollection &aStatusCollection);

    /// Счетчик отсутствия ответа на полловые посылки.
    int mBadAnswerCounter;

    /// Максимальное число демпфируемых полловых запросов c некорректным ответом.
    int mMaxBadAnswers;

    /// Признак принудительного включения буфера статусов.
    bool mForceStatusBufferEnabled;

    /// Версия драйвера.
    QString mVersion;

    /// После polling устройства эмитить статусы и выполнять device-specific действия.
    bool mPostPollingAction;

    /// Теоретически исправимые после пере инициализации ошибки.
    TStatusCodes mRecoverableErrors;

    /// Неустойчивые пограничные состояния (не-ошибки), в которых нельзя исправлять исправимые
    /// ошибки.
    TStatusCodes mUnsafeStatusCodes;

    /// Устройство протестировано на совместимость.
    bool mVerified;

    /// Модель соответствует своему плагину.
    bool mModelCompatibility;

    /// Мьютекс для блокировки polling при выполнении внешних операций.
    QRecursiveMutex mExternalMutex;

    /// Мьютекс для блокировки запросов к логическим ресурсам (контейнеры и т.п.).
    mutable QRecursiveMutex mResourceMutex;

    /// Экземпляр класса-описателя статусов устройства.
    DeviceStatusCode::PSpecifications mStatusCodesSpecification;

    /// Кэш состояний устройства. Фильтрованы только несуществующие, но не несущественные статусы,
    /// применен буфер статусов.
    TStatusCollection mStatusCollection;

    /// История состояний устройства. Фильтрованы несуществующие статусы, применен буфер статусов.
    typedef HistoryList<TStatusCollection> TStatusCollectionList;
    TStatusCollectionList mStatusCollectionHistory;

    /// Уровень тревожности для последнего отправленного в пп набора статусов.
    SDK::Driver::EWarningLevel::Enum mLastWarningLevel;

    /// Устройство подключено.
    bool mConnected;

    /// Статус-коды для фильтрации 3-го уровня.
    TStatusCollection mExcessStatusCollection;

    /// Флаг старой прошивки.
    bool mOldFirmware;

    /// Флаг необходимости перезагрузки устройства по питанию.
    bool mNeedReboot;

    /// Количество повторов при инициализации.
    int mInitializeRepeatCount;

    /// Может находиться авто поиском.
    bool mAutoDetectable;

    /// Статусы, наличие которых в ответе на статус запускает работу буфера статусов.
    TStatusCodes mReplaceableStatuses;

    /// Имя устройства.
    QString mDeviceName;
};

//---------------------------------------------------------------------------

/* @file Принтеры семейства Star. */

#pragma once

#include <Hardware/Printers/PortPrintersBase.h>
#include <Hardware/Printers/StarMemorySwitches.h>

//--------------------------------------------------------------------------------
class StarPrinter : public TSerialPrinterBase {
    SET_SERIES("STAR")

public:
    StarPrinter();

    /// Задаёт лог.
    virtual void setLog(ILog *aLog);

    /// Возвращает список поддерживаемых устройств.
    static QStringList getModelList();

protected:
    /// Анализирует коды статусов устройства и фильтрует лишние.
    virtual void cleanStatusCodes(TStatusCodes &aStatusCodes);

    /// Попытка самоидентификации.
    virtual bool isConnected();

    /// Получить статус.
    virtual bool getStatus(TStatusCodes &aStatusCodes);

    /// Инициализация устройства.
    virtual bool updateParameters();

    /// Инициализация регистров устройства.
    bool initializeRegisters();

    /// Напечатать чек.
    virtual bool printReceipt(const Tags::TLexemeReceipt &aLexemeReceipt);

    /// После подачи команды, связанной с печатью ждем окончания печати.
    static bool waitForPrintingEnd();

    /// Сброс.
    static bool reset();

    /// Установить параметры мем-свича.
    static bool setMemorySwitch(int aSwitch, ushort aValue);

    /// Получить параметры мем-свича.
    static bool getMemorySwitch(int aSwitch, ushort &aValue);

    /// Прочитать мем-свичей.
    void getMemorySwitches();

    /// Обновить мем-свичи.
    bool updateMemorySwitches(const CSTAR::TMemorySwitches &aMemorySwitches);

    /// Получить ответ на команду мем-свича.
    bool readMSWAnswer(QByteArray &aAnswer);

    /// Получить ответ на ASB статус.
    static bool readASBAnswer(QByteArray &aAnswer, int &aLength);

    /// Получить ответ на запрос идентификации.
    static bool readIdentificationAnswer(QByteArray &aAnswer);

    /// Напечатать картинку.
    virtual bool printImage(const QImage &aImage, const Tags::TTypes &aTags);

    /// Подождать состояния эжектора.
    bool waitEjectorState(bool aBusy);

    /// Находится ли бумага в презентере.
    bool isPaperInPresenter();

    /// Наложить маску на 1 байт и сдвинуть.
    static inline int
    shiftData(const QByteArray aAnswer, int aByteNumber, int aSource, int aShift, int aDigits);

    /// Мем-свичи.
    CSTAR::TMemorySwitches m_MemorySwitches{};

    /// Экземпляр утилитного класса для работы с мем-свичами.
    CSTAR::MemorySwitches::Utils m_MemorySwitchUtils;

    /// Модели данной реализации.
    QStringList m_Models;

    /// Время работы буфера статусов после печати.
    QDateTime m_StartPrinting;

    /// Необходимо забрать чек из презентера.
    bool m_NeedPaperTakeOut;

    /// Флаг полного поллинга.
    bool m_FullPolling;
};

//--------------------------------------------------------------------------------

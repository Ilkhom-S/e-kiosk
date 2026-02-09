/* @file Принтер Custom VKP-80 III. */

#pragma once

#include <Hardware/Printers/CustomVKP80.h>

//--------------------------------------------------------------------------------
/// Константы и команды Custom VKP-80 III.
namespace CCustom_VKP80III {
/// Команды.
namespace Command {
extern const char GetModelId[];        /// Получение идентификатора модели.
extern const char EjectorActivation[]; /// Неизменяемая часть команды активации эжектора.
} // namespace Command

extern const char ModelId[];  /// Идентификатор модели.
const char Blinking = '\x01'; /// Мигать светодиодами при презентации.
const char Pushing = 'E';     /// Выталкивание чека.
const char Retraction = 'R';  /// Ретракция чека.
} // namespace CCustom_VKP80III

//--------------------------------------------------------------------------------
template <class T> class Custom_VKP80III : public Custom_VKP80<T> {
    SET_SUBSERIES("Custom_VKP80III")

public:
    Custom_VKP80III();

protected:
    /// Инициализация устройства.
    virtual bool updateParameters();

    /// Обработка чека после печати.
    virtual bool receiptProcessing();

    /// Получить Id модели.
    virtual bool getModelId(QByteArray &aAnswer) const;

    /// Распарсить Id модели.
    virtual char parseModelId(QByteArray &aAnswer);
};

//--------------------------------------------------------------------------------
typedef SerialPOSPrinter<Custom_VKP80III<TSerialPrinterBase>> SerialCustom_VKP80III;
typedef Custom_VKP80III<TLibUSBPrinterBase> LibUSBCustom_VKP80III;

//--------------------------------------------------------------------------------

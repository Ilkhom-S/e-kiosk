/* @file Данные протокола сторожевого таймера OSMP 2.5. */

#pragma once

#include <QtCore/QByteArray>

//----------------------------------------------------------------------------
namespace COSMP25 {
/// Тег модели для идентификации.
extern const char ModelTag[];

/// Интервал пинга, [мс].
const int PingInterval = 10000;

/// Интервал пинга, [с].
const uchar PingTimeout = 30;

/// Пауза при отключении питания модема, [с].
const uchar ModemResettingPause = 2;

/// Пауза при отключении питания PC, [с].
const uchar PCResettingPause = 3;

/// Истек таймаут при регистрации ключа.
const char KeyRegisteringExpired = '\x05';

/// Максимальное количество ключей.
const int MaxKeys = 32;

/// Интервал включения питания PC, [с].
const int PCWakingUpInterval = 30 * 60;

/// Формат представления даты и времени для вывода в лог.
extern const char TimeLogFormat[];

/// Временной лаг установки времени перезагрузки PC, [с].
const int PCWakingUpLag = 5;

/// Команды.
namespace Commands {
const char GetVersion = '\x00';           /// Версия
const char SerialNumber = '\x01';         /// Серийный номер
const char ResetModem = '\x02';           /// Сброс модема
extern const char SetModemPause[];  /// Установка времени сброса модема
const char SetPingEnable = '\x03';        /// Включение таймера сторожа
extern const char SetPingTimeout[]; /// Установка времени сторожа таймера
const char SetPingDisable = '\x04';       /// Отключение таймера сторожа
const char Ping = '\x05';                 /// СБрос времени сторожа
const char ResetClock = '\x06';           /// Сброс часов
const char ResetPC = '\x0C';              /// Сброс PC
extern const char SetPCPause[];     /// Установка таймаута сброса PC
const char WriteKey = '\x11';             /// Запись ключа
const char ReadKey = '\x12';              /// Чтение ключа
const char PCWakeUpTime = '\x15';         /// Установка/получение времени включения PC

const QByteArray ResetPCWakeUpTime =
    QByteArray::fromRawData("\x15\x00", 2); /// Сброс времени включения PC
} // namespace Commands
} // namespace COSMP25

//--------------------------------------------------------------------------------

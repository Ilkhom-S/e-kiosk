/* @file Константы протокола SSP. */

#pragma once

#include <QtGlobal>

//--------------------------------------------------------------------------------
namespace CSSP {
/// Первый байт при упаковке данных.
extern const char Prefix[];

/// Маскированный проефикс.
extern const char MaskedPrefix[];

/// Флаг последовательности.
const char SequenceFlag = '\x80';

/// Константа для подсчета контрольной суммы.
const ushort Polynominal = 0x8005;

/// Последний бит для вычисления CRC.
const ushort LastBit = 0x8000;

/// Минимальный размер отклика от валидатора.
const int MinAnswerSize = 6;

/// Максимальное количество повторов команды в случае NAK-а или незаконченного/неверного ответа.
const int MaxRepeatPacket = 3;
} // namespace CSSP

//--------------------------------------------------------------------------------

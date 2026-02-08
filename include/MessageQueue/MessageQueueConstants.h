/* @file Константы для очереди сообщений */

#pragma once

#include <QtCore/QByteArray>

//----------------------------------------------------------------------------
namespace MessageQueueConstants {
/// Сообщение
const QByteArray PingMessage = "@@@PingMessage@@@";
/// Время, через которое пингуем
const int PingTime = 10000;
/// Время, за которое должен придти ответ от сервера
const int AnswerFrom_ServerTime = 30000;
}; // namespace MessageQueueConstants
//----------------------------------------------------------------------------

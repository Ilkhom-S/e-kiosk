/* @file Запросы к серверу рекламы Хумо. */

#pragma once

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Humo/Request.h>

#include <AdBackend/StatisticRecord.h>

//------------------------------------------------------------------------------
namespace Ad {
namespace Parameters {
extern const char RequestType[];
extern const char ResponseType[];
extern const char Channel[];
extern const char Channels[];

extern const char ID[];
extern const char MD5[];
extern const char Expired[];
extern const char Url[];
extern const char Text[];

extern const char Stat[];

extern const char DefaultID[];
extern const char DefaultUrl[];
extern const char DefaultMD5[];
extern const char DefaultText[];

/// Формат даты/времени в ответах сервера Хумо и в ini файле конфигурации
extern const char DateTimeFormat[];
extern const char DateFormat[];

/// Поля запроса с формой
extern const char DateForm[];
extern const char Data[];
} // namespace Parameters

namespace Requests {
extern const char ChannelList[];
extern const char Channel[];
extern const char Statistics[];
extern const char Form[];
} // namespace Requests
} // namespace Ad

//---------------------------------------------------------------------------
class AdRequest : public SDK::PaymentProcessor::Humo::Request {
public:
    explicit AdRequest(SDK::PaymentProcessor::ICore *aCore);
};

//---------------------------------------------------------------------------
class AdGetChannelsRequest : public AdRequest {
public:
    AdGetChannelsRequest(SDK::PaymentProcessor::ICore *aCore);
};

//---------------------------------------------------------------------------
class AdGetChannelRequest : public AdRequest {
public:
    AdGetChannelRequest(SDK::PaymentProcessor::ICore *aCore, const QString &aName);
};

//---------------------------------------------------------------------------
class AdStatisticRequest : public AdRequest {
public:
    AdStatisticRequest(SDK::PaymentProcessor::ICore *aCore,
                       const QList<Ad::SStatisticRecord> aStatistic);
};

//---------------------------------------------------------------------------

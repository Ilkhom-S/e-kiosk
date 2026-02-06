/* @file Запросы к серверу рекламы Хумо. */

#pragma once

#include <QtCore/QList>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Humo/Request.h>
#include <SDK/PaymentProcessor/Humo/Response.h>

#include <AdBackend/Campaign.h>

#include "AdRequests.h"

//---------------------------------------------------------------------------
class AdResponse : public SDK::PaymentProcessor::Humo::Response {
public:
    AdResponse(const SDK::PaymentProcessor::Humo::Request &aRequest,
               const QString &aResponseString);
};

//---------------------------------------------------------------------------
class AdGetChannelsResponse : public AdResponse {
public:
    AdGetChannelsResponse(const SDK::PaymentProcessor::Humo::Request &aRequest,
                          const QString &aResponseString);

    /// Возвращает список каналов рекламы
    virtual QStringList channels() const;
};

//---------------------------------------------------------------------------
class AdGetChannelResponse : public AdGetChannelsResponse {
public:
    AdGetChannelResponse(const SDK::PaymentProcessor::Humo::Request &aRequest,
                         const QString &aResponseString);

    /// Получить список кампаний канала (основную и резервную)
    QList<Ad::Campaign> getCampaigns() const;
};

//---------------------------------------------------------------------------

/* @file Запросы к серверу рекламы Хумо. */

#include "AdResponses.h"

#include <QtCore/QStringDecoder>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <AdBackend/Client.h>

#include "AdRequests.h"

namespace PPSDK = SDK::PaymentProcessor;

//------------------------------------------------------------------------------
AdResponse::AdResponse(const PPSDK::Humo::Request &aRequest, const QString &aResponseString)
    : PPSDK::Humo::Response(aRequest, aResponseString) {}

//------------------------------------------------------------------------------
AdGetChannelsResponse::AdGetChannelsResponse(const PPSDK::Humo::Request &aRequest,
                                             const QString &aResponseString)
    : AdResponse(aRequest, aResponseString) {}

//------------------------------------------------------------------------------
QStringList AdGetChannelsResponse::channels() const {
    return getParameter(Ad::Parameters::Channels).toString().split(",", Qt::SkipEmptyParts);
}

//------------------------------------------------------------------------------
AdGetChannelResponse::AdGetChannelResponse(const SDK::PaymentProcessor::Humo::Request &aRequest,
                                           const QString &aResponseString)
    : AdGetChannelsResponse(aRequest, aResponseString) {}

//------------------------------------------------------------------------------
QList<Ad::Campaign> AdGetChannelResponse::getCampaigns() const {
    QList<Ad::Campaign> result;

    Ad::Campaign c;
    c.type = getParameter(Ad::Parameters::Channel).toString();
    c.id = getParameter(Ad::Parameters::ID).toLongLong();
    c.md5 = getParameter(Ad::Parameters::MD5).toString();
    c.expired = QDateTime::from_String(getParameter(Ad::Parameters::Expired).toString(),
                                       Ad::Parameters::DateTimeFormat);
    c.url = QUrl::from_Encoded(getParameter(Ad::Parameters::Url).toByteArray());
    {
        QStringDecoder decoder("windows-1251");
        c.text = decoder.decode(
            QByteArray::from_Base64(getParameter(Ad::Parameters::Text).toByteArray()));
    }

    result << c;

    c.type = getParameter(Ad::Parameters::Channel).toString() + Ad::DefaultChannelPostfix;
    c.id = getParameter(Ad::Parameters::DefaultID).toInt();
    c.md5 = getParameter(Ad::Parameters::DefaultMD5).toString();
    c.expired = QDateTime(QDate(2999, 12, 31), QTime(23, 59, 59));
    c.url = QUrl::from_Encoded(getParameter(Ad::Parameters::DefaultUrl).toByteArray());
    {
        QStringDecoder decoder("windows-1251");
        c.text = decoder.decode(
            QByteArray::from_Base64(getParameter(Ad::Parameters::DefaultText).toByteArray()));
    }

    result << c;

    return result;
}

//------------------------------------------------------------------------------

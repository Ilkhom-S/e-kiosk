/* @file Реализация клиента, взаимодействующего с сервером рекламы. */

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtCore/QTimerEvent>
#include <QtXml/QDomDocument>

#include <SDK/PaymentProcessor/Core/ICryptService.h>
#include <SDK/PaymentProcessor/Core/INetworkService.h>
#include <SDK/PaymentProcessor/Core/IRemoteService.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <AdBackend/Client.h>
#include <AdBackend/DatabaseUtils.h>
#include <AdBackend/IDatabaseUtils.h>

namespace Ad {
extern const char DefaultChannelPostfix[] = "_default";
} // namespace Ad
#include <NetworkTaskManager/DataStream.h>
#include <NetworkTaskManager/FileDownloadTask.h>
#include <SysUtils/ISysUtils.h>

#include "AdRequests.h"
#include "AdResponses.h"

namespace PPSDK = SDK::PaymentProcessor;

using namespace std::placeholders;

namespace Ad {
/// Имя файла настроек
const QString SettingsName = "ad.ini";

/// Имена параметров в ini, описывающие рекламную кампанию
namespace Settings {
const QString Url = "ad/url";

const QString Types = "ad/types";

const QString ID = "ID";
const QString Source = "source";
const QString Expired = "expired";
const QString Text = "text";
const QString MD5 = "md5";
} // namespace Settings

//------------------------------------------------------------------------
namespace CClient {
const int ReinitInterval = 10 * 60 * 1000;
const int ContentCheckInterval = 20 * 60 * 1000;
const int ContentRecheckInterval = 3 * 60 * 1000;
const int StatisticsSendInterval = 12 * 60 * 60 * 1000;
const int StatisticsResendInterval = 30 * 60 * 1000;

const QString ThreadName = "AdClient";
} // namespace CClient

//------------------------------------------------------------------------
Client::Client(SDK::PaymentProcessor::ICore *aCore, ILog *aLog, int aKeyPair)
    : ILogable(aLog), m_Core(aCore), m_SavedTypes(0), m_ExpirationTimer(-1),
      m_CurrentDownloadCommand(-999) {
    m_Thread.setObjectName(CClient::ThreadName);
    moveToThread(&m_Thread);

    auto *terminalSettings = dynamic_cast<PPSDK::TerminalSettings *>(
        m_Core->getSettingsService()->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
    m_ContentPath = terminalSettings->getAppEnvironment().adPath;

    QDir dir;
    if (!dir.mkpath(m_ContentPath)) {
        toLog(LogLevel::Error, QString("Cannot create path %1.").arg(m_ContentPath));
    }

    m_Settings = QSharedPointer<QSettings>(
        new QSettings(ISysUtils::rm_BOM(m_ContentPath + QDir::separator() + Ad::SettingsName),
                      QSettings::IniFormat));
    // В Qt6 метод setIniCodec() удален, UTF-8 используется по умолчанию
    // В Qt5 необходимо явно установить кодировку UTF-8
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_Settings->setIniCodec("utf-8");
#endif
    m_Settings->moveToThread(&m_Thread);

    m_ServerUrl = m_Settings->value(Settings::Url).toUrl();
    m_DatabaseUtils = QSharedPointer<DatabaseUtils>(new DatabaseUtils(m_ContentPath, getLog()));
    m_Http = QSharedPointer<RequestSender>(
        new RequestSender(m_Core->getNetworkService()->getNetworkTaskManager(),
                          m_Core->getCryptService()->getCryptEngine()));

    m_Http->setCryptKeyPair(aKeyPair);
    m_Http->setResponseCreator([this](auto &&pH1, auto &&pH2) {
        return createResponse(std::forward<decltype(pH1)>(pH1), std::forward<decltype(pH2)>(pH2));
    });

    connect(m_Core->getRemoteService(),
            SIGNAL(commandStatusChanged(int, int, QVariantMap)),
            SLOT(onCommandStatusChanged(int, int, QVariantMap)));

    m_Thread.start();
}

//------------------------------------------------------------------------
Client::~Client() {
    if (m_Thread.isRunning()) {
        m_Thread.quit();
        m_Thread.wait();
    }
}

//------------------------------------------------------------------------
QSharedPointer<QSettings> Client::getSettings() {
    return m_Settings;
}

//------------------------------------------------------------------------
void Client::reinitialize() {
    toLog(LogLevel::Normal, "Initializing advertising system client.");

    toLog(LogLevel::Normal, QString("Server: %1.").arg(m_ServerUrl.toString()));
    toLog(LogLevel::Normal, QString("Content path: %1.").arg(m_ContentPath));

    QDir dir;
    if (!dir.mkpath(m_ContentPath)) {
        toLog(LogLevel::Error, QString("Cannot create path %1.").arg(m_ContentPath));

        QTimer::singleShot(CClient::ReinitInterval, this, SLOT(reinitialize()));

        return;
    }

    foreach (auto channel, m_Settings->value(Settings::Types).toStringList()) {
        checkExpiration(getAdInternal(channel));
    }
}

//------------------------------------------------------------------------------
void Client::update() {
    // Проинициализировались, теперь можно проверить обновление
    QTimer::singleShot(0, this, SLOT(doUpdate()));

    // Пробуем отправить статистику
    QTimer::singleShot(0, this, SLOT(sendStatistics()));
}

//------------------------------------------------------------------------------
Response *Client::sendRequest(const QUrl &aUrl, Request &aRequest) {
    if (m_Http) {
        toLog(LogLevel::Normal, QString("> %1.").arg(aRequest.toLogString()));

        RequestSender::ESendError error = RequestSender::ESendError::NoError;

        std::unique_ptr<Response> response(
            m_Http->post(aUrl, aRequest, RequestSender::Solid, error));

        if (response) {
            toLog(LogLevel::Normal, QString("< %1.").arg(response->toLogString()));
        } else {
            toLog(LogLevel::Error,
                  QString("Failed to send: %1.").arg(RequestSender::translateError(error)));
        }

        return response.release();
    }

    return nullptr;
}

//------------------------------------------------------------------------------
Response *Client::createResponse(const Request &aRequest, const QString &aResponseString) {
    QString requestType = aRequest.getParameter(Ad::Parameters::RequestType).toString();

    if (requestType == Ad::Requests::ChannelList) {
        return new AdGetChannelsResponse(aRequest, aResponseString);
    }
    if (requestType == Ad::Requests::Channel) {
        return new AdGetChannelResponse(aRequest, aResponseString);
    }

    return new AdResponse(aRequest, aResponseString);
}

//------------------------------------------------------------------------
void Client::doUpdate() {
    toLog(LogLevel::Normal, "Getting channels list...");

    m_UpdateStamp = QDateTime::currentDateTime();

    AdGetChannelsRequest request(m_Core);

    std::unique_ptr<Response> response(sendRequest(m_ServerUrl, request));

    if (!response || !response->isOk()) {
        toLog(LogLevel::Error,
              QString("Failed to get channels list: (%1) %2")
                  .arg(response ? response->getError() : 0)
                  .arg(response ? response->getErrorMessage() : ""));

        QTimer::singleShot(CClient::ReinitInterval, this, SLOT(doUpdate()));
    } else {
        m_TypeList = dynamic_cast<AdGetChannelsResponse *>(response.get())->channels();

        QTimer::singleShot(0, this, SLOT(updateTypes()));
    }
}

//------------------------------------------------------------------------
void Client::updateTypes() {
    foreach (auto channel, m_TypeList) {
        AdGetChannelRequest request(m_Core, channel);

        std::unique_ptr<Response> response(sendRequest(m_ServerUrl, request));

        if (!response || !response->isOk()) {
            toLog(LogLevel::Error,
                  QString("Failed to get channel '%1': (%2) %3")
                      .arg(channel)
                      .arg(response ? response->getError() : 0)
                      .arg(response ? response->getErrorMessage() : ""));

            QTimer::singleShot(CClient::ReinitInterval, this, SLOT(updateTypes()));
            break;
        }
        foreach (auto campaign,
                 static_cast<AdGetChannelResponse *>(response.get())->getCampaigns()) {
            toLog(LogLevel::Debug, QString("Receive campaign: %1").arg(campaign.toString()));

            m_Campaigns.insert(campaign.type, campaign);
        }
    }

    QTimer::singleShot(0, this, SLOT(checkContent()));
}

//------------------------------------------------------------------------
void Client::checkContent() {
    m_TypeDownloadList.clear();

    foreach (auto campaign, m_Campaigns.values()) {
        Campaign c = getAdInternal(campaign.type);

        if (!c.isEqual(campaign)) {
            if (campaign.isDownloaded()) {
                m_TypeDownloadList << campaign.type;
            } else {
                // сохраняем содержимое канала
                saveChannel(campaign);
            }
        }
    }

    download();
}

//------------------------------------------------------------------------
void Client::download() {
    if (m_TypeDownloadList.isEmpty()) {
        toLog(LogLevel::Normal, "Download nothing. All up to date.");

        if (m_SavedTypes != 0) {
            emit contentUpdated();
        }

        return;
    }

    QString campaignName = m_TypeDownloadList.at(0);

    if (m_Campaigns[campaignName].isValid() && m_Campaigns[campaignName].isDownloaded()) {
        toLog(LogLevel::Normal,
              QString("Download campaign [%1]:%2...")
                  .arg(m_Campaigns[campaignName].id)
                  .arg(m_Campaigns[campaignName].type));

        m_CurrentDownloadCommand =
            m_Core->getRemoteService()->registerUpdateCommand(PPSDK::IRemoteService::AdUpdate,
                                                              m_Campaigns[campaignName].url,
                                                              QUrl(),
                                                              m_Campaigns[campaignName].md5);

        if (m_CurrentDownloadCommand == 0) {
            toLog(LogLevel::Warning, "Failed try to start update ad. Try later.");

            QTimer::singleShot(CClient::ReinitInterval, this, SLOT(download()));
        }
    } else {
        m_TypeDownloadList.removeAt(0);

        QTimer::singleShot(10, this, SLOT(download()));
    }
}

//------------------------------------------------------------------------
void Client::sendStatistics() {
    QList<SStatisticRecord> statisticList;

    int retryTimeout = CClient::StatisticsResendInterval;

    if (!m_DatabaseUtils->getUnsentStatisticRecords(statisticList)) {
        toLog(LogLevel::Error, "Cannot get ad statistics from DB.");
    } else {
        if (!statisticList.isEmpty()) {
            AdStatisticRequest request(m_Core, statisticList);

            std::unique_ptr<Response> response(sendRequest(m_ServerUrl, request));

            if (!response || !response->isOk()) {
                toLog(LogLevel::Error,
                      QString("Failed to sent statistic: (%1) %2")
                          .arg(response ? response->getError() : 0)
                          .arg(response ? response->getErrorMessage() : ""));
            } else {
                toLog(LogLevel::Normal, "Mark statistic as sent.");

                m_DatabaseUtils->deleteStatisticRecords(statisticList);

                retryTimeout = CClient::StatisticsSendInterval;
            }
        } else {
            toLog(LogLevel::Normal, "Statistic: nothing to send.");

            retryTimeout = CClient::StatisticsSendInterval;
        }
    }

    QTimer::singleShot(retryTimeout, this, SLOT(sendStatistics()));
}

//---------------------------------------------------------------------------
QList<Campaign> Client::getAds() const {
    QList<Campaign> result;

    foreach (auto ch, m_Settings->value(Settings::Types).toStringList()) {
        result << getAdInternal(ch);
    }

    return result;
}

//---------------------------------------------------------------------------
Ad::Campaign Client::getAdInternal(const QString &aType) const {
    Ad::Campaign camp;

    m_Settings->beginGroup("ad_" + aType);

    camp.id = m_Settings->value(Settings::ID, -1).toLongLong();
    camp.type = aType;
    camp.url = m_Settings->value(Settings::Source).toUrl();
    camp.md5 = m_Settings->value(Settings::MD5).toString();
    camp.expired = QDateTime::fromString(m_Settings->value(Settings::Expired).toString(),
                                         Ad::Parameters::DateTimeFormat);
    camp.text = m_Settings->value(Settings::Text).toString();

    m_Settings->endGroup();

    return camp;
}

//------------------------------------------------------------------------
Ad::Campaign Client::getAd(const QString &aType) const {
    auto channel = getAdInternal(aType);

    QString bannerPath =
        m_ContentPath + QDir::separator() + channel.type + QDir::separator() + "banner.swf";

    if (!channel.isValid() || channel.isExpired()) {
        return getAdInternal(aType + Ad::DefaultChannelPostfix);
    }

    if (!channel.isDefault() && channel.isBanner() && !QFile::exists(bannerPath)) {
        toLog(LogLevel::Error,
              QString("Failed to get ad content: path '%1' not found. Use default channel.")
                  .arg(bannerPath));

        return getAdInternal(aType + Ad::DefaultChannelPostfix);
    }

    return channel;
}

//------------------------------------------------------------------------
void Client::saveChannel(const Campaign &aCampaign) {
    m_Settings->beginGroup("ad_" + aCampaign.type);

    m_Settings->setValue(Settings::ID, aCampaign.id);
    m_Settings->setValue(Settings::Source, aCampaign.url.toString());
    m_Settings->setValue(Settings::MD5, aCampaign.md5);
    m_Settings->setValue(Settings::Expired,
                         aCampaign.expired.toString(Ad::Parameters::DateTimeFormat));
    m_Settings->setValue(Settings::Text, aCampaign.text);

    m_Settings->endGroup();

    QStringList types = m_Settings->value(Settings::Types).toStringList();
    if (!types.contains(aCampaign.type)) {
        types << aCampaign.type;
        m_Settings->setValue(Settings::Types, types);
    }

    m_Settings->sync();

    m_SavedTypes++;

    checkExpiration(aCampaign);
}

//------------------------------------------------------------------------
void Client::onCommandStatusChanged(int aID, int aStatus, QVariantMap aParameters) {
    if (m_CurrentDownloadCommand != aID) {
        return;
    }

    // Нас интересуют только финальные состояния
    switch (aStatus) {
    case PPSDK::IRemoteService::OK: {
        if (!m_TypeDownloadList.isEmpty()) {
            QString type = m_TypeDownloadList.at(0);

            saveChannel(m_Campaigns[type]);

            m_CurrentDownloadCommand = 0;
            m_TypeDownloadList.removeAt(0);

            toLog(LogLevel::Normal, QString("Download '%1' complete.").arg(type));

            QTimer::singleShot(0, this, SLOT(download()));
        }
    } break;

    case PPSDK::IRemoteService::Error: {
        toLog(LogLevel::Error, "Failed to download ad content. Try later.");

        // Перезапускаем загрузку компаний
        QTimer::singleShot(CClient::ContentCheckInterval, this, SLOT(download()));
    } break;

    default:
        break;
    }
}

//------------------------------------------------------------------------
QString Client::getContent(const QString &aType) {
    auto channel = getAd(aType);

    return channel.isBanner() ? m_ContentPath + QDir::separator() + channel.type : channel.text;
}

//------------------------------------------------------------------------
void Client::addEvent(const QString &aType) {
    auto channel = getAd(aType);

    if (channel.isValid()) {
        if (channel.isBanner()) {
            // Баннер регистрируем только 1 в день
            m_DatabaseUtils->setStatisticRecord(channel.id, channel.type, 1);
        } else {
            m_DatabaseUtils->addStatisticRecord(channel.id, channel.type);
        }
    } else {
        toLog(LogLevel::Warning, QString("Skip not valid channel '%1' event.").arg(aType));
    }
}

//------------------------------------------------------------------------
bool Client::updateExpired() const {
    return m_UpdateStamp.isNull() ||
           (m_UpdateStamp.addSecs(6 * 60 * 60) < QDateTime::currentDateTime());
}

//------------------------------------------------------------------------
void Client::timerEvent(QTimerEvent *aEvent) {
    if (aEvent->timerId() == m_ExpirationTimer) {
        emit contentExpired();
    }
}

//------------------------------------------------------------------------
void Client::checkExpiration(const Campaign &aCampaign) {
    // Планируем перезагрузку ТК по истечении рекламы
    if (aCampaign.isValid() && !aCampaign.isExpired() && !aCampaign.expired.isNull()) {
        toLog(LogLevel::Debug,
              QString("expTime: %1  camp exp time: %2")
                  .arg(m_ExpirationTime.toString())
                  .arg(aCampaign.expired.toString()));

        if (m_ExpirationTime.isNull() || m_ExpirationTime > aCampaign.expired) {
            if (m_ExpirationTimer >= 0) {
                killTimer(m_ExpirationTimer);
            }

            int interval =
                static_cast<int>(QDateTime::currentDateTime().msecsTo(aCampaign.expired));

            if (interval > 0) {
                m_ExpirationTime = aCampaign.expired;
                toLog(LogLevel::Normal,
                      QString("Start expiration timer on '%1'")
                          .arg(m_ExpirationTime.toString("yyyy-MM-dd hh:mm:ss")));

                m_ExpirationTimer = startTimer(interval);
            }
        }
    }
}

//------------------------------------------------------------------------
} // namespace Ad

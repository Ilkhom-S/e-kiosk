/* @file Загрузчик списков номиналов для пиновых провайдеров. */

// boost

#include "PinLoader.h"

#include <QtCore/QScopedPointer>

#include <SDK/PaymentProcessor/Components.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <boost/bind/bind.hpp>
#include <boost/foreach.hpp>

#include "PinGetCardListRequest.h"
#include "PinGetCardListResponse.h"
#include "PinPayment.h"

namespace CPinLoader {
const char ThreadName[] = "PinLoaderThread";
const char GetCardsRequestName[] = "GETCARDS";

const int FirstLoadPinTimeout = 2 * 60; // таймаут первой попытки загрузки пинов (сек)
const int ErrorRetryTimeout =
    5 * 60; // таймаут повторной попытки загрузки пинов в случае ошибок (сек)
const int NextLoadTimeout =
    12 * 60 * 60; // таймаут повторной попытки загрузки пинов в случае успеха (сек)

const int PinsExpirationTime =
    24 * 60 *
    60; // время в секундах, до следующей попытки загрузки успешно загруженного списка пинов
} // namespace CPinLoader

namespace CProcessorType {
const QString HumoPin = "humo_pin";
} // namespace CProcessorType

//------------------------------------------------------------------------------
PinLoader::PinLoader(PaymentFactoryBase *aPaymentFactoryBase)
    : QObject(aPaymentFactoryBase), m_PaymentFactoryBase(aPaymentFactoryBase), m_IsStopping(false) {
    ILogable::setLog(aPaymentFactoryBase->getLog("PinLoader"));

    m_PinThread.setObjectName(CPinLoader::ThreadName);

    m_PinTimer.moveToThread(&m_PinThread);

    connect(&m_PinThread, SIGNAL(started()), SLOT(onPinThreadStarted()), Qt::DirectConnection);
    connect(&m_PinThread, SIGNAL(finished()), &m_PinTimer, SLOT(stop()), Qt::DirectConnection);

    connect(&m_PinTimer, SIGNAL(timeout()), SLOT(onLoadPinList()), Qt::DirectConnection);

    m_PinThread.start();
}

//------------------------------------------------------------------------------
PinLoader::~PinLoader() {
    m_IsStopping = true;

    m_PinThread.quit();
    if (!m_PinThread.wait(3000)) {
        toLog(LogLevel::Error, "Terminate PinLoader thread.");
        m_PinThread.terminate();
    }
}

//------------------------------------------------------------------------------
PPSDK::SProvider PinLoader::getProviderSpecification(const PPSDK::SProvider &aProvider) {
    QMutexLocker lock(&m_PinMutex);

    if (m_PinProviders.contains(aProvider.id) && m_PinProviders[aProvider.id].lastLoad.isValid()) {
        // Возвращаем описание с заполненными номиналами.
        return m_PinProviders[aProvider.id].provider;
    }

    return {};
}

//------------------------------------------------------------------------------
void PinLoader::onPinThreadStarted() {
    m_PinTimer.start(CPinLoader::FirstLoadPinTimeout * 1000);
}

//------------------------------------------------------------------------------
Response *PinLoader::createResponse(const SDK::PaymentProcessor::Humo::Request &aRequest,
                                    const QString &aResponseString) {
    return new PinGetCardListResponse(aRequest, aResponseString);
}

//------------------------------------------------------------------------------
void PinLoader::findPinProviders() {
    PPSDK::ISettingsService *settingsService =
        m_PaymentFactoryBase->getCore()->getSettingsService();
    if (!settingsService) {
        toLog(LogLevel::Error, "Failed to get settings service.");
        return;
    }

    if (m_PinProviders.isEmpty()) {
        toLog(LogLevel::Normal, "Updating list...");

        PPSDK::DealerSettings *dealerSettings = dynamic_cast<PPSDK::DealerSettings *>(
            settingsService->getAdapter(PPSDK::CAdapterNames::DealerAdapter));
        if (!dealerSettings) {
            toLog(LogLevel::Error, "Failed to get dealer settings.");
            return;
        }

        foreach (qint64 providerId, dealerSettings->getProviders(CProcessorType::HumoPin)) {
            PPSDK::SProvider provider = dealerSettings->getProvider(providerId);

            if (provider.processor.requests.contains(CPinLoader::GetCardsRequestName)) {
                QMutexLocker lock(&m_PinMutex);

                m_PinProviders.insert(provider.id, SProviderPins(provider));
            }
        }
    }
}

//------------------------------------------------------------------------------
void PinLoader::onLoadPinList() {
    if (m_PinProviders.isEmpty()) {
        findPinProviders();
    }

    if (m_PinProviders.isEmpty()) {
        toLog(LogLevel::Normal, "No providers.");

        m_PinThread.quit();

        return;
    }

    PPSDK::ISettingsService *settingsService =
        m_PaymentFactoryBase->getCore()->getSettingsService();
    if (!settingsService) {
        toLog(LogLevel::Error, "Failed to get settings service.");
        return;
    }

    PPSDK::TerminalSettings *terminalSettings = dynamic_cast<PPSDK::TerminalSettings *>(
        settingsService->getAdapter(PPSDK::CAdapterNames::TerminalAdapter));
    if (!terminalSettings) {
        toLog(LogLevel::Error, "Failed to get terminal settings.");
        return;
    }

    SDK::PaymentProcessor::Humo::RequestSender http(m_PaymentFactoryBase->getNetworkTaskManager(),
                                                    m_PaymentFactoryBase->getCryptEngine());
    http.setResponseCreator([this](const SDK::PaymentProcessor::Humo::Request &aRequest,
                                   const QString &aResponseString) {
        return createResponse(aRequest, aResponseString);
    });

    int failedCount = 0;

    foreach (qint64 id, m_PinProviders.keys()) {
        if (m_IsStopping) {
            toLog(LogLevel::Normal, "Updating providers stopped.");
            break;
        }

        if (m_PinProviders[id].lastLoad.isValid() &&
            m_PinProviders[id].lastLoad.addSecs(CPinLoader::PinsExpirationTime) >
                QDateTime::currentDateTime()) {
            // пропускаем т.к. еще не закончилось время действия загруженных пинов
            continue;
        }

        PPSDK::SProvider &provider = m_PinProviders[id].provider;

        toLog(LogLevel::Normal,
              QString("Updating provider %1 (%2).").arg(provider.id).arg(provider.name));

        PinGetCardListRequest request(terminalSettings->getKeys()[provider.processor.keyPair]);

        toLog(LogLevel::Normal,
              QString("Sending request to url: %1. Fields: %2.")
                  .arg(provider.processor.requests[CPinLoader::GetCardsRequestName].url)
                  .arg(request.toLogString()));

        SDK::PaymentProcessor::Humo::RequestSender::ESendError error;

        http.setCryptKeyPair(provider.processor.keyPair);

        QScopedPointer<Response> response(
            http.post(provider.processor.requests[CPinLoader::GetCardsRequestName].url,
                      request,
                      RequestSender::Solid,
                      error));

        if (!response) {
            toLog(LogLevel::Error,
                  QString("Failed to retrieve pin card list. Network error: %1.")
                      .arg(SDK::PaymentProcessor::Humo::RequestSender::translateError(error)));

            ++failedCount;

            continue;
        }

        toLog(LogLevel::Normal,
              QString("Response received. Fields: %1.").arg(response->toLogString()));

        if (!response->isOk()) {
            toLog(LogLevel::Error, QString("Server response error: %1").arg(response->getError()));

            ++failedCount;

            continue;
        }

        updatePinList(id, dynamic_cast<PinGetCardListResponse *>(response.data())->getCards());
    }

    if (failedCount != 0) {
        toLog(LogLevel::Error,
              QString("Pin nominals update failed for %1 in %2 providers. Retry after %3 min.")
                  .arg(failedCount)
                  .arg(m_PinProviders.size())
                  .arg(CPinLoader::ErrorRetryTimeout / 60.));
    } else {
        toLog(LogLevel::Normal, "All pin nominals successfully updated.");
    }

    // меняем таймаут в зависимости от результата.
    m_PinTimer.start(
        ((failedCount != 0) ? CPinLoader::ErrorRetryTimeout : CPinLoader::NextLoadTimeout) * 1000);
}

//------------------------------------------------------------------------------
void PinLoader::updatePinList(qint64 aProvider, const QList<SPinCard> &aCards) {
    QMutexLocker lock(&m_PinMutex);

    m_PinProviders[aProvider].pins = aCards;
    m_PinProviders[aProvider].lastLoad = QDateTime::currentDateTime();

    PPSDK::SProvider &provider = m_PinProviders[aProvider].provider;

    BOOST_FOREACH (PPSDK::SProviderField &field, provider.fields) {
        if (field.id == CPin::UIFieldName) {
            field.enum_Items.clear();

            foreach (const SPinCard &card, m_PinProviders[aProvider].pins) {
                PPSDK::SProviderField::SEnum_Item item;
                item.title = card.name;
                item.value = card.id;

                field.enum_Items << item;
            }

            toLog(LogLevel::Normal,
                  QString("Pin card list was successfully updated operator: %1 (%2).")
                      .arg(provider.id)
                      .arg(provider.name));
        }
    }
}

//------------------------------------------------------------------------------
QList<SPinCard> PinLoader::getPinCardList(qint64 aProvider) {
    QMutexLocker lock(&m_PinMutex);

    if (m_PinProviders.contains(aProvider) && m_PinProviders[aProvider].lastLoad.isValid()) {
        // Возвращаем описание с заполненными номиналами.
        return m_PinProviders[aProvider].pins;
    }

    return {};
}

//------------------------------------------------------------------------------

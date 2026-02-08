/* @file Прокси класс для получения информации из конфигураций в скриптах */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QString>

#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/ISettingsService.h>
#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/Directory.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <algorithm>

namespace SDK {
namespace PaymentProcessor {

class ICore;
class IGUIService;

namespace Scripting {

/// Преобразует последовательность в строку.
template <typename T> QString seq2Str2(T aSequence) {
    QStringList result;

    foreach (auto i, aSequence) {
        result << i.toString();
    }

    std::sort(result.begin(), result.end());

    return result.join(";");
}

//------------------------------------------------------------------------------
/// Настройки терминала для скриптов.
class TerminalSettings : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString AP READ getAp CONSTANT)
    Q_PROPERTY(QString version READ getVersion CONSTANT)
    Q_PROPERTY(QString dataPath READ getDataPath CONSTANT)
    Q_PROPERTY(QString contentPath READ getContentPath CONSTANT)
    Q_PROPERTY(QString interfacePath READ getInterfacePath CONSTANT)
    Q_PROPERTY(QString skinPath READ getCurrentSkinPath CONSTANT)
    Q_PROPERTY(QString adProfile READ getAdProfile CONSTANT)
    Q_PROPERTY(QString minNote READ getMinNote CONSTANT)
    Q_PROPERTY(QString currencyId READ getCurrencyId CONSTANT)
    Q_PROPERTY(QString currencyCode READ getCurrencyCode CONSTANT)
    Q_PROPERTY(QString currencyName READ getCurrencyName CONSTANT)
    Q_PROPERTY(QString currencyAllNotes READ getCurrencyAllNotes CONSTANT)
    Q_PROPERTY(QString currencyAllCoins READ getCurrencyAllCoins CONSTANT)
    Q_PROPERTY(QString enabledNotes READ geEnabledNotes CONSTANT)
    Q_PROPERTY(QString enabledCoins READ geEnabledCoins CONSTANT)
    Q_PROPERTY(QString disabledNotes READ getDisabledNotes CONSTANT)
    Q_PROPERTY(QString disabledCoins READ getDisabledCoins CONSTANT)

public:
    TerminalSettings(ICore *aCore);

public slots:
    /// Если комбинация провайдера и заполненных полей подходит для входа в сервисное меню.
    bool isItServiceProvider(qint64 aProvider, const QVariantMap &aParameters);

private:
    QString getAp() const { return m_TerminalSettings->getKeys()[0].ap; }
    QString getDataPath() const { return m_TerminalSettings->getAppEnvironment().userDataPath; }
    QString getContentPath() const { return m_TerminalSettings->getAppEnvironment().contentPath; }
    QString getVersion() const { return m_TerminalSettings->getAppEnvironment().version; }
    QString getAdProfile() const { return m_TerminalSettings->getAdProfile(); }
    QString getMinNote() const { return m_TerminalSettings->getCommonSettings().minPar.toString(); }
    QString getCurrencyId() const {
        return QString::number(m_TerminalSettings->getCurrencySettings().id);
    }
    QString getCurrencyCode() const { return m_TerminalSettings->getCurrencySettings().code; }
    QString getCurrencyName() const { return m_TerminalSettings->getCurrencySettings().name; }
    QString getCurrencyAllNotes() const {
        return seq2Str2(m_TerminalSettings->getCurrencySettings().notes);
    }
    QString getCurrencyAllCoins() const {
        return seq2Str2(m_TerminalSettings->getCurrencySettings().coins);
    }
    QString geEnabledNotes() const {
        return seq2Str2(m_TerminalSettings->getCommonSettings().enabledParNotesList);
    }
    QString geEnabledCoins() const {
        return seq2Str2(m_TerminalSettings->getCommonSettings().enabledParCoinsList);
    }

    QString getDisabledNotes() const {
        QSet<Currency::Nominal> allNotes(m_TerminalSettings->getCurrencySettings().notes.begin(),
                                         m_TerminalSettings->getCurrencySettings().notes.end());
        return seq2Str2(
            allNotes.subtract(m_TerminalSettings->getCommonSettings().enabledParNotesList).values());
    }

    QString getDisabledCoins() const {
        QSet<Currency::Nominal> allCoins(m_TerminalSettings->getCurrencySettings().coins.begin(),
                                         m_TerminalSettings->getCurrencySettings().coins.end());
        return seq2Str2(
            allCoins.subtract(m_TerminalSettings->getCommonSettings().enabledParCoinsList).values());
    }

    QString getInterfacePath() const {
        return m_TerminalSettings->getAppEnvironment().interfacePath;
    }
    QString getCurrentSkinPath() const;

private:
    SDK::PaymentProcessor::TerminalSettings *m_TerminalSettings;
    SDK::PaymentProcessor::IGUIService *m_GuiService;
};

//------------------------------------------------------------------------------
class DealerSettings : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString pointName READ getPointName)
    Q_PROPERTY(QString pointAddress READ getPointAddress CONSTANT)
    Q_PROPERTY(QString name READ getName CONSTANT)
    Q_PROPERTY(QString address READ getAddress CONSTANT)
    Q_PROPERTY(QString inn READ getInn CONSTANT)
    Q_PROPERTY(QString kbk READ getKbk CONSTANT)
    Q_PROPERTY(QString phone READ getPhone CONSTANT)
    Q_PROPERTY(QString isBank READ getIsBank CONSTANT)
    Q_PROPERTY(QString operatorName READ getOperatorName CONSTANT)
    Q_PROPERTY(QString operatorAddress READ getOperatorAddress CONSTANT)
    Q_PROPERTY(QString operatorInn READ getOperatorInn CONSTANT)
    Q_PROPERTY(QString operatorContractNumber READ getOperatorContractNumber CONSTANT)
    Q_PROPERTY(QString bankName READ getBankName CONSTANT)
    Q_PROPERTY(QString bankAddress READ getBankAddress CONSTANT)
    Q_PROPERTY(QString bankBik READ getBankBik CONSTANT)
    Q_PROPERTY(QString bankInn READ getBankInn CONSTANT)
    Q_PROPERTY(QString bankPhone READ getBankPhone CONSTANT)
    Q_PROPERTY(QString bankContractNumber READ getBankContractNumber CONSTANT)

public:
    DealerSettings(ICore *m_Core);

public slots:
    /// Получение разрешения на платёж.
    bool isPaymentAllowed(const QVariantMap &aParameters) const;

    /// Получение списка комиссий для отображения.
    QObject *getCommissions(qint64 aProvider, const QVariantMap &aParameters, double aAmount);

private:
    QString getPointName() const { return m_PersonalSettings.pointName; }
    QString getPointAddress() const { return m_PersonalSettings.pointAddress; }
    QString getName() const { return m_PersonalSettings.name; }
    QString getAddress() const {
        return m_PersonalSettings.businessAddress.isEmpty() ? m_PersonalSettings.address
                                                           : m_PersonalSettings.businessAddress;
    }
    QString getInn() const { return m_PersonalSettings.inn; }
    QString getKbk() const { return m_PersonalSettings.kbk; }
    QString getPhone() const { return m_PersonalSettings.phone; }
    QString getIsBank() const { return m_PersonalSettings.isBank; }
    QString getOperatorName() const { return m_PersonalSettings.operatorName; }
    QString getOperatorAddress() const { return m_PersonalSettings.operatorAddress; }
    QString getOperatorInn() const { return m_PersonalSettings.operatorInn; }
    QString getOperatorContractNumber() const { return m_PersonalSettings.operatorContractNumber; }
    QString getBankName() const { return m_PersonalSettings.bankName; }
    QString getBankAddress() const { return m_PersonalSettings.bankAddress; }
    QString getBankBik() const { return m_PersonalSettings.bankBik; }
    QString getBankInn() const { return m_PersonalSettings.bankInn; }
    QString getBankPhone() const { return m_PersonalSettings.bankPhone; }
    QString getBankContractNumber() const { return m_PersonalSettings.bankContractNumber; }

private:
    SDK::PaymentProcessor::DealerSettings *m_Settings;
    SDK::PaymentProcessor::SPersonalSettings m_PersonalSettings;
};

//------------------------------------------------------------------------------
class SCommission : public QObject {
    Q_OBJECT

    Q_PROPERTY(double minLimit READ getMinLimit CONSTANT)
    Q_PROPERTY(double maxLimit READ getMaxLimit CONSTANT)
    Q_PROPERTY(double minCharge READ getMinCharge CONSTANT)
    Q_PROPERTY(double maxCharge READ getMaxCharge CONSTANT)
    Q_PROPERTY(double value READ getValue CONSTANT)
    Q_PROPERTY(bool isPercent READ getIsPercent CONSTANT)
    Q_PROPERTY(bool hasLimits READ getHasLimits CONSTANT)
    Q_PROPERTY(bool hasMinLimit READ getHasMinLimit CONSTANT)
    Q_PROPERTY(bool hasMaxLimit READ getHasMaxLimit CONSTANT)

public:
    SCommission(const Commission &aCommission, QObject *aParent)
        : QObject(aParent), m_Commission(aCommission) {}

private:
    double getMinLimit() const { return m_Commission.getMinLimit(); }
    double getMaxLimit() const { return m_Commission.getMaxLimit(); }
    double getMinCharge() const { return m_Commission.getMinCharge(); }
    double getMaxCharge() const { return m_Commission.getMaxCharge(); }
    double getValue() const { return m_Commission.getValue(); }
    bool getIsPercent() const { return m_Commission.getType() == Commission::Percent; }
    bool getHasLimits() const { return m_Commission.hasLimits(); }
    bool getHasMinLimit() const { return m_Commission.hasMinLimit(); }
    bool getHasMaxLimit() const { return m_Commission.hasMaxLimit(); }

private:
    Commission m_Commission;
};

//------------------------------------------------------------------------------
class Settings : public QObject {
    Q_OBJECT

    Q_PROPERTY(QObject *dealer READ getDealerSettings CONSTANT)
    Q_PROPERTY(QObject *terminal READ getTerminalSettings CONSTANT)

public:
    Settings(ICore *aCore)
        : m_Core(aCore), m_TerminalSettingsProxy(aCore), m_DealerSettingsProxy(aCore) {}

private:
    QObject *getDealerSettings() { return &m_DealerSettingsProxy; }
    QObject *getTerminalSettings() { return &m_TerminalSettingsProxy; }

private:
    ICore *m_Core;
    TerminalSettings m_TerminalSettingsProxy;
    DealerSettings m_DealerSettingsProxy;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

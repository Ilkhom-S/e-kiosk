/* @file Средства подсчёта комиссий. */

// Stl

#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>

#include <SDK/PaymentProcessor/Settings/Commissions.h>

#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace SDK {
namespace PaymentProcessor {

//----------------------------------------------------------------------------
Commission::Commission()
    : m_Value(0.0), m_Above(CCommissions::DefaultAboveValue),
      m_Below(CCommissions::DefaultBelowValue), m_MinCharge(CCommissions::MinChargeValue),
      m_MaxCharge(CCommissions::MaxChargeValue), m_Type(Absolute), m_Round(Bank),
      m_Base(AmountAll) {}

//----------------------------------------------------------------------------
bool Commission::contains(double aSum) const {
    return qFuzzyCompare(aSum, m_Below) || ((aSum > m_Above) && (aSum < m_Below));
}

//----------------------------------------------------------------------------
double Commission::getMinLimit() const {
    return m_Above;
}

//----------------------------------------------------------------------------
double Commission::getMaxLimit() const {
    return m_Below;
}

//----------------------------------------------------------------------------
bool Commission::hasMinLimit() const {
    return !qFuzzyCompare(getMinLimit(), CCommissions::DefaultAboveValue);
}

//----------------------------------------------------------------------------
bool Commission::hasMaxLimit() const {
    return !qFuzzyCompare(getMaxLimit(), CCommissions::DefaultBelowValue);
}

//----------------------------------------------------------------------------
bool Commission::hasLimits() const {
    return hasMinLimit() || hasMaxLimit();
}

//----------------------------------------------------------------------------
double Commission::getValueFor(double aSum, bool aAtAmount) const {
    double commission = 0;

    if (m_Type == Percent) {
        if (aAtAmount || m_Base == Amount) {
            // Комиссия считается по сумме к зачислению
            commission = aSum - (aSum / (1 + m_Value / 100.0));
        } else {
            // Комиссия считается по внесённым средствам
            commission = m_Value * aSum * 0.01;
        }
    } else {
        commission = m_Value;
    }

    return commission < m_MinCharge   ? m_MinCharge
           : commission > m_MaxCharge ? m_MaxCharge
                                      : commission;
}

//----------------------------------------------------------------------------
double Commission::getValue() const {
    return m_Value;
}

//----------------------------------------------------------------------------
Commission::Type Commission::getType() const {
    return m_Type;
}

//----------------------------------------------------------------------------
Commission::Base Commission::getBase() const {
    return m_Base;
}

//----------------------------------------------------------------------------
double Commission::getMinCharge() const {
    return m_MinCharge;
}

//----------------------------------------------------------------------------
double Commission::getMaxCharge() const {
    return m_MaxCharge;
}

//----------------------------------------------------------------------------
bool Commission::operator>(const Commission &aOther) const {
    if ((qFuzzyCompare(m_Above, aOther.m_Above) && (m_Below < aOther.m_Below)) ||
        (qFuzzyCompare(m_Below, aOther.m_Below) && (m_Above > aOther.m_Above))) {
        return true;
    }

    return ((m_Above > aOther.m_Above) && (m_Below < aOther.m_Below));
}

//----------------------------------------------------------------------------
bool Commission::operator<(const Commission &aOther) const {
    if (qFuzzyCompare(aOther.m_Below, CCommissions::DefaultBelowValue) &&
        qFuzzyCompare(aOther.m_Above, CCommissions::DefaultAboveValue)) {
        return true;
    }

    if (qFuzzyCompare(m_Below, CCommissions::DefaultBelowValue) &&
        qFuzzyCompare(m_Above, CCommissions::DefaultAboveValue)) {
        return false;
    }

    return (m_Above < aOther.m_Above);
}

//----------------------------------------------------------------------------
Commission Commission::from_Settings(const TPtree &aSettings) {
    Commission commission;

    commission.m_Type =
        (aSettings.get<QString>("<xmlattr>.type").toLower() == "absolute") ? Absolute : Percent;
    commission.m_Value = aSettings.get<double>("<xmlattr>.amount");

    QString minChargeValue =
        aSettings
            .get<QString>("<xmlattr>.min_charge", QString::number(CCommissions::MinChargeValue))
            .trimmed();
    commission.m_MinCharge =
        minChargeValue.isEmpty() ? CCommissions::MinChargeValue : minChargeValue.toDouble();

    QString maxChargeValue =
        aSettings
            .get<QString>("<xmlattr>.max_charge", QString::number(CCommissions::MaxChargeValue))
            .trimmed();
    commission.m_MaxCharge =
        maxChargeValue.isEmpty() ? CCommissions::MaxChargeValue : maxChargeValue.toDouble();

    QString round = aSettings.get<QString>("<xmlattr>.round", "bank").toLower();
    if (round == "high") {
        commission.m_Round = High;
    } else if (round == "low") {
        commission.m_Round = Low;
    } else {
        commission.m_Round = Bank;
    }

    commission.m_Base =
        (aSettings.get<QString>("<xmlattr>.base", "amount_all").toLower() == "amount") ? Amount
                                                                                       : AmountAll;

    return commission;
}

//----------------------------------------------------------------------------
Commission Commission::from_Variant(const QVariant &aCommissions) {
    Commission commission;

    auto val = [&aCommissions](QString aName, QString aDefault) -> QVariant {
        QVariant v = aCommissions.toMap().value(aName);
        return v.isNull() ? aDefault : v;
    };

    commission.m_Type =
        val("type", QString()).toString().toLower() == "absolute" ? Absolute : Percent;
    commission.m_Value = val("amount", QString()).toDouble();

    QString minChargeValue =
        val("min_charge", QString::number(CCommissions::MinChargeValue)).toString().trimmed();
    commission.m_MinCharge =
        minChargeValue.isEmpty() ? CCommissions::MinChargeValue : minChargeValue.toDouble();

    QString maxChargeValue =
        val("max_charge", QString::number(CCommissions::MaxChargeValue)).toString().trimmed();
    commission.m_MaxCharge =
        maxChargeValue.isEmpty() ? CCommissions::MaxChargeValue : maxChargeValue.toDouble();

    QString round = val("round", "bank").toString().toLower();
    if (round == "high") {
        commission.m_Round = High;
    } else if (round == "low") {
        commission.m_Round = Low;
    } else {
        commission.m_Round = Bank;
    }

    commission.m_Base =
        (val("base", "amount_all").toString().toLower() == "amount") ? Amount : AmountAll;

    return commission;
}

//----------------------------------------------------------------------------
CommissionList::CommissionList() = default;

//----------------------------------------------------------------------------
TCommissions CommissionList::getCommissions() const {
    return m_Commissions;
}

//----------------------------------------------------------------------------
bool CommissionList::query(double aSum, Commission &aCommission) const {
    bool response = true;

    foreach (const Commission &commission, m_Commissions) {
        if (commission.contains(aSum)) {
            if (response) {
                aCommission = commission;

                response = false;
            } else {
                if (commission > aCommission) {
                    aCommission = commission;
                }
            }
        }
    }

    return !response;
}

//----------------------------------------------------------------------------
CommissionList &CommissionList::operator<<(const Commission &aCommission) {
    m_Commissions << aCommission;

    return *this;
}

//----------------------------------------------------------------------------
CommissionList CommissionList::from_Settings(const TPtree &aSettings) noexcept(false) {
    CommissionList list;

    std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds =
        aSettings.equal_range("amount");
    for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it) {
        Commission commission(Commission::from_Settings(it->second.get_child("commission")));

        commission.m_Above =
            it->second.get<double>("<xmlattr>.above", CCommissions::DefaultAboveValue);
        commission.m_Below =
            it->second.get<double>("<xmlattr>.below", CCommissions::DefaultBelowValue);

        list.m_Commissions << commission;
    }

    searchBounds = aSettings.equal_range("commission");
    for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it) {
        list.m_Commissions << Commission::from_Settings(it->second);
    }

    return list;
}

//----------------------------------------------------------------------------
CommissionList CommissionList::from_Variant(const QVariant &aCommissions) {
    CommissionList list;
    QVariant amount = aCommissions.toMap().value("amount").toMap();
    Commission commission(Commission::from_Variant(amount.toMap().value("commission")));

    commission.m_Above = amount.toMap().value("above", CCommissions::DefaultAboveValue).toDouble();
    commission.m_Below = amount.toMap().value("below", CCommissions::DefaultBelowValue).toDouble();

    list.m_Commissions << commission;

    return list;
}

//----------------------------------------------------------------------------
CommissionByTimeList::CommissionByTimeList() = default;

//----------------------------------------------------------------------------
TCommissions CommissionByTimeList::getCommissions() const {
    QTime currentTime = QTime::currentTime();

    if ((currentTime >= m_Begin) && (currentTime <= m_End)) {
        return m_Commissions.getCommissions();
    }
    return {};
}

//----------------------------------------------------------------------------
bool CommissionByTimeList::query(double aSum,
                                 SDK::PaymentProcessor::Commission &aCommission) const {
    QTime time(QTime::currentTime());

    if ((m_Begin <= time) && (time <= m_End)) {
        return m_Commissions.query(aSum, aCommission);
    }

    return false;
}

//----------------------------------------------------------------------------
CommissionByTimeList &CommissionByTimeList::operator<<(const Commission &aCommission) {
    m_Commissions << aCommission;

    return *this;
}

//----------------------------------------------------------------------------
CommissionByTimeList CommissionByTimeList::from_Settings(const TPtree &aSettings) {
    CommissionByTimeList list;

    list.m_Begin = QTime::fromString(aSettings.get<QString>("<xmlattr>.begin"), "hh:mm:ss");
    list.m_End = QTime::fromString(aSettings.get<QString>("<xmlattr>.end"), "hh:mm:ss");
    list.m_Commissions = CommissionList::from_Settings(aSettings);

    return list;
}

//----------------------------------------------------------------------------
CommissionByTimeList CommissionByTimeList::from_Variant(const QVariant &aCommissions) {
    Q_UNUSED(aCommissions);

    // TODO-TODO
    CommissionByTimeList list;
    return list;
}

//----------------------------------------------------------------------------
CommissionByDayList::CommissionByDayList() = default;

//----------------------------------------------------------------------------
TCommissions CommissionByDayList::getCommissions() const {
    TCommissions result;

    if (m_Days.contains(static_cast<Commission::Day>(QDate::currentDate().dayOfWeek()))) {
        foreach (const CommissionByTimeList &commissionByTime, m_CommissionsByTime) {
            result = commissionByTime.getCommissions();

            if (!result.isEmpty()) {
                break;
            }
        }

        if (result.isEmpty()) {
            result = m_Commissions.getCommissions();
        }
    }

    return result;
}

//----------------------------------------------------------------------------
bool CommissionByDayList::query(double aSum, SDK::PaymentProcessor::Commission &aCommission) const {
    if (m_Days.contains(static_cast<Commission::Day>(QDate::currentDate().dayOfWeek()))) {
        foreach (const CommissionByTimeList &commissionByTime, m_CommissionsByTime) {
            if (commissionByTime.query(aSum, aCommission)) {
                return true;
            }
        }

        return m_Commissions.query(aSum, aCommission);
    }

    return false;
}

//----------------------------------------------------------------------------
CommissionByDayList CommissionByDayList::from_Settings(const TPtree &aSettings) {
    CommissionByDayList list;

    QStringList days = aSettings.get<QString>("<xmlattr>.id").split(",", Qt::SkipEmptyParts);
    for (QString &day : days) {
        day = day.trimmed();
    }

    foreach (const QString &day, days) {
        int temp = day.toInt();

        if ((temp >= Commission::Mon) && (temp <= Commission::Sun)) {
            list.m_Days << static_cast<Commission::Day>(temp);
        }
    }

    std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds =
        aSettings.equal_range("time");
    for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it) {
        list.m_CommissionsByTime << CommissionByTimeList::from_Settings(it->second);
    }

    list.m_Commissions = CommissionList::from_Settings(aSettings);

    return list;
}

//----------------------------------------------------------------------------
CommissionByDayList CommissionByDayList::from_Variant(const QVariant &aCommissions) {
    CommissionByDayList list;

    QStringList days = aCommissions.toMap().value("day").toStringList();

    foreach (const QString &day, days) {
        int temp = day.toInt();

        if ((temp >= Commission::Mon) && (temp <= Commission::Sun)) {
            list.m_Days << static_cast<Commission::Day>(temp);
        }
    }

    list.m_Commissions = CommissionList::from_Variant(aCommissions);

    return list;
}

//----------------------------------------------------------------------------
ProcessingCommission::ProcessingCommission() : m_Value(0), m_MinValue(0), m_Type(Real) {}

//----------------------------------------------------------------------------
ProcessingCommission ProcessingCommission::from_Settings(const TPtree &aSettings) {
    ProcessingCommission result;

    auto settings = aSettings.get_child_optional("cyberaddcomission");

    if (settings.is_initialized()) {
        result.m_Value = settings->get<double>("<xmlattr>.percent");
        result.m_MinValue = settings->get<double>("<xmlattr>.min_value");
        result.m_Type = static_cast<Type>(settings->get<int>("<xmlattr>.type"));
    }

    return result;
}

//----------------------------------------------------------------------------
ProcessingCommission ProcessingCommission::from_Variant(const QVariant &aCommissions) {
    ProcessingCommission result;

    auto settings = aCommissions.toMap().value("cyberaddcomission");

    if (settings.isValid()) {
        result.m_Value = settings.toMap().value("percent").toDouble();
        result.m_MinValue = settings.toMap().value("min_value").toDouble();
        result.m_Type = static_cast<Type>(settings.toMap().value("type").toInt());
    }

    return result;
}

//----------------------------------------------------------------------------
double ProcessingCommission::getValue(double aAmount, double aAmountAll) {
    double result = 0.F;

    switch (m_Type) {
    case Real:
        result = aAmount * m_Value / 100.0;
        break;
    case Diff:
        result = aAmountAll < aAmount ? 0.0 : (aAmountAll - aAmount) * m_Value / 100.0;
        break;
    case Inverse:
        result = aAmount * m_Value / (qFuzzyIsNull(100.0 - m_Value) ? 1.0 : (100.0 - m_Value));
        break;
    }

    return qRound((result < m_MinValue ? m_MinValue : result) * 100.0) / 100.0;
}

//----------------------------------------------------------------------------
bool ProcessingCommission::isNull() const {
    return qFuzzyIsNull(m_Value) && qFuzzyIsNull(m_MinValue);
}

//----------------------------------------------------------------------------
Commissions::Commissions() : m_IsValid(false) {}

//----------------------------------------------------------------------------
bool Commissions::SComplexCommissions::sortByMinLimit(const Commission &aFirst,
                                                      const Commission &aSecond) {
    return aFirst.getMinLimit() < aSecond.getMinLimit();
}

//----------------------------------------------------------------------------
TCommissions Commissions::SComplexCommissions::getCommissions() const {
    TCommissions result;

    foreach (const CommissionByDayList &commissionByDay, commissionsByDay) {
        result << commissionByDay.getCommissions();
    }

    if (result.isEmpty()) {
        foreach (const CommissionByTimeList &commissionByTime, commissionsByTime) {
            result << commissionByTime.getCommissions();
        }

        if (result.isEmpty()) {
            result = commissions.getCommissions();
        }
    }

    std::sort(result.begin(), result.end(), &Commissions::SComplexCommissions::sortByMinLimit);

    return result;
}

//----------------------------------------------------------------------------
Commission Commissions::SComplexCommissions::query(double aSum) const {
    Commission result;

    foreach (const CommissionByDayList &commissionByDay, commissionsByDay) {
        if (commissionByDay.query(aSum, result)) {
            return result;
        }
    }

    foreach (const CommissionByTimeList &commissionByTime, commissionsByTime) {
        if (commissionByTime.query(aSum, result)) {
            return result;
        }
    }

    commissions.query(aSum, result);

    return result;
}

//----------------------------------------------------------------------------
TCommissions Commissions::getCommissions(qint64 aProvider) const {
    if (m_ProviderCommissions.contains(aProvider)) {
        return m_ProviderCommissions[aProvider].getCommissions();
    }

    return m_DefaultCommissions.getCommissions();
}

//----------------------------------------------------------------------------
Commission Commissions::getCommission(qint64 aProvider, double aSum) const {
    if (m_ProviderCommissions.contains(aProvider)) {
        return m_ProviderCommissions[aProvider].query(aSum);
    }

    return m_DefaultCommissions.query(aSum);
}

//----------------------------------------------------------------------------
ProcessingCommission Commissions::getProcessingCommission(qint64 aProvider) {
    return m_ProcessingCommissions.contains(aProvider) ? m_ProcessingCommissions[aProvider]
                                                       : ProcessingCommission();
}

//----------------------------------------------------------------------------
int Commissions::getVAT(qint64 aProvider) {
    if (m_ProviderCommissions.contains(aProvider)) {
        return m_ProviderCommissions[aProvider].vat;
    }

    return 0;
}

//----------------------------------------------------------------------------
bool Commissions::isValid() const {
    return m_IsValid;
}

//----------------------------------------------------------------------------
bool Commissions::contains(qint64 aProvider, bool aCheckProcessing) {
    return aCheckProcessing ? m_ProcessingCommissions.contains(aProvider)
                            : m_ProviderCommissions.contains(aProvider);
}

//----------------------------------------------------------------------------
Commissions Commissions::from_Settings(const TPtree &aSettings) {
    Commissions result;

    std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds =
        aSettings.equal_range("operator");
    for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it) {
        try {
            result.m_ProviderCommissions.insert(it->second.get<qint64>("<xmlattr>.id"),
                                                result.loadCommissions(it->second));

            ProcessingCommission processingCommission =
                ProcessingCommission::from_Settings(it->second);
            if (!processingCommission.isNull()) {
                result.m_ProcessingCommissions.insert(it->second.get<qint64>("<xmlattr>.id"),
                                                      processingCommission);
            }
        } catch (std::runtime_error &) {
        }
    }

    result.m_DefaultCommissions = result.loadCommissions(aSettings);
    result.m_IsValid = true;

    return result;
}

//----------------------------------------------------------------------------
SDK::PaymentProcessor::Commissions Commissions::from_Variant(const QVariantList &aCommissions) {
    Commissions result;

    foreach (QVariant com, aCommissions) {
        result.m_ProviderCommissions.insert(com.toMap()["provider"].toInt(),
                                            result.loadCommissions(com));

        ProcessingCommission processingCommission = ProcessingCommission::from_Variant(com);

        if (!processingCommission.isNull()) {
            result.m_ProcessingCommissions.insert(com.toMap()["provider"].toInt(),
                                                  processingCommission);
        }
    }

    result.m_DefaultCommissions = Commissions::SComplexCommissions();
    result.m_IsValid = true;

    return result;
}

//----------------------------------------------------------------------------
Commissions::SComplexCommissions Commissions::loadCommissions(const TPtree &aBranch) {
    SComplexCommissions result;

    result.vat = aBranch.get<int>("vat", 0);

    std::pair<TPtree::const_assoc_iterator, TPtree::const_assoc_iterator> searchBounds =
        aBranch.equal_range("day");
    for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it) {
        result.commissionsByDay << CommissionByDayList::from_Settings(it->second);
    }

    searchBounds = aBranch.equal_range("time");
    for (TPtree::const_assoc_iterator it = searchBounds.first; it != searchBounds.second; ++it) {
        result.commissionsByTime << CommissionByTimeList::from_Settings(it->second);
    }

    result.commissions = CommissionList::from_Settings(aBranch);

    return result;
}

//----------------------------------------------------------------------------
Commissions::SComplexCommissions Commissions::loadCommissions(const QVariant &aCommissions) {
    SComplexCommissions result;

    result.vat = aCommissions.toMap().value("vat", 0).toDouble();

    foreach (QVariant com, aCommissions.toMap().value("commissions").toList()) {
        result.commissionsByDay << CommissionByDayList::from_Variant(com);
    }

    return result;
}

//----------------------------------------------------------------------------
void Commissions::appendFrom_Settings(const TPtree &aSettings) {
    auto localCommissions = from_Settings(aSettings);

    // Комиссии провайдера.
    {
        QMapIterator<qint64, SComplexCommissions> i(localCommissions.m_ProviderCommissions);
        while (i.hasNext()) {
            i.next();

            if (!this->m_ProviderCommissions.contains(i.key())) {
                this->m_ProviderCommissions.insert(i.key(), i.value());
            }
        }
    }

    // Комиссии процессинга.
    {
        QMapIterator<qint64, ProcessingCommission> i(localCommissions.m_ProcessingCommissions);
        while (i.hasNext()) {
            i.next();

            if (!this->m_ProcessingCommissions.contains(i.key())) {
                this->m_ProcessingCommissions.insert(i.key(), i.value());
            }
        }
    }
}

//----------------------------------------------------------------------------
void Commissions::clear() {
    m_IsValid = false;

    m_ProviderCommissions.clear();
    m_ProcessingCommissions.clear();
}

//----------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK

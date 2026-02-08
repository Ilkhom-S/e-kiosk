/* @file Описатель фискальных реквизитов. */

#include <QtCore/QCoreApplication>
#include <QtCore/QDate>

#include <SDK/Drivers/FR/FiscalFields.h>

#include <Hardware/FR/FiscalFieldDescriptions.h>

namespace CFR {
namespace FiscalFields {

//---------------------------------------------------------------------------
Data::Data() {
    TAllData allData = process(0);
    m_Buffer = allData.first;
    m_DescriptionData = allData.second;

    for (auto it = m_Buffer.begin(); it != m_Buffer.end(); ++it) {
        it->translationPF =
            QCoreApplication::translate("FiscalFields", it->translationPF.toLatin1());
    }

    foreach (int field, m_Buffer.keys()) {
        checkRFVAT20(field);
    }
}

//---------------------------------------------------------------------------
void Data::add(const TData &aData) {
    m_Buffer.unite(aData);

    for (auto it = aData.begin(); it != aData.end(); ++it) {
        m_DescriptionData.insert(it.key(), it->textKey);
    }
}

//---------------------------------------------------------------------------
TAllData Data::process(int aField, const SData &aData) {
    static TData data;
    static TDescriptionData descriptionData;

    if (!aField) {
        return TAllData(data, descriptionData);
    }

    data.insert(aField, aData);
    descriptionData.insert(aField, aData.textKey);

    return TAllData();
}

//---------------------------------------------------------------------------
int Data::getKey(const QString &aTextKey) const {
    return m_DescriptionData.values().contains(aTextKey) ? m_DescriptionData.key(aTextKey) : 0;
}

//---------------------------------------------------------------------------
TFields Data::getKeys(const QStringList &aTextKeys) const {
    TFields result;

    foreach (const QString &textKey, aTextKeys) {
        result << getKey(textKey);
    }

    return result;
}

//---------------------------------------------------------------------------
QStringList Data::getTextKeys() const {
    return m_DescriptionData.values();
}

//---------------------------------------------------------------------------
QStringList Data::getTextKeys(const TFields &aFields) const {
    QStringList result;

    foreach (int field, aFields) {
        if (m_DescriptionData.contains(field)) {
            result << m_DescriptionData[field];
        }
    }

    return result;
}

//---------------------------------------------------------------------------
QString Data::getTextLog(int aField) const {
    return QString("fiscal field %1 (%2)").arg(aField).arg(value(aField).textKey.replace("_", " "));
}

//---------------------------------------------------------------------------
QString Data::getLogFromList(const TFields &aFields) const {
    QStringList data;

    foreach (int field, aFields) {
        data << getTextLog(field);
    }

    return data.join(", ").replace("fiscal field ", "");
}

//---------------------------------------------------------------------------
void Data::checkRFVAT20(int aField) {
    if (isRFVAT20() && TaxAmountFields().contains(aField)) {
        m_Buffer[aField].translationPF.replace("18", "20");
        m_DescriptionData[aField].replace("18", "20");
    }
}

} // namespace FiscalFields
} // namespace CFR

//---------------------------------------------------------------------------

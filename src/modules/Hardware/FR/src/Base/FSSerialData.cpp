/* @file Данные ФН. */

#include <Hardware/FR/FSSerialData.h>

namespace FS {
CData::CData() {
    add("87100001", 13, EFFD::F10, "ООО \"РИК\"");
    add("87110001", 36, EFFD::F10, "ООО \"РИК\"");
    add("8712000100", 13, EFFD::F10, "ООО \"Прагматик\"", "3 верс.2");
    add("8712000101", 13, EFFD::F10, "ООО \"НТЦ \"Измеритель\"", "3 верс.1");
    add("87150001", 13, EFFD::F10, "ООО \"Прагматик\"", "Пр13-2");
    add("87160001", 13, EFFD::F10, "ООО \"НТЦ \"Измеритель\"", "Из13-2");
    add("92810001", 36, EFFD::F11, "ООО \"Инвента\"", "4");
    add("92820001", 15, EFFD::F11, "ООО \"Инвента\"", "5-15-2");
    add("92834403", 36, EFFD::F11, "ООО \"Эвотор\"", "ЭВ36-2");
    add("92840001", 15, EFFD::F11, "ООО \"Эвотор\"", "ЭВ15-2");
    add("92850001", 15, EFFD::F11, "ООО \"НТЦ \"Измеритель\"", "Из15-2");
    add("92860001", 15, EFFD::F11, "АО \"Концерн \"Автоматика\"", "3");
    add("92874403", 36, EFFD::F11, "ООО \"Инвента\"", "2");
    add("92880001", 15, EFFD::F11, "ООО \"Инвента\"", "5-15-1");
    add("92890001", 15, EFFD::F11, "ООО \"Прагматик\"", "Пр15-2");
    add("92514403", 15, EFFD::F11, "ООО \"Дримкас\"", "6-15-2");
    add("92524403", 36, EFFD::F11, "АО \"Концерн \"Автоматика\"", "Ав36-2");

    m_SerialNumberTemplates = m_Buffer.keys();
    auto pred = [](const QString &aData1, const QString &aData2) -> bool {
        return aData1.size() > aData2.size();
    };
    qSort(m_SerialNumberTemplates.begin(), m_SerialNumberTemplates.end(), pred);
}

//--------------------------------------------------------------------------------
bool CData::contains(const QString &aSerialNumber) const {
    return getSerialNumberTemplate(aSerialNumber) != m_SerialNumberTemplates.end();
}

//--------------------------------------------------------------------------------
void CData::add(const QString &aSerialNumber,
                int aExpiration,
                EFFD::Enum aFFD,
                const char *aProvider,
                const char *aRevision) {
    append(aSerialNumber,
           SData(aExpiration, aFFD, QString::from_Utf8(aProvider), QString::from_Utf8(aRevision)));
}

//--------------------------------------------------------------------------------
SData CData::value(const QString &aSerialNumber) const {
    QStringList::const_iterator serialNumberTemplate = getSerialNumberTemplate(aSerialNumber);

    return (serialNumberTemplate == m_SerialNumberTemplates.end()) ? SData()
                                                                  : m_Buffer[*serialNumberTemplate];
}

//--------------------------------------------------------------------------------
QStringList::const_iterator CData::getSerialNumberTemplate(const QString &aSerialNumber) const {
    QString serialNumber = aSerialNumber.simplified();

    return std::find_if(m_SerialNumberTemplates.begin(),
                        m_SerialNumberTemplates.end(),
                        [&serialNumber](const QString &aSerialNumberTemplate) -> bool {
                            return serialNumber.startsWith(aSerialNumberTemplate);
                        });
}

} // namespace FS

//--------------------------------------------------------------------------------

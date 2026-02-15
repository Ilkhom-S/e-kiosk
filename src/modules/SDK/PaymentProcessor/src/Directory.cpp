/* @file Справочники. */

#include "Directory.h"

#include <SDK/PaymentProcessor/Settings/Provider.h>

#include <algorithm>
#include <array>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>

namespace SDK {
namespace PaymentProcessor {

//---------------------------------------------------------------------------
namespace CDefaults {
const char DefaultDatabaseName[] = "data.db";
} // namespace CDefaults

//---------------------------------------------------------------------------
QString Directory::getAdapterName() {
    return CAdapterNames::Directory;
}

//---------------------------------------------------------------------------
Directory::Directory(TPtree &aProperties)
    : m_Properties(aProperties.get_child(CAdapterNames::Directory, aProperties)) {
    m_Ranges.reserve(20000);
    SRange range;

    static TPtree emptyNumCapacityTree;
    const auto &numCapacityTree = m_Properties.get_child("numcapacity", emptyNumCapacityTree);
    BOOST_FOREACH (const TPtree::value_type &record, numCapacityTree) {
        if (record.first == "<xmlattr>") {
            m_RangesTimestamp = QDateTime::fromString(record.second.get_value<QString>("stamp"));
        } else {
            range.cids.clear();
            range.ids.clear();

            try {
                BOOST_FOREACH (const TPtree::value_type &idTag, record.second) {
                    if (idTag.first == "<xmlattr>") {
                        range.from = idTag.second.get<qint64>("from");
                        range.to = idTag.second.get<qint64>("to");
                    } else {
                        if (idTag.first == "id") {
                            auto id = idTag.second.get_value<qint64>();

                            range.ids << id;
                            m_OverlappedIDs
                                << id; // Запоминаем ID операторов с виртуальными ёмкостями
                        } else if (idTag.first == "cid") {
                            range.cids << idTag.second.get_value<qint64>();
                        }
                    }
                }

                if (static_cast<int>(!range.ids.empty()) != 0) {
                    // Диапазон виртуальный, если есть хоть один ID
                    m_OverlappedRanges << range;
                } else if (static_cast<int>(!range.cids.empty()) != 0) {
                    m_Ranges << range;
                } else {
                    toLog(LogLevel::Error,
                          QString("Skipping broken range \"%1-%2\"").arg(range.from).arg(range.to));
                }
            } catch (std::exception &e) {
                toLog(LogLevel::Error, QString("Skipping broken range: %1.").arg(e.what()));
            }
        }
    }

    m_OverlappedIDs.squeeze();
    m_Ranges.squeeze();
    m_OverlappedRanges.squeeze();
    std::sort(m_Ranges.begin(), m_Ranges.end());
    std::sort(m_OverlappedRanges.begin(), m_OverlappedRanges.end());
}

//---------------------------------------------------------------------------
Directory::~Directory() = default;

//---------------------------------------------------------------------------
QList<SConnectionTemplate> Directory::getConnectionTemplates() const {
    QList<SConnectionTemplate> templates;

    try {
        static TPtree emptyConnectionsTree;
        const auto &connectionsTree =
            m_Properties.get_child("directory.connections", emptyConnectionsTree);
        BOOST_FOREACH (const TPtree::value_type &record, connectionsTree) {
            try {
                SConnectionTemplate connection;

                connection.name = record.second.get<QString>("<xmlattr>.name");
                connection.phone = record.second.get<QString>("<xmlattr>.phone");
                connection.login = record.second.get<QString>("<xmlattr>.login");
                connection.password = record.second.get<QString>("<xmlattr>.password");
                connection.initString = record.second.get<QString>("<xmlattr>.init_string");
                connection.balanceNumber = record.second.get("balance.<xmlattr>.number", QString());
                connection.regExp = record.second.get("balance.<xmlattr>.regexp", QString());

                templates.append(connection);
            } catch (std::exception &e) {
                toLog(LogLevel::Error,
                      QString("Skipping broken connection template: %1.").arg(e.what()));
            }
        }
    } catch (std::exception &e) {
        toLog(LogLevel::Error, QString("Failed to read connection templates: %1.").arg(e.what()));
    }

    return templates;
}

//---------------------------------------------------------------------------
QList<SRange> Directory::getRangesForNumber(qint64 aNumber) const {
    QList<SRange> ranges;
    ranges.reserve(5);

    // Сначала ищем в виртуальных диапазонах
    QVector<SRange>::const_iterator begin =
        std::lower_bound(m_OverlappedRanges.begin(), m_OverlappedRanges.end(), aNumber);
    QVector<SRange>::const_iterator end =
        std::upper_bound(m_OverlappedRanges.begin(), m_OverlappedRanges.end(), aNumber);

    // Если нет в виртуальных, то ищем в обычных
    if (begin == end) {
        begin = std::lower_bound(m_Ranges.begin(), m_Ranges.end(), aNumber);
        end = std::upper_bound(m_Ranges.begin(), m_Ranges.end(), aNumber);
    }

    std::copy(begin, end, std::back_inserter(ranges));

    return ranges;
}

//---------------------------------------------------------------------------
QSet<qint64> Directory::getOverlappedIDs() const {
    return m_OverlappedIDs;
}

//---------------------------------------------------------------------------
bool Directory::isValid() const {
    return true;
}

//---------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK

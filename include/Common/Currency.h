/* @file Класс описывающий номинал валюты. */

#pragma once

#include <QtCore/QHash>
#include <QtCore/qmath.h>

namespace Currency {

//--------------------------------------------------------------------------------
/// Класс для описания номинала купюры.
class Nominal {
public:
    typedef int RawType;

    explicit Nominal(int aValue) : m_Nominal(aValue * 100) {}
    explicit Nominal(double aValue) : m_Nominal(qFloor((aValue * 1000 + 0.001) / 10.0)) {}

    static Nominal from_RawValue(RawType aRawValue) {
        Nominal n(0);
        n.m_Nominal = aRawValue;
        return n;
    }

    operator int() const { return m_Nominal / 100; }
    operator double() const { return toDouble(); }

    RawType rawValue() const { return m_Nominal; }
    double toDouble() const { return m_Nominal / 100.; }

    bool operator==(const Nominal &aNominal) const { return this->m_Nominal == aNominal.m_Nominal; }
    bool operator<(const Nominal &aNominal) const { return this->m_Nominal < aNominal.m_Nominal; }
    bool operator>=(const Nominal &aNominal) const { return this->m_Nominal >= aNominal.m_Nominal; }

    const Nominal &operator=(int aNominal) {
        m_Nominal = aNominal * 100;
        return *this;
    }
    const Nominal &operator=(double aNominal) {
        m_Nominal = qFloor(aNominal * 100);
        return *this;
    }

    QString toString(bool aTrim_ZeroFraction = true) const {
        return (m_Nominal % 100) == 0 && aTrim_ZeroFraction
                   ? QString::number(m_Nominal / 100)
                   : QString::number(m_Nominal / 100., 'f', 2);
    }

private:
    RawType m_Nominal;
};

} // end namespace Currency

//--------------------------------------------------------------------------------
inline uint qHash(const Currency::Nominal &aValue) {
    return qHash(aValue.rawValue());
}

//--------------------------------------------------------------------------------
namespace std {
template <> struct hash<Currency::Nominal> {
    size_t operator()(const Currency::Nominal &aValue) const noexcept {
        return std::hash<Currency::Nominal::RawType>()(aValue.rawValue());
    }
};
} // namespace std

//--------------------------------------------------------------------------------

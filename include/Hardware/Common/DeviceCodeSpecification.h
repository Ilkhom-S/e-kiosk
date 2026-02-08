/* @file Описатели состояний валидаторов */

#pragma once

#include <QtCore/QByteArray>

#include "Hardware/Common/BaseStatusTypes.h"
#include "Hardware/Common/DeviceCodeSpecificationBase.h"

//--------------------------------------------------------------------------------
/// Буфер описателей кодов устройств.
typedef QMap<QString, SDeviceCodeSpecification> TDeviceCodeSpecifications;

class IDeviceCodeSpecification {
public:
    virtual void getSpecification(const QByteArray &aBuffer,
                                  TDeviceCodeSpecifications &aSpecifications) = 0;
    virtual void getSpecification(char aAnswerData, TStatusCodes &aStatusCodes) = 0;
};

/// Структура для описаний состояний купюроприемника.
class CommonDeviceCodeSpecification : public IDeviceCodeSpecification,
                                      protected DeviceCodeSpecificationBase<char> {
public:
    /// Получить спецификации девайс-кодов по байт-массиву. байт-массив не должен содержать лишних
    /// байтов перед статусными байтами.
    virtual void getSpecification(const QByteArray &aBuffer,
                                  TDeviceCodeSpecifications &aSpecifications) {
        SDeviceCodeSpecification specification =
            ((aBuffer.size() > 1) && m_ExtraCodeSpecification.data().contains(aBuffer[0]))
                ? m_ExtraCodeSpecification[aBuffer[0]][aBuffer[1]]
                : value(aBuffer[0]);
        aSpecifications.insert(aBuffer.toHex().data(), specification);
    }

    /// Получить спецификации девайс-кодов по 1 байту.
    void getSpecification(char aAnswerData, TStatusCodes &aStatusCodes) {
        TDeviceCodeSpecifications specification;
        getSpecification(QByteArray(1, aAnswerData), specification);

        aStatusCodes.insert(specification.values()[0].statusCode);
    }

    /// Добавить спецификацию девайс-кода.
    void addStatus(char aCode, int aStatusCode, const QString &aDescription = "") {
        if (m_ExtraCodeSpecification.data().isEmpty()) {
            appendStatus(aCode, aStatusCode, aDescription);
        } else {
            m_ExtraCodeSpecification.data()[m_ExtraCode].appendStatus(
                aCode, aStatusCode, aDescription);
        }
    }

    const SDeviceCodeSpecification operator[](char aKey) const {
        return DeviceCodeSpecificationBase<char>::operator[](aKey);
    }

protected:
    /// Начать ветку спецификации дополнительных девайс-кодов и установить для нее значение по
    /// умолчанию. Вызывать, когда все спецификации обычных девайс-кодов определены.
    void setExtraCodeDefault(char aCode, int aStatusCode, const QString &aDescription = "") {
        appendStatus(aCode, aStatusCode, aDescription);
        m_ExtraCodeSpecification.data()[aCode].setDefault(
            SDeviceCodeSpecification(aStatusCode, aDescription));
        m_ExtraCode = aCode;
    }

private:
    /// Спецификация дополнительных девайс-кодов.
    CSpecification<char, DeviceCodeSpecificationBase<char>> m_ExtraCodeSpecification;

    /// Текущий дополнительный девайс-код.
    char m_ExtraCode;
};

/// Структура для описаний состояний купюроприёмника.
class BitmapDeviceCodeSpecification : public IDeviceCodeSpecification,
                                      protected DeviceCodeSpecificationBase<int> {
public:
    /// Получить спецификации девайс-кодов по байт-массиву. байт-массив не должен содержать лишних
    /// байтов перед статусными байтами.
    virtual void getSpecification(const QByteArray &aBuffer,
                                  TDeviceCodeSpecifications &aSpecifications) {
        foreach (int shift, m_Buffer.keys()) {
            int byteNumber = shift / 8;

            if (byteNumber < aBuffer.size()) {
                int bitNumber = shift - byteNumber * 8;
                bool bit = (aBuffer[byteNumber] >> bitNumber) & 1;
                SDeviceCodeSpecification specification = m_Buffer[shift];
                int totalBit = byteNumber * 8 + bitNumber;

                if (m_Inverts.contains(totalBit) != bit) {
                    aSpecifications.insert(QString::number(shift), specification);
                }
            } else {
                aSpecifications.insert("",
                                       SDeviceCodeSpecification(DeviceStatusCode::Error::Unknown,
                                                                "answer size is too small"));
            }
        }
    }

    /// Получить спецификации девайс-кодов по байт-массиву. байт-массив не должен содержать лишних
    /// байтов перед статусными байтами.
    virtual void getSpecification(const QByteArray &aBuffer, TStatusCodes &aStatusCodes) {
        TDeviceCodeSpecifications specifications;
        getSpecification(aBuffer, specifications);

        foreach (auto specification, specifications) {
            aStatusCodes.insert(specification.statusCode);
        }
    }

    /// Получить спецификации девайс-кодов по байт-массиву. байт-массив не должен содержать лишних
    /// байтов перед статусными байтами.
    void getSpecification(char aAnswerData, TStatusCodes &aStatusCodes) {
        getSpecification(QByteArray(1, aAnswerData), aStatusCodes);
    }

protected:
    /// Добавить спецификацию девайс-кода.
    virtual void
    addStatus(int aBit, int aStatusCode, const QString &aDescription = "", bool aInvert = false) {
        appendStatus(aBit, aStatusCode, aDescription);

        if (aInvert) {
            m_Inverts.insert(aBit);
        }
    }

    /// Добавить спецификацию девайс-кода с указанием байта.
    virtual void addStatus(int aByte,
                           int aBit,
                           int aStatusCode,
                           const QString &aDescription = "",
                           bool aInvert = false) {
        addStatus(aByte * 8 + aBit, aStatusCode, aDescription, aInvert);
    }

private:
    /// Сет инвертированных битов.
    QSet<int> m_Inverts;
};

//--------------------------------------------------------------------------------

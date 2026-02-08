/* @file Кодек для Custom-ов с казахской локалью. */

#pragma once

#include <QtCore/QStringDecoder>

#include <Hardware/Common/CodecBase.h>

//---------------------------------------------------------------------------
class CustomKZTCodec : public CodecBase {
public:
    CustomKZTCodec() {
        m_Name = CHardware::Codepages::CustomKZT;
        m_MIB = 3003;

        auto encoding = QStringConverter::encodingForName("IBM866");
        QStringDecoder decoder(encoding.value_or(QStringConverter::Encoding::Latin1));

        // TODO
        if (decoder.isValid()) {
            for (uchar ch = uchar('\x80'); ch && (ch <= uchar('\xFF')); ++ch) {
                QString value = decoder.decode(QByteArray(1, ch));
                m_Data.add(ch, value);
            }

            m_Data.add('\xB0', "Ә");
            m_Data.add('\xB1', "ә");
            m_Data.add('\xB2', "Ғ");
            m_Data.add('\xB3', "ғ");
            m_Data.add('\xB4', "Қ");
            m_Data.add('\xB5', "қ");
            m_Data.add('\xB6', "Ң");
            m_Data.add('\xB7', "ң");
            m_Data.add('\xB8', "Ө");
            m_Data.add('\xB9', "ө");
            m_Data.add('\xBA', "Ұ");
            m_Data.add('\xBB', "ұ");
            m_Data.add('\xBC', "Ү");
            m_Data.add('\xBD', "ү");
            m_Data.add('\xBE', "Һ");
            m_Data.add('\xBF', "һ");
        }
    }
};

//---------------------------------------------------------------------------

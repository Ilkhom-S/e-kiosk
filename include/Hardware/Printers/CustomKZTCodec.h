/* @file Кодек для Custom-ов с казахской локалью. */

#pragma once

#include <Hardware/Common/CodecBase.h>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringDecoder>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
class CustomKZTCodec : public CodecBase {
  public:
    CustomKZTCodec() {
        mName = CHardware::Codepages::CustomKZT;
        mMIB = 3003;

        auto encoding = QStringConverter::encodingForName("IBM866");
        QStringDecoder decoder(encoding.value_or(QStringConverter::Encoding::Latin1));

        // TODO
        if (decoder.isValid()) {
            for (uchar ch = uchar('\x80'); ch && (ch <= uchar('\xFF')); ++ch) {
                QString value = decoder.decode(QByteArray(1, ch));
                mData.add(ch, value);
            }

            mData.add('\xB0', "Ә");
            mData.add('\xB1', "ә");
            mData.add('\xB2', "Ғ");
            mData.add('\xB3', "ғ");
            mData.add('\xB4', "Қ");
            mData.add('\xB5', "қ");
            mData.add('\xB6', "Ң");
            mData.add('\xB7', "ң");
            mData.add('\xB8', "Ө");
            mData.add('\xB9', "ө");
            mData.add('\xBA', "Ұ");
            mData.add('\xBB', "ұ");
            mData.add('\xBC', "Ү");
            mData.add('\xBD', "ү");
            mData.add('\xBE', "Һ");
            mData.add('\xBF', "һ");
        }
    }
};

//---------------------------------------------------------------------------

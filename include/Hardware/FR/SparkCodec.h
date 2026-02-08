/* @file Кодек СПАРК-а. */

#pragma once

#include <Hardware/Common/CodecBase.h>

//---------------------------------------------------------------------------
class SparkTextCodec : public CodecBase {
public:
    SparkTextCodec() {
        m_Name = CHardware::Codepages::SPARK;
        m_MIB = 3001;

        m_Data.add('\x80', "А");
        m_Data.add('\x90', "П");
        m_Data.add('\xA0', "Я");
        m_Data.add('\xB0', "а");
        m_Data.add('\x81', "Б");
        m_Data.add('\x91', "Р");
        m_Data.add('\xA1', "«");
        m_Data.add('\xB1', "б");
        m_Data.add('\x82', "В");
        m_Data.add('\x92', "С");
        m_Data.add('\xA2', "»");
        m_Data.add('\xB2', "в");
        m_Data.add('\x83', "Г");
        m_Data.add('\x93', "Т");
        m_Data.add('\xA3', "€");
        m_Data.add('\xB3', "г");
        m_Data.add('\x84', "Д");
        m_Data.add('\x94', "У");
        m_Data.add('\xA4', "€", false);
        m_Data.add('\xB4', "д");
        m_Data.add('\x85', "Е");
        m_Data.add('\x95', "Ф");
        m_Data.add('\xA5', "Σ");
        m_Data.add('\xB5', "е");
        m_Data.add('\x86', "Ё");
        m_Data.add('\x96', "Х");
        m_Data.add('\xA6', ".", false);
        m_Data.add('\xB6', "Ё");
        m_Data.add('\x87', "Ж");
        m_Data.add('\x97', "Ц");
        m_Data.add('\xA7', "º");
        m_Data.add('\xB7', "ж");
        m_Data.add('\x88', "З");
        m_Data.add('\x98', "Ч");
        m_Data.add('\xA8', "°");
        m_Data.add('\xB8', "з");
        m_Data.add('\x89', "И");
        m_Data.add('\x99', "Ш");
        m_Data.add('\xA9', "И", false);
        m_Data.add('\xB9', "и");
        m_Data.add('\x8A', "Й");
        m_Data.add('\x9A', "Щ");
        m_Data.add('\xAA', "Т", false);
        m_Data.add('\xBA', "й");
        m_Data.add('\x8B', "К");
        m_Data.add('\x9B', "Ъ");
        m_Data.add('\xAB', "О", false);
        m_Data.add('\xBB', "к");
        m_Data.add('\x8C', "Л");
        m_Data.add('\x9C', "Ы");
        m_Data.add('\xAC', "Г", false);
        m_Data.add('\xBC', "л");
        m_Data.add('\x8D', "М");
        m_Data.add('\x9D', "Ь");
        m_Data.add('\xAD', "▬");
        m_Data.add('\xBD', "м");
        m_Data.add('\x8E', "Н");
        m_Data.add('\x9E', "Э");
        m_Data.add('\xAE', "▬", false);
        m_Data.add('\xBE', "н");
        m_Data.add('\x8F', "О");
        m_Data.add('\x9F', "Ю");
        m_Data.add('\xAF');
        m_Data.add('\xBF', "о");

        m_Data.add('\xC0', "п");
        m_Data.add('\xD0', "я");
        m_Data.add('\xE0', "⁴");
        m_Data.add('\xF0', "=", false);
        m_Data.add('\xC1', "р");
        m_Data.add('\xD1');
        m_Data.add('\xE1', "⁵");
        m_Data.add('\xF1');
        m_Data.add('\xC2', "с");
        m_Data.add('\xD2', "0", false);
        m_Data.add('\xE2', "⁶");
        m_Data.add('\xF2');
        m_Data.add('\xC3', "т");
        m_Data.add('\xD3', "1", false);
        m_Data.add('\xE3', "⁷");
        m_Data.add('\xF3');
        m_Data.add('\xC4', "у");
        m_Data.add('\xD4', "2", false);
        m_Data.add('\xE4', "⁸");
        m_Data.add('\xF4');
        m_Data.add('\xC5', "ф");
        m_Data.add('\xD5', "3", false);
        m_Data.add('\xE5', "⁹");
        m_Data.add('\xF5');
        m_Data.add('\xC6', "х");
        m_Data.add('\xD6', "4", false);
        m_Data.add('\xE6', ":", false);
        m_Data.add('\xF6', "|");
        m_Data.add('\xC7', "ц");
        m_Data.add('\xD7', "5", false);
        m_Data.add('\xE7', "█");
        m_Data.add('\xF7');
        m_Data.add('\xC8', "ч");
        m_Data.add('\xD8', "6", false);
        m_Data.add('\xE8', "√");
        m_Data.add('\xF8');
        m_Data.add('\xC9', "ш");
        m_Data.add('\xD9', "7", false);
        m_Data.add('\xE9');
        m_Data.add('\xF9', "½");
        m_Data.add('\xCA', "щ");
        m_Data.add('\xDA', "8", false);
        m_Data.add('\xEA');
        m_Data.add('\xFA', "≡");
        m_Data.add('\xCB', "ъ");
        m_Data.add('\xDB', "9", false);
        m_Data.add('\xEB', "◙");
        m_Data.add('\xFB');
        m_Data.add('\xCC', "ы");
        m_Data.add('\xDC', "⁰");
        m_Data.add('\xEC');
        m_Data.add('\xFC');
        m_Data.add('\xCD', "ь");
        m_Data.add('\xDD', "¹");
        m_Data.add('\xED', "▬", false);
        m_Data.add('\xFD');
        m_Data.add('\xCE', "э");
        m_Data.add('\xDE', "²");
        m_Data.add('\xEE');
        m_Data.add('\xFE');
        m_Data.add('\xCF', "ю");
        m_Data.add('\xDF', "³");
        m_Data.add('\xEF', "×");
        m_Data.add('\xFF');
    }
};

//---------------------------------------------------------------------------

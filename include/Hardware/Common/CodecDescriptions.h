/* @file Описатель кодеков. */

#pragma once

#include "Hardware/FR/AtolCodec.h"
#include "Hardware/FR/SparkCodec.h"
#include "Hardware/Printers/CustomKZTCodec.h"

// Qt
#include <memory>

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QStringDecoder>
#include <Common/QtHeadersEnd.h>

//---------------------------------------------------------------------------
// Используем макрос для удобного добавления кодеков (совместимо с Qt 6)
#define APPEND_CODEC(aName, aCodec) append(CHardware::Codepages::aName, std::make_shared<QStringDecoder>(#aCodec))

// T2 теперь std::shared_ptr<QStringDecoder>, который можно копировать в QMap
class CodecDescriptions : public CSpecification<QString, std::shared_ptr<QStringDecoder>> {
  public:
    CodecDescriptions() {
        // Инициализация стандартных кодировок через новый API Qt 6
        APPEND_CODEC(CP850, "IBM 850");
        APPEND_CODEC(CP866, "IBM 866");
        APPEND_CODEC(Win1250, "windows-1250");
        APPEND_CODEC(Win1251, "windows-1251");
        APPEND_CODEC(Win1252, "windows-1252");

        // Для кастомных кодеков (Atol, Spark) вам нужно реализовать их
        // как наследников QStringConverter или продолжать использовать Core5Compat
    }
};

static CodecDescriptions CodecByName;

//---------------------------------------------------------------------------

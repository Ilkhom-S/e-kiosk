/* @file Данные моделей устройств на протоколе ID003. */

#pragma once

#include "Hardware/CashAcceptors/ModelData.h"

//--------------------------------------------------------------------------------
namespace CID003 {
/// Модели
namespace Models {
extern const char GPTAurora[];
extern const char JCMIPRO[];
extern const char JCMUBA[];
extern const char CashcodeMVU[];
extern const char JCMVega[];
} // namespace Models

/// Подтип протокола. Разделение чисто символическое, связано с лицензированием пользования
/// протокола ID003. По подтипу можно сделать вывод о фирме, которая его реализовала в девайсе.
namespace ProtocolData {
extern const char GPTAurora[];

/// Подтип протокола. Разделение чисто символическое, связано с лицензированием пользования
/// протоколом ID003. По подтипу можно сделать вывод о фирме, которая его реализовала в девайсе.
namespace Alias {
extern const char ID003[];
extern const char ID003Ext[];
extern const char BDP[];
extern const char OP003[];
} // namespace Alias

/// Лексема ответа на запрос идентификации.
extern const char Lexeme[];
const QString IdLexeme = QString("(%1*)").arg(Lexeme);

/// Класс для перебора регулярок для парсинга ответа на запрос идентификации.
class CIdentification : public CDescription<QString> {
public:
    CIdentification();
};
} // namespace ProtocolData

/// Количество данных о конструктиве в ответе на идентификацию для новых моделей JCM (IPRO и Vega)
const int NewJCMModelDataCount = 3;

//--------------------------------------------------------------------------------
class BaseModelData : public CSpecification<QString, SBaseModelData> {
public:
    void add(const QString &aId, const QString &aName, bool aVerified = false) {
        append(aId, SBaseModelData(aName, aVerified));
    }
};

//--------------------------------------------------------------------------------
struct SModelData : public SBaseModelData {
    QString type;
    BaseModelData models;

    SModelData() {}
    SModelData(const QString &aName, bool aVerified) : SBaseModelData(aName, aVerified) {}
    SModelData(const QString &aDefaultName, const QString &aType);
};

class ModelData : public CSpecification<char, SModelData> {
public:
    ModelData();
    SBaseModelData getData(char aCode, const QString &aNumber, const QString &aStackerType);

private:
    void add(char aId, const QString &aName, bool aVerified = false);
};
} // namespace CID003

//--------------------------------------------------------------------------------

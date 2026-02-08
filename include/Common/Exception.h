/* @file Базовый класс исключений для EKiosk. */

#pragma once

#include <QtCore/QString>

//--------------------------------------------------------------------------------
/// Категории исключений.
namespace ECategory {
enum Enum { System, Hardware, Protocol, Application, Network };
} // namespace ECategory

//--------------------------------------------------------------------------------
/// Уровни серьезности исключений.
namespace ESeverity {
enum Enum { Info, Warning, Minor, Major, Critical };
} // namespace ESeverity

//--------------------------------------------------------------------------------
/// Класс исключения.
class Exception {
public:
    Exception(ECategory::Enum aCategory,
              ESeverity::Enum aSeverity,
              int aCode,
              const QString &aMessage)
        : m_Category(aCategory), m_Severity(aSeverity), m_Code(aCode), m_Message(aMessage) {}

    ECategory::Enum getCategory() const { return m_Category; }
    ESeverity::Enum getSeverity() const { return m_Severity; }
    int getCode() const { return m_Code; }
    QString getMessage() const { return m_Message; }

private:
    ECategory::Enum m_Category;
    ESeverity::Enum m_Severity;
    int m_Code;
    QString m_Message;
};

//--------------------------------------------------------------------------------

/* @file Базовый класс исключений для EKiosk. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

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
    Exception(ECategory::Enum aCategory, ESeverity::Enum aSeverity, int aCode, const QString &aMessage)
        : mCategory(aCategory), mSeverity(aSeverity), mCode(aCode), mMessage(aMessage) {
    }

    ECategory::Enum getCategory() const {
        return mCategory;
    }
    ESeverity::Enum getSeverity() const {
        return mSeverity;
    }
    int getCode() const {
        return mCode;
    }
    QString getMessage() const {
        return mMessage;
    }

  private:
    ECategory::Enum mCategory;
    ESeverity::Enum mSeverity;
    int mCode;
    QString mMessage;
};

//--------------------------------------------------------------------------------

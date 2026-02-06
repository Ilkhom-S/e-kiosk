/* @file Описатель графического объекта. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>

namespace SDK {
namespace GUI {

//---------------------------------------------------------------------------
/// Описатель графического объекта. Загружается из файла description.ini в
/// каталоге объекта.
struct GraphicsItemInfo {
    QString name;      /// Уникальное название графического элемента.
    QString type;      /// Тип нужного графического движка.
    QString directory; /// Директория с контентом элемента.

    QMap<QString, QString> parameters; /// Специфические параметры для движка.
    QVariantMap context;               /// Специфические параметры для виджета.

    /// Оператор ==.
    inline bool operator==(const GraphicsItemInfo &aGraphicsItemInfo) {
        return name == aGraphicsItemInfo.name && type == aGraphicsItemInfo.type &&
               directory == aGraphicsItemInfo.directory &&
               parameters == aGraphicsItemInfo.parameters && context == aGraphicsItemInfo.context;
    }

    /// Оператор !=.
    inline bool operator!=(const GraphicsItemInfo &aGraphicsItemInfo) {
        return !(*this == aGraphicsItemInfo);
    }
};

} // namespace GUI
} // namespace SDK

//---------------------------------------------------------------------------

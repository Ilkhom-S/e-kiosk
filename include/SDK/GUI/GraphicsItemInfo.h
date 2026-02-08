/* @file Описатель графического объекта. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QVariant>

namespace SDK {
namespace GUI {

//---------------------------------------------------------------------------
/// Описатель графического объекта. Загружается из файла description.ini в
/// каталоге объекта.
struct GraphicsItem_Info {
    QString name;      /// Уникальное название графического элемента.
    QString type;      /// Тип нужного графического движка.
    QString directory; /// Директория с контентом элемента.

    QMap<QString, QString> parameters; /// Специфические параметры для движка.
    QVariantMap context;               /// Специфические параметры для виджета.

    /// Оператор ==.
    inline bool operator==(const GraphicsItem_Info &aGraphicsItem_Info) {
        return name == aGraphicsItem_Info.name && type == aGraphicsItem_Info.type &&
               directory == aGraphicsItem_Info.directory &&
               parameters == aGraphicsItem_Info.parameters && context == aGraphicsItem_Info.context;
    }

    /// Оператор !=.
    inline bool operator!=(const GraphicsItem_Info &aGraphicsItem_Info) {
        return !(*this == aGraphicsItem_Info);
    }
};

} // namespace GUI
} // namespace SDK

//---------------------------------------------------------------------------

/* @file Интерфейс графического движка. */

#pragma once

#include <QtCore/QString>

#include <SDK/GUI/GraphicsItemInfo.h>
#include <SDK/GUI/IGraphicsHost.h>
#include <SDK/GUI/IGraphicsItem.h>

class ILog;

namespace SDK {
namespace GUI {

//---------------------------------------------------------------------------
/// Интерфейс графического движка.
class IGraphicsEngine {
public:
    /// Возвращает указатель на владельца движка.
    virtual IGraphicsHost *getGraphicsHost() = 0;

    /// Возвращает лог.
    virtual ILog *getLog() const = 0;

protected:
    virtual ~IGraphicsEngine() {}
};

} // namespace GUI
} // namespace SDK

//---------------------------------------------------------------------------

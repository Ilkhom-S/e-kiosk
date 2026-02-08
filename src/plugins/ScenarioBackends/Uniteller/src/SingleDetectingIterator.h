/* @file Заглушка итератор поиска устройств в единственном числе. */

#pragma once

#include <SDK/Drivers/IDevice.h>

namespace SDK {
namespace Driver {

//--------------------------------------------------------------------------------

class SingleDetectingIterator : public IDevice::IDetectingIterator {
    int m_DetectNextIndex;

public:
    SingleDetectingIterator() { resetDetectingIterator(); }

    void resetDetectingIterator() { m_DetectNextIndex = 0; }

#pragma region IDetectingIterator interface

    /// Переход к следующим параметрам устройства.
    virtual bool moveNext() { return (m_DetectNextIndex++ == 0); }

    /// Поиск устройства на текущих параметрах.
    virtual bool find() { return true; }

#pragma endregion
};

} // namespace Driver
} // namespace SDK

//--------------------------------------------------------------------------------

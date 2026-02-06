/* @file Удаление указателя с помощью deleteLater. */

#pragma once

//--------------------------------------------------------------------------------
template <class T> class ScopedPointerLaterDeleter {
public:
    /// Очистка указателя с вызовом deleteLater.
    static inline void cleanup(T *aPointer) {
        if (aPointer) {
            aPointer->deleteLater();
        }
    }
};

//--------------------------------------------------------------------------------

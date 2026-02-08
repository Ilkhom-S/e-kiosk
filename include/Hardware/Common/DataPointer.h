/* @file Обертка указателя на произвольный тип. Структура нужна для инициализации в статической
 * функции. */

#pragma once

//--------------------------------------------------------------------------------
template <class T> struct SPData {
public:
    SPData<T>() : m_Data(0) {}

    operator T() { return m_Data; }

    SPData<T> &operator=(T aResult) {
        m_Data = aResult;

        return *this;
    }

    T *operator&() { return &m_Data; }

    operator bool() { return m_Data; }

private:
    T m_Data;
};

//--------------------------------------------------------------------------------

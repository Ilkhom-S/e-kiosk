/* @file Тип результата выполнения команд. Содержит int, структура нужна для перегрузки bool(). */

#pragma once

//--------------------------------------------------------------------------------
struct TResult {
public:
    TResult() : m_Data(0) {}
    TResult(const int &aResult) { *this = aResult; }

    TResult(bool aResult) { *this = aResult; }

    TResult &operator=(const int &aResult) {
        m_Data = aResult;

        return *this;
    }

    TResult &operator=(bool aResult) {
        m_Data = aResult ? 0 : -1;

        return *this;
    }

    bool operator==(const int &aResult) { return m_Data == aResult; }

    bool operator!=(const int &aResult) { return !operator==(aResult); }

    operator int() { return m_Data; }

    operator bool() { return !m_Data; }

private:
    int m_Data;
};

//--------------------------------------------------------------------------------

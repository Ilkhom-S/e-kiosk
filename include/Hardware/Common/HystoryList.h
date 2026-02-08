/* @file Лист истории изменений элементов. */

#pragma once

#include <QtCore/QList>
#include <QtCore/QtGlobal>

//--------------------------------------------------------------------------------
template <class T> class HistoryList : public QList<T> {
public:
    using QList<T>::size;
    using QList<T>::value;
    using QList<T>::removeFirst;
    using QList<T>::isEmpty;

    HistoryList() : m_Size(0), m_Level(0) {}

    //--------------------------------------------------------------------------------
    /// Безопасно получить последний элемент или его заменяющий дефолтный. aLevel >= 1.
    T lastValue(int aLevel = 1) const {
        int index = qMin(size() - aLevel, size() - 1);

        return value(index);
    }

    //--------------------------------------------------------------------------------
    /// Добавить элемент с контролем размера листа.
    void append(const T &aItem) {
        QList<T>::append(aItem);

        while (m_Size && (size() > m_Size)) {
            removeFirst();

            if (m_Level && (m_Level < m_Size)) {
                m_Level--;
            }
        }
    }

    //--------------------------------------------------------------------------------
    /// Установить размер листа.
    void setSize(int aSize) { m_Size = aSize; }

    //--------------------------------------------------------------------------------
    /// Получить уровень обработанных элементов листа.
    int getLevel() { return m_Level; }

    //--------------------------------------------------------------------------------
    /// Обновить уровень обработанных элементов листа.
    void updateLevel(bool aCorrectOnly = false) {
        if (!aCorrectOnly || (m_Level > size())) {
            m_Level = size();
        }
    }

    //--------------------------------------------------------------------------------
    /// Сделать последний элемент необработанным.
    void checkLastUnprocessed() {
        if (!isEmpty() && (m_Level >= size())) {
            m_Level = size() - 1;
        }
    }

    //--------------------------------------------------------------------------------
    /// Запомнить состояние как необработанное.
    void saveLevel() {
        if ((m_Level == m_Size) && m_Size) {
            m_Level--;
        }
    }

private:
    /// Размер листа.
    int m_Size;

    /// Уровень обработанных элементов листа.
    int m_Level;
};

//---------------------------------------------------------------------------

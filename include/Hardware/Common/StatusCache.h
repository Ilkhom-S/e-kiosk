/* @file Кэш состояний устройства. */

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <Common/QtHeadersEnd.h>

//--------------------------------------------------------------------------------
typedef QSet<int> TStatusCodes;

template <class T> class StatusCache : public QMap<T, TStatusCodes> {
  public:
    using QMap<T, TStatusCodes>::contains;
    using QMap<T, TStatusCodes>::isEmpty;
    using QMap<T, TStatusCodes>::size;

    const StatusCache<T> operator-(const StatusCache<T> &aStatusCache) {
        StatusCache<T> result(*this);

        foreach (T key, aStatusCache.keys()) {
            if (result.contains(key)) {
                result[key] -= aStatusCache[key];
            }
        }

        return result;
    }

    int size(T key) const {
        return contains(key) ? value(key).size() : 0;
    }

    int isEmpty(T key) const {
        return !this->contains(key) || this->value(key).isEmpty();
    }

    bool contains(int aStatusCode) const {
        for (auto it = this->begin(); it != this->end(); ++it) {
            if (it.value().contains(aStatusCode)) {
                return true;
            }
        }

        return false;
    }
};

//--------------------------------------------------------------------------------

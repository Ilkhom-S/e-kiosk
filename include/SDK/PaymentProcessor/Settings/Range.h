#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVector>
#include <Common/QtHeadersEnd.h>

namespace SDK {
    namespace PaymentProcessor {

        //----------------------------------------------------------------------------
        /// Range of ids/capacity.
        struct SRange {
            qint64 from;
            qint64 to;

            QVector<qint64> cids;
            QVector<qint64> ids;

            bool operator<(const SRange &aRange) const;
        };

        //---------------------------------------------------------------------------
        bool operator<(const SRange &aRange, qint64 aNumber);
        bool operator<(qint64 aNumber, const SRange &aRange);

    } // namespace PaymentProcessor
} // namespace SDK


/* @file Реализация запроса на проверку номера. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QRegularExpression>
#include <Common/QtHeadersEnd.h>

// Project
#include "Payment.h"
#include "PaymentCheckRequest.h"

namespace CPayment {
    const char DefaultMinLimit[] = "200";
} // namespace CPayment

//---------------------------------------------------------------------------
PaymentCheckRequest::PaymentCheckRequest(Payment *aPayment, bool aFake)
    : PaymentRequest(aPayment, CPayment::Requests::Check) {
    addProviderParameters(CPayment::Requests::Check);

    if (aFake) {
        addParameter("REQ_TYPE", 1);

        QString limit = mPayment->getProviderSettings().limits.check.isEmpty()
                            ? mPayment->getProviderSettings().limits.min
                            : mPayment->getProviderSettings().limits.check;

        // Сначала смотрим на сумму для проверку номера для оператора.
        bool convertOk(false);
        limit.toDouble(&convertOk);

        if (!convertOk) {
            // Если получается определить минимальный лимит оператора, используем его.
            // Иначе берём минимульную сумму CPayment::DefaultMinLimit.
            QRegularExpression macroPattern("\\{(.+)\\}");

            QRegularExpressionMatch match = macroPattern.match(limit);
            while (match.capturedStart() != -1) {
                limit.replace(match.captured(0), mPayment->getParameter(match.captured(1)).value.toString());
                match = macroPattern.match(limit);
            }

            limit.toDouble(&convertOk);
            if (!convertOk) {
                limit = CPayment::DefaultMinLimit;
            }
        }

        addParameter("AMOUNT", limit);
        addParameter("AMOUNT_ALL", limit);
    }
}

//---------------------------------------------------------------------------

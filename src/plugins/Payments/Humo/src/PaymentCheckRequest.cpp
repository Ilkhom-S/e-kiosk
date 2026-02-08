/* @file Реализация запроса на проверку номера. */

#include "PaymentCheckRequest.h"

#include <QtCore/QRegularExpression>

#include "Payment.h"

namespace CPayment {
const char DefaultMinLimit[] = "200";
} // namespace CPayment

//---------------------------------------------------------------------------
PaymentCheckRequest::PaymentCheckRequest(Payment *aPayment, bool aFake)
    : PaymentRequest(aPayment, CPayment::Requests::Check) {
    addProviderParameters(CPayment::Requests::Check);

    if (aFake) {
        addParameter("REQ_TYPE", 1);

        QString limit = m_Payment->getProviderSettings().limits.check.isEmpty()
                            ? m_Payment->getProviderSettings().limits.min
                            : m_Payment->getProviderSettings().limits.check;

        // Сначала смотрим на сумму для проверку номера для оператора.
        bool convertOk(false);
        limit.toDouble(&convertOk);

        if (!convertOk) {
            // Если получается определить минимальный лимит оператора, используем его.
            // Иначе берём минимульную сумму CPayment::DefaultMinLimit.
            QRegularExpression macroPattern("\\{(.+)\\}");

            QRegularExpressionMatch match = macroPattern.match(limit);
            while (match.capturedStart() != -1) {
                limit.replace(match.captured(0),
                              m_Payment->getParameter(match.captured(1)).value.toString());
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

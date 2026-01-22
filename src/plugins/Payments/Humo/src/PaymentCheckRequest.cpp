/* @file Реализация запроса на проверку номера. */

// Project
#include "Payment.h"
#include "PaymentCheckRequest.h"

//---------------------------------------------------------------------------
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
            ////////macroPattern.setMinimal(true); // Removed for Qt5/6 compatibility // Removed for Qt5/6 compatibility // Removed for Qt5/6 compatibility // Removed for Qt5/6 compatibility

            while (macroPattern.match(limit).capturedStart() != -1) {
                limit.replace(// TODO: // TODO: // TODO: // TODO: macroPattern.cap(0) needs manual migration to match.captured(0) needs manual migration to match.captured(0) needs manual migration to match.captured(0) needs manual migration to match.captured(0), mPayment->getParameter(// TODO: // TODO: // TODO: // TODO: macroPattern.cap(1) needs manual migration to match.captured(1) needs manual migration to match.captured(1) needs manual migration to match.captured(1) needs manual migration to match.captured(1)).value.toString());
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

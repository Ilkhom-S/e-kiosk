/* @file Прокси-класс для работы с рекламным контентом в скриптах. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

#include <SDK/GUI/IAdSource.h>

namespace SDK {
namespace PaymentProcessor {

class ICore;

namespace Scripting {

/// Константы сервиса рекламы.
namespace CAdService {
extern const char DefaultBanner[];
} // namespace CAdService

//------------------------------------------------------------------------------
/// Прокси-класс для работы с рекламным контентом в скриптах.
class AdService : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString receiptHeader READ getReceiptHeader CONSTANT)
    Q_PROPERTY(QString receiptFooter READ getReceiptFooter CONSTANT)

public:
    /// Конструктор.
    AdService(ICore *aCore);

public slots:
    /// Добавить рекламное событие в статистику.
    void addEvent(const QString &aEvent, const QVariantMap &aParameters);

    /// Имя баннера.
    QString getBanner(const QString &aBanner = QString(CAdService::DefaultBanner));

private:
    /// Рекламный текст для шапки на чеке.
    QString getReceiptHeader();
    /// Рекламный текст для подвала на чеке.
    QString getReceiptFooter();

    /// Получить источник рекламы.
    SDK::GUI::IAdSource *getAdSource();
    /// Получить содержимое.
    QString getContent(const QString &aName);

private:
    ICore *mCore;
    SDK::GUI::IAdSource *mAdSource;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

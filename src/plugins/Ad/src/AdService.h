/* @file Сервис для работы с рекламой. */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QSharedPointer>
#include <QtCore/QUrl>

#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Core/IService.h>

#include <System/IApplication.h>

//---------------------------------------------------------------------------
namespace Ad {
struct Campaign;
class DatabaseUtils;
class Client;
} // namespace Ad

//---------------------------------------------------------------------------
class AdService : public QObject, public SDK::PaymentProcessor::IService, private ILogable {
    Q_OBJECT

public:
    //---------------------------------------------------------------------------
    // Получение AdService'а
    static AdService *instance(IApplication *aApplication);

    explicit AdService(IApplication *aApplication);
    virtual ~AdService();

    //---------------------------------------------------------------------------
    // IService: Инициализация сервиса
    virtual bool initialize();

    //---------------------------------------------------------------------------
    // IService: Закончена инициализация всех сервисов
    virtual void finishInitialize();

    //---------------------------------------------------------------------------
    // Возвращает false, если сервис не может быть остановлен в текущий момент
    virtual bool canShutdown();

    //---------------------------------------------------------------------------
    // IService: Завершение работы сервиса
    virtual bool shutdown();

    //---------------------------------------------------------------------------
    // IService: Возвращает имя сервиса
    virtual QString getName() const;

    //---------------------------------------------------------------------------
    // IService: Список необходимых сервисов
    virtual const QSet<QString> &getRequiredServices() const;

    //---------------------------------------------------------------------------
    // IService: Получить параметры сервиса
    virtual QVariantMap getParameters() const;

    //---------------------------------------------------------------------------
    // IService: Сброс служебной информации
    virtual void resetParameters(const QSet<QString> &aParameters);

#pragma region AdService interface

    //---------------------------------------------------------------------------
    // Получить содержимое рекламного контента
    virtual QVariant getContent(const QString &aName) const;

    //---------------------------------------------------------------------------
    // Зарегистрировать событие в статистике
    virtual void addEvent(const QString &aName);

#pragma endregion

private:
    IApplication *m_Application;
    QSettings *m_Settings;
    QMap<int, QString> m_ChannelsMap;

    QSharedPointer<Ad::DatabaseUtils> m_Database;
    QSharedPointer<Ad::Client> m_Client;
};

//---------------------------------------------------------------------------

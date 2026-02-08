/* @file Событийный менеджер. */

#pragma once

#include <QtCore/QObject>

#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/PaymentProcessor/Core/IService.h>

class Event;
class IApplication;

//---------------------------------------------------------------------------
class EventService : public QObject,
                     public SDK::PaymentProcessor::IEventService,
                     public SDK::PaymentProcessor::IService {
    Q_OBJECT

public:
    /// Получение EventService'а.
    static EventService *instance(IApplication *aApplication);

    EventService();
    virtual ~EventService() override;

    /// IEventService: Генерация события aEvent.
    virtual void sendEvent(const SDK::PaymentProcessor::Event &aEvent) override;

    /// Генерация события типа aEventType.
    void sendEvent(SDK::PaymentProcessor::EEventType::Enum aType, const QVariant &aData);

    /// IEventService: Подписывает объект aObject на получение событий в слот aSlot.
    /// Сигнатура слота: void aSlot(const Event & aEvent).
    virtual void subscribe(const QObject *aObject, const char *aSlot) override;

    /// IEventService: Отписывает объект от получения событий в слот aSlot.
    virtual void unsubscribe(const QObject *aObject, const char *aSlot) override;

    /// IService: Инициализация сервиса.
    virtual bool initialize() override;

    /// IService: Закончена инициализация всех сервисов.
    virtual void finishInitialize() override;

    /// IService: Возвращает false, если сервис не может быть остановлен в текущий момент.
    virtual bool canShutdown() override;

    /// IService: Завершение работы сервиса.
    virtual bool shutdown() override;

    /// IService: Возвращает имя сервиса.
    virtual QString getName() const override;

    /// IService: Список необходимых сервисов.
    virtual const QSet<QString> &getRequiredServices() const override;

    /// IService: Получить параметры сервиса.
    virtual QVariantMap getParameters() const override;

    /// IService: Сброс служебной информации.
    virtual void resetParameters(const QSet<QString> &aParameters) override;

signals:
    void event(const SDK::PaymentProcessor::Event &aEvent);
};

//---------------------------------------------------------------------------

/* @file Менеджер для работы со звуком. */

#pragma once

#include <QtCore/QSharedPointer>
#include <QtMultimedia/QMediaPlayer>

#include <Common/ILogable.h>

#include <SDK/PaymentProcessor/Core/IAudioService.h>
#include <SDK/PaymentProcessor/Core/IService.h>

#include <System/IApplication.h>

//---------------------------------------------------------------------------
class AudioService : public QObject,
                     public SDK::PaymentProcessor::IAudioService,
                     public SDK::PaymentProcessor::IService,
                     private ILogable {
    Q_OBJECT

public:
    /// Получение AudioService'а.
    static AudioService *instance(IApplication *aApplication);

    AudioService(IApplication *aApplication);
    virtual ~AudioService();

    /// IService: Инициализация сервиса.
    virtual bool initialize() override;

    /// IService: Закончена инициализация всех сервисов.
    virtual void finishInitialize() override;

    /// Возвращает false, если сервис не может быть остановлен в текущий момент.
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

#pragma region SDK::PaymentProcessor::IAudioService interface

    /// Воспроизвести звуковой файл.
    virtual void play(const QString &aFileName);

    /// Остановить воспроизведение.
    virtual void stop();

#pragma endregion

private slots:
    /// Изменение состояния проигрывателя музыки
    void stateChanged(QMediaPlayer::PlaybackState aState);

private:
    IApplication *m_Application;
    QString m_InterfacePath;
    QSharedPointer<QMediaPlayer> m_Player;
};

//---------------------------------------------------------------------------

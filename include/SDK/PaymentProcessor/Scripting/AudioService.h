/* @file Прокси-класс для работы со звуком в скриптах. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

namespace SDK {
namespace PaymentProcessor {

class ICore;
class IAudioService;

namespace Scripting {

//------------------------------------------------------------------------------
/// Прокси-класс для работы со звуком в скриптах.
class AudioService : public QObject {
    Q_OBJECT

public:
    /// Конструктор.
    AudioService(ICore *aCore);

public slots:
    /// Воспроизводит wav-файл.
    void play(const QString &aFileName);

    /// Остановить воспроизведение.
    void stop();

private:
    /// Указатель на ядро.
    ICore *mCore;
    /// Указатель на сервис звука.
    IAudioService *mAudioService;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

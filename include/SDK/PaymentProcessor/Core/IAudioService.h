/* @file Интерфейс, обеспечивающий взаимодействие со звуком. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

namespace SDK {
namespace PaymentProcessor {

//------------------------------------------------------------------------------
class IAudioService {
public:
    /// Воспроизвести звуковой файл.
    virtual void play(const QString &aFileName) = 0;

    /// Остановить воспроизведение.
    virtual void stop() = 0;

protected:
    virtual ~IAudioService() {}
};

//------------------------------------------------------------------------------
} // namespace PaymentProcessor
} // namespace SDK

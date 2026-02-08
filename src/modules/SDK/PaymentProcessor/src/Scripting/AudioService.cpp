/* @file Прокси-класс для работы со звуком в скриптах. */

#include <SDK/PaymentProcessor/Core/IAudioService.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Scripting/AudioService.h>

namespace SDK {
namespace PaymentProcessor {
namespace Scripting {

//------------------------------------------------------------------------------
AudioService::AudioService(ICore *aCore)
    : m_Core(aCore), m_AudioService(nullptr /*m_Core->getAudioService()*/) {}

//------------------------------------------------------------------------------
void AudioService::play(const QString &aFileName) {
    m_AudioService->play(aFileName);
}

//------------------------------------------------------------------------------
void AudioService::stop() {
    m_AudioService->stop();
}

//------------------------------------------------------------------------------

} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

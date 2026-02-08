/* @file Менеджер для работы со звуком. */

#include "Services/AudioService.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QString>

#include "Services/ServiceNames.h"
#include "System/IApplication.h"
#include "System/SettingsConstants.h"

namespace CAudioService {
/// Название лога.
const char LogName[] = "Interface";
} // namespace CAudioService

//---------------------------------------------------------------------------
AudioService *AudioService::instance(IApplication *aApplication) {
    return static_cast<AudioService *>(
        aApplication->getCore()->getService(CServices::AudioService));
}

//---------------------------------------------------------------------------
AudioService::AudioService(IApplication *aApplication)
    : m_Application(aApplication), ILogable(CAudioService::LogName) {
    // Получаем директорию с файлами интерфейса из настроек.
    m_InterfacePath = IApplication::toAbsolutePath(
        m_Application->getSettings().value(CSettings::InterfacePath).toString());
}

//---------------------------------------------------------------------------
AudioService::~AudioService() {}

//---------------------------------------------------------------------------
bool AudioService::initialize() {
    m_Player = QSharedPointer<QMediaPlayer>(new QMediaPlayer());
    connect(
        m_Player.data(), &QMediaPlayer::playbackStateChanged, this, &AudioService::stateChanged);

    return true;
}

//------------------------------------------------------------------------------
void AudioService::finishInitialize() {}

//---------------------------------------------------------------------------
bool AudioService::canShutdown() {
    return true;
}

//---------------------------------------------------------------------------
bool AudioService::shutdown() {
    return true;
}

//---------------------------------------------------------------------------
QString AudioService::getName() const {
    return CServices::AudioService;
}

//---------------------------------------------------------------------------
const QSet<QString> &AudioService::getRequiredServices() const {
    static QSet<QString> requiredResources;
    return requiredResources;
}

//---------------------------------------------------------------------------
QVariantMap AudioService::getParameters() const {
    return QVariantMap();
}

//---------------------------------------------------------------------------
void AudioService::resetParameters(const QSet<QString> &) {}

//---------------------------------------------------------------------------
void AudioService::play(const QString &aFileName) {
    QString filePath = m_InterfacePath + "/" + aFileName;

    if (QFile::exists(filePath)) {
        if (m_Player->playbackState() != QMediaPlayer::StoppedState) {
            stop();
        }

        m_Player->setSource(QUrl::from_LocalFile(filePath));
        m_Player->play();
    } else {
        stop();

        toLog(LogLevel::Warning, QString("Audio file %1 not found.").arg(aFileName));
    }
}

//---------------------------------------------------------------------------
void AudioService::stop() {
    if (m_Player) {
        m_Player->stop();
    }
}

//---------------------------------------------------------------------------
void AudioService::stateChanged(QMediaPlayer::PlaybackState aState) {
    if (aState == QMediaPlayer::StoppedState) {
    }
}

//---------------------------------------------------------------------------
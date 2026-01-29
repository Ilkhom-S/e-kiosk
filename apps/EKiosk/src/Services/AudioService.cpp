/* @file Менеджер для работы со звуком. */

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

// System
#include "Services/AudioService.h"
#include "Services/ServiceNames.h"
#include "System/IApplication.h"
#include "System/SettingsConstants.h"

namespace CAudioService
{
    /// Название лога.
    const char LogName[] = "Interface";
} // namespace CAudioService

//---------------------------------------------------------------------------
AudioService *AudioService::instance(IApplication *aApplication)
{
    return static_cast<AudioService *>(aApplication->getCore()->getService(CServices::AudioService));
}

//---------------------------------------------------------------------------
AudioService::AudioService(IApplication *aApplication) : mApplication(aApplication), ILogable(CAudioService::LogName)
{
    // Получаем директорию с файлами интерфейса из настроек.
    mInterfacePath =
        IApplication::toAbsolutePath(mApplication->getSettings().value(CSettings::InterfacePath).toString());
}

//---------------------------------------------------------------------------
AudioService::~AudioService()
{
}

//---------------------------------------------------------------------------
bool AudioService::initialize()
{
    mPlayer = QSharedPointer<QMediaPlayer>(new QMediaPlayer());
    connect(mPlayer.data(), &QMediaPlayer::playbackStateChanged, this, &AudioService::stateChanged);

    return true;
}

//------------------------------------------------------------------------------
void AudioService::finishInitialize()
{
}

//---------------------------------------------------------------------------
bool AudioService::canShutdown()
{
    return true;
}

//---------------------------------------------------------------------------
bool AudioService::shutdown()
{
    return true;
}

//---------------------------------------------------------------------------
QString AudioService::getName() const
{
    return CServices::AudioService;
}

//---------------------------------------------------------------------------
const QSet<QString> &AudioService::getRequiredServices() const
{
    static QSet<QString> requiredResources;
    return requiredResources;
}

//---------------------------------------------------------------------------
QVariantMap AudioService::getParameters() const
{
    return QVariantMap();
}

//---------------------------------------------------------------------------
void AudioService::resetParameters(const QSet<QString> &)
{
}

//---------------------------------------------------------------------------
void AudioService::play(const QString &aFileName)
{
    QString filePath = mInterfacePath + "/" + aFileName;

    if (QFile::exists(filePath))
    {
        if (mPlayer->playbackState() != QMediaPlayer::StoppedState)
        {
            stop();
        }

        mPlayer->setSource(QUrl::fromLocalFile(filePath));
        mPlayer->play();
    }
    else
    {
        stop();

        toLog(LogLevel::Warning, QString("Audio file %1 not found.").arg(aFileName));
    }
}

//---------------------------------------------------------------------------
void AudioService::stop()
{
    if (mPlayer)
    {
        mPlayer->stop();
    }
}

//---------------------------------------------------------------------------
void AudioService::stateChanged(QMediaPlayer::PlaybackState aState)
{
    if (aState == QMediaPlayer::StoppedState)
    {
    }
}

//---------------------------------------------------------------------------
# Audio Service

The Audio Service manages audio playback for the EKiosk system.

## Overview

The Audio Service (`IAudioService`) handles:

- Audio file playback (WAV, MP3, and other Qt-supported formats)
- Simple play/stop controls
- Asynchronous playback via Qt QMediaPlayer

## Current Implementation

**Note**: Audio service provides basic playback functionality. Advanced features like
text-to-speech, recording, and volume control are not currently implemented.

## Interface

```cpp
class IAudioService {
public:
    /// Play audio file
    virtual void play(const QString &aFileName) = 0;

    /// Stop playback
    virtual void stop() = 0;
};
```

## Usage

### Playing Audio Files

```cpp
// Get audio service from core
auto audioService = core->getAudioService();

if (!audioService) {
    LOG(log, LogLevel::Error, "Audio service not available");
    return;
}

// Play audio file
audioService->play("sounds/welcome.wav");
```

### Stopping Playback

```cpp
// Stop audio playback
audioService->stop();
```

## File Paths

Audio files are resolved relative to the application working directory.

**Supported formats**: WAV, MP3, OGG (depends on Qt platform support)

## Implementation Details

The AudioService wraps Qt's QMediaPlayer for audio playback:

- **Backend**: Qt Multimedia (QMediaPlayer)
- **Threading**: Audio playback occurs in the service's thread

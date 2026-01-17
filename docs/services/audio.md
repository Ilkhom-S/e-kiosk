# Audio Service

The Audio Service manages audio playback and recording for the EKiosk system.

## Overview

The Audio Service (`IAudioService`) handles:

- Audio file playback
- Text-to-speech synthesis
- Audio recording
- Volume and audio device control
- Sound effect management
- Audio stream processing

## Interface

```cpp
class IAudioService : public QObject {
    Q_OBJECT

public:
    enum AudioStatus { Playing, Paused, Stopped, Error };

    /// Play audio file
    virtual bool playFile(const QString &filePath, bool async = true) = 0;

    /// Play text-to-speech
    virtual bool speakText(const QString &text, const QString &language = "en") = 0;

    /// Stop current playback
    virtual bool stopPlayback() = 0;

    /// Pause/resume playback
    virtual bool pausePlayback() = 0;
    virtual bool resumePlayback() = 0;

    /// Get playback status
    virtual AudioStatus getPlaybackStatus() const = 0;

    /// Set volume level
    virtual bool setVolume(int volume) = 0;  // 0-100

    /// Get current volume
    virtual int getVolume() const = 0;

    /// Start audio recording
    virtual bool startRecording(const QString &filePath) = 0;

    /// Stop audio recording
    virtual bool stopRecording() = 0;

    /// Get available audio devices
    virtual QStringList getAudioDevices() const = 0;

    /// Set default audio device
    virtual bool setDefaultAudioDevice(const QString &deviceName) = 0;

    // ... additional methods for audio management
};
```

## Audio Playback

### Playing Audio Files

```cpp
// Get audio service from core
auto audioService = core->getAudioService();

if (!audioService) {
    LOG(log, LogLevel::Error, "Audio service not available");
    return;
}

// Play welcome sound
bool playing = audioService->playFile("sounds/welcome.wav");

if (playing) {
    LOG(log, LogLevel::Info, "Welcome sound playing");
} else {
    LOG(log, LogLevel::Error, "Failed to play welcome sound");
}
```

### Asynchronous Playback

```cpp
// Play sound effect asynchronously (non-blocking)
audioService->playFile("sounds/button_click.wav", true);

// Continue with other operations immediately
processUserInput();
```

### Playback Control

```cpp
// Control playback
audioService->pausePlayback();
QThread::msleep(2000);  // Wait 2 seconds
audioService->resumePlayback();

// Stop playback
audioService->stopPlayback();
```

## Text-to-Speech

### Basic Speech Synthesis

```cpp
// Speak welcome message
bool spoken = audioService->speakText("Welcome to EKiosk. How can I help you today?");

if (spoken) {
    LOG(log, LogLevel::Info, "Welcome message spoken");
} else {
    LOG(log, LogLevel::Error, "Failed to speak welcome message");
}
```

### Multi-language Support

```cpp
// Speak in different languages
audioService->speakText("Welcome to EKiosk", "en");     // English
audioService->speakText("Bienvenido a EKiosk", "es");   // Spanish
audioService->speakText("Willkommen bei EKiosk", "de"); // German
audioService->speakText("Добро пожаловать в EKiosk", "ru"); // Russian
```

### Status Announcements

```cpp
void announcePaymentStatus(const QString &status, double amount) {
    QString message;

    if (status == "completed") {
        message = QString("Payment completed. Amount: %1 dollars.").arg(amount);
    } else if (status == "failed") {
        message = "Payment failed. Please try again.";
    } else if (status == "processing") {
        message = "Processing payment. Please wait.";
    }

    audioService->speakText(message);
}
```

## Volume Control

### Setting Volume

```cpp
// Set volume to 75%
bool volumeSet = audioService->setVolume(75);

if (volumeSet) {
    LOG(log, LogLevel::Info, "Volume set to 75%");
} else {
    LOG(log, LogLevel::Error, "Failed to set volume");
}
```

### Volume Management

```cpp
// Get current volume
int currentVolume = audioService->getVolume();
LOG(log, LogLevel::Info, QString("Current volume: %1%").arg(currentVolume));

// Adjust volume for different contexts
void setContextVolume(const QString &context) {
    int volume = 50;  // Default

    if (context == "payment") {
        volume = 30;  // Lower for payment processing
    } else if (context == "error") {
        volume = 80;  // Higher for error messages
    } else if (context == "welcome") {
        volume = 60;  // Medium for welcome messages
    }

    audioService->setVolume(volume);
}
```

## Audio Recording

### Recording Audio

```cpp
// Start recording user feedback
QString recordingPath = QString("recordings/feedback_%1.wav")
                        .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));

bool recording = audioService->startRecording(recordingPath);

if (recording) {
    LOG(log, LogLevel::Info, QString("Recording started: %1").arg(recordingPath));

    // Record for 10 seconds
    QThread::sleep(10);

    // Stop recording
    bool stopped = audioService->stopRecording();

    if (stopped) {
        LOG(log, LogLevel::Info, "Recording completed");
        // Process recorded file
        processRecording(recordingPath);
    } else {
        LOG(log, LogLevel::Error, "Failed to stop recording");
    }
} else {
    LOG(log, LogLevel::Error, "Failed to start recording");
}
```

## Audio Device Management

### Available Devices

```cpp
// Get available audio devices
QStringList devices = audioService->getAudioDevices();

LOG(log, LogLevel::Info, QString("Available audio devices: %1").arg(devices.join(", ")));

// Set preferred audio device
if (devices.contains("External Speaker")) {
    audioService->setDefaultAudioDevice("External Speaker");
}
```

### Device Status

```cpp
// Check audio device status
IAudioService::AudioStatus status = audioService->getPlaybackStatus();

switch (status) {
    case IAudioService::Playing:
        LOG(log, LogLevel::Info, "Audio is currently playing");
        break;
    case IAudioService::Paused:
        LOG(log, LogLevel::Info, "Audio is paused");
        break;
    case IAudioService::Stopped:
        LOG(log, LogLevel::Info, "Audio is stopped");
        break;
    case IAudioService::Error:
        LOG(log, LogLevel::Error, "Audio playback error");
        break;
}
```

## Usage in Plugins

Audio Service is commonly used in user interface and notification plugins:

```cpp
class AudioFeedbackPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mAudioService = mCore->getAudioService();
            mEventService = mCore->getEventService();
            mLog = kernel->getLog("AudioFeedback");
        }

        return true;
    }

    void onUserInteraction(const QString &interactionType) {
        if (!mAudioService) return;

        QString soundFile;

        if (interactionType == "button_press") {
            soundFile = "sounds/button_press.wav";
        } else if (interactionType == "payment_success") {
            soundFile = "sounds/payment_success.wav";
            // Also speak confirmation
            mAudioService->speakText("Payment successful. Thank you!");
        } else if (interactionType == "error") {
            soundFile = "sounds/error.wav";
            mAudioService->speakText("An error occurred. Please try again.");
        } else if (interactionType == "welcome") {
            soundFile = "sounds/welcome.wav";
            mAudioService->speakText("Welcome to EKiosk!");
        }

        if (!soundFile.isEmpty()) {
            bool played = mAudioService->playFile(soundFile, true);
            if (!played) {
                LOG(mLog, LogLevel::Warning, QString("Failed to play sound: %1").arg(soundFile));
            }
        }
    }

    void announceProductScan(const QString &productName, double price) {
        QString message = QString("Scanned: %1. Price: %2 dollars.").arg(productName).arg(price);
        bool spoken = mAudioService->speakText(message);

        if (spoken) {
            LOG(mLog, LogLevel::Info, QString("Announced product: %1").arg(productName));
        } else {
            LOG(mLog, LogLevel::Error, "Failed to announce product");
        }
    }

    void providePaymentGuidance() {
        // Set appropriate volume for payment
        mAudioService->setVolume(40);

        // Guide user through payment process
        mAudioService->speakText("Please select your payment method.");

        // Wait for user input...

        mAudioService->speakText("Processing payment. Please wait.");

        // After payment processing
        mAudioService->speakText("Payment completed successfully.");
    }

    void handleError(const QString &errorType) {
        // Set higher volume for errors
        mAudioService->setVolume(70);

        QString message;

        if (errorType == "card_declined") {
            message = "Card declined. Please try a different payment method.";
        } else if (errorType == "insufficient_funds") {
            message = "Insufficient funds. Please check your account balance.";
        } else if (errorType == "network_error") {
            message = "Network error. Please try again later.";
        } else {
            message = "An error occurred. Please contact support.";
        }

        // Play error sound and speak message
        mAudioService->playFile("sounds/error.wav", true);
        mAudioService->speakText(message);

        LOG(mLog, LogLevel::Error, QString("Audio error feedback: %1").arg(message));
    }

    void playBackgroundMusic(bool enable) {
        static QString musicFile = "sounds/background_music.wav";

        if (enable) {
            // Play background music at low volume
            mAudioService->setVolume(20);
            bool playing = mAudioService->playFile(musicFile, true);

            if (playing) {
                LOG(mLog, LogLevel::Info, "Background music started");
            }
        } else {
            // Stop background music
            mAudioService->stopPlayback();
            LOG(mLog, LogLevel::Info, "Background music stopped");
        }
    }

private:
    IAudioService *mAudioService;
    IEventService *mEventService;
    ILog *mLog;
};
```

## Error Handling

```cpp
try {
    // Check audio service availability
    if (!audioService) {
        throw std::runtime_error("Audio service not available");
    }

    // Check audio devices
    QStringList devices = audioService->getAudioDevices();
    if (devices.isEmpty()) {
        throw std::runtime_error("No audio devices available");
    }

    // Attempt audio operation
    if (!audioService->playFile("sounds/test.wav")) {
        throw std::runtime_error("Failed to play audio file");
    }

} catch (const std::runtime_error &e) {
    LOG(log, LogLevel::Error, QString("Audio service error: %1").arg(e.what()));

    // Handle error - show visual feedback, try alternative output, etc.
    handleAudioError(e.what());

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected audio error: %1").arg(e.what()));
}
```

## Audio Configuration

```cpp
// Configure audio settings through settings service
void configureAudio() {
    auto settings = core->getSettingsService();

    // Configure default volume
    int defaultVolume = settings->getValue("Audio/DefaultVolume", 50).toInt();
    audioService->setVolume(defaultVolume);

    // Configure TTS language
    QString ttsLanguage = settings->getValue("Audio/TTSLanguage", "en").toString();

    // Configure audio device
    QString audioDevice = settings->getValue("Audio/DefaultDevice", "default").toString();
    audioService->setDefaultAudioDevice(audioDevice);

    // Configure audio feedback
    bool audioFeedbackEnabled = settings->getValue("Audio/FeedbackEnabled", true).toBool();

    if (audioFeedbackEnabled) {
        LOG(log, LogLevel::Info, "Audio feedback enabled");
    }
}
```

## Performance Considerations

- Use asynchronous playback for non-blocking operations
- Cache frequently used audio files
- Consider audio compression for storage efficiency
- Monitor audio buffer levels to prevent underruns

## Accessibility Features

```cpp
// Provide audio assistance for visually impaired users
void enableAudioAssistance() {
    // Enable text-to-speech for all UI elements
    mAudioService->speakText("Audio assistance enabled");

    // Subscribe to UI events for audio feedback
    connect(ui, &UserInterface::elementFocused,
            this, &AudioAssistance::announceElement);

    connect(ui, &UserInterface::buttonClicked,
            this, &AudioAssistance::confirmAction);
}

void announceElement(const QString &elementName, const QString &elementType) {
    QString announcement = QString("%1 %2").arg(elementType, elementName);
    mAudioService->speakText(announcement);
}

void confirmAction(const QString &action) {
    mAudioService->speakText(QString("Activated: %1").arg(action));
}
```

## Security Considerations

- Validate audio file paths to prevent directory traversal
- Sanitize text input for TTS to prevent command injection
- Implement audio content filtering if needed
- Secure audio recording storage and access

## Dependencies

- Settings Service (for audio configuration)
- Event Service (for audio event notifications)
- Device Service (for audio hardware management)

## See Also

- [Settings Service](settings.md) - Audio configuration
- [Event Service](event.md) - Audio event notifications
- [Device Service](device.md) - Audio hardware management

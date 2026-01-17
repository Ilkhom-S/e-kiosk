# Crypt Service

The Crypt Service provides cryptographic operations and key management for secure transactions in EKiosk.

## Overview

The Crypt Service (`ICryptService`) is responsible for:

- Key generation and registration with remote servers
- Key storage and management
- Key replacement operations
- Access to the cryptographic engine (`ICryptEngine`)

## Interface

```cpp
class ICryptService {
public:
    /// Generate and register key on server
    virtual int generateKey(int aKeyId, const QString &aLogin, const QString &aPassword,
                           const QString &aURL, QString &aSD, QString &aAP, QString &aOP,
                           const QString &aDescription = QString()) = 0;

    /// Save generated key
    virtual bool saveKey() = 0;

    /// Replace keys
    virtual bool replaceKeys(int aKeyIdSrc, int aKeyIdDst) = 0;

    /// Get loaded key IDs
    virtual QList<int> getLoadedKeys() const = 0;

    /// Get key information
    virtual SKeyInfo getKeyInfo(int aKeyId) = 0;

    /// Get cryptographic engine
    virtual ICryptEngine *getCryptEngine() = 0;
};
```

## Key Management

### Key Generation

```cpp
auto cryptService = core->getCryptService();

// Generate new key pair
int result = cryptService->generateKey(
    keyId,           // Key pair ID
    login,           // Server login
    password,        // Server password
    serverUrl,       // Server URL
    sd, ap, op,      // Output: key components
    description      // Optional description
);

if (result >= 0) {
    // Key generated successfully
    bool saved = cryptService->saveKey();
}
```

### Key Information

```cpp
// Get information about a key
ICryptService::SKeyInfo keyInfo = cryptService->getKeyInfo(keyId);
if (keyInfo.isValid()) {
    qDebug() << "Dealer Code:" << keyInfo.sd;
    qDebug() << "Point Code:" << keyInfo.ap;
    qDebug() << "Operator Code:" << keyInfo.op;
}
```

### Key Replacement

```cpp
// Replace one key pair with another
bool success = cryptService->replaceKeys(sourceKeyId, destinationKeyId);
```

## Cryptographic Engine

The service provides access to the cryptographic engine for low-level operations:

```cpp
ICryptEngine *engine = cryptService->getCryptEngine();

// Use engine for encryption/decryption operations
// (See ICryptEngine documentation for details)
```

## Usage in Plugins

The Crypt Service is commonly used in payment plugins for secure transaction processing:

```cpp
class PaymentPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mCryptService = mCore->getCryptService();
            mCryptEngine = mCryptService->getCryptEngine();
        }

        return true;
    }

private:
    SDK::PaymentProcessor::ICore *mCore;
    ICryptService *mCryptService;
    ICryptEngine *mCryptEngine;
};
```

## Configuration

Crypt Service configuration is typically managed through the Settings Service:

```cpp
auto settings = core->getSettingsService();
auto adapter = settings->getAdapter("CryptSettings");
// Configure server URLs, timeouts, etc.
```

## Error Handling

```cpp
try {
    int result = cryptService->generateKey(/* parameters */);
    if (result < 0) {
        // Handle key generation failure
        LOG(log, LogLevel::Error, QString("Key generation failed: %1").arg(result));
    }
} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Crypt service error: %1").arg(e.what()));
}
```

## Thread Safety

The Crypt Service is generally thread-safe for read operations but key generation and modification operations should be performed sequentially.

## Dependencies

- Settings Service (for configuration)
- Network Service (for server communication)
- Logging Service (for diagnostics)

## See Also

- [Payment Service](payment.md) - Payment processing operations
- [Network Service](network.md) - Server communication
- [Settings Service](settings.md) - Configuration management

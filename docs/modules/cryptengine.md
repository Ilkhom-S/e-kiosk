# CryptEngine Module

## Purpose

The CryptEngine module provides comprehensive cryptographic operations for EKiosk applications, based on the **libipriv** library. It enables digital signatures (RSA/GOST), encryption/decryption, hardware security token integration, and secure key pair management for payment processing and secure server communication.

---

## Quick start 🔧

```cpp
#include <Crypt/ICryptEngine.h>
#include <Crypt/CryptEngine.h>

// Get crypto instance (singleton)
ICryptEngine& crypto = CryptEngine::instance();

// Initialize cryptographic system
if (!crypto.initialize()) {
    qWarning() << "Failed to initialize crypto";
    return;
}

// Load key pair from disk
QString errorDesc;
if (!crypto.loadKeyPair(0,                           // Key pair ID
                        CCrypt::ETypeEngine::File,   // Use file-based keys
                        "keys/secret.key",           // Private key path
                        "password",                  // Key password
                        "keys/public.key",           // Public key path
                        12345,                       // Serial number
                        67890,                       // Bank serial number
                        errorDesc)) {
    qWarning() << "Failed to load keys:" << errorDesc;
    return;
}

// Sign payment data (attached signature)
QByteArray paymentData = "payment_info";
QByteArray signature;
if (!crypto.sign(0, paymentData, signature, errorDesc)) {
    qWarning() << "Sign failed:" << errorDesc;
    return;
}

// Verify response from server (detached signature)
QByteArray serverResponse = "response_data";
QByteArray serverSignature = "signature_data";
if (crypto.verifyDetach(0, serverResponse, serverSignature, errorDesc)) {
    qDebug() << "Server response verified!";
} else {
    qWarning() << "Verification failed:" << errorDesc;
}

// Cleanup
crypto.releaseKeyPair(0);
crypto.shutdown();
```

---

## Features

- **Digital Signatures**: Attached and detached signature support with RSA and GOST algorithms
- **Encryption/Decryption**: Standard and large-data variants (RSA-based)
- **Hash Algorithms**: MD5 (deprecated), SHA-256 (recommended), configurable at compile-time
- **Hardware Token Support**: RuToken smart card integration (compile-time optional)
- **Key Management**: Load, create, export private/public key pairs with serialization
- **Server Communication**: Secure request/response handling with signatures
- **Thread Safety**: All operations are mutex-protected for concurrent access
- **Error Handling**: Descriptive error messages for all operations

---

## Platform support

| Platform | Status          | Notes                                      |
| -------- | --------------- | ------------------------------------------ |
| Windows  | ✅ Full support | Complete crypto + RuToken hardware support |
| Linux    | 🔬 Experimental | Functional, limited hardware token testing |
| macOS    | 🔬 Experimental | Functional, limited hardware token testing |

---

## Configuration

### Compile-time Options

```cpp
// Hash algorithm selection (in CryptEngine.cpp)
#ifdef TC_USE_MD5
    // Uses MD5 (deprecated, for legacy compatibility)
#else
    // Uses SHA-256 (recommended, default)
#endif

// Hardware token support
#ifdef TC_USE_TOKEN
    // Enables RuToken smart card integration
#endif
```

### INI Configuration (Optional)

```ini
[Crypto]
# Key storage location
KeyPath=./keys/

# Enable hardware token support
UseHardwareToken=false

# Default key pair ID
DefaultKeyPairId=0
```

---

## API Reference

### Initialization

```cpp
bool initialize();              // Initialize crypto library
void shutdown();                // Cleanup and release resources
QSet<CCrypt::ETypeEngine> availableEngines();  // Get available crypto engines
```

### Key Management

```cpp
// Load existing key pair from disk or token
bool loadKeyPair(int aKeyPair,
                 CCrypt::ETypeEngine aEngine,      // File or RuToken
                 const QString &aSecretKeyPath,    // Private key path
                 const QString &aPassword,          // Key password
                 const QString &aPublicKeyPath,     // Public key path
                 const ulong aSerialNumber,
                 const ulong aBankSerialNumber,
                 QString &aErrorDescription);       // Output error message

// Create new key pair
bool createKeyPair(int aKeyPair,
                   CCrypt::ETypeEngine aEngine,
                   const QByteArray &aKeyCard,
                   int aKeySize,                    // Usually 2048
                   QString &aErrorDescription);

// Retrieve key serial number
QString getKeyPairSerialNumber(int aKeyPair);

// Release specific or all key pairs
bool releaseKeyPair(int aKeyPair);
void releaseKeyPairs();

// Export keys
bool exportSecretKey(int aKeyPair,
                     QByteArray &aSecretKey,
                     const QByteArray &aPassword);
bool exportPublicKey(int aKeyPair,
                     QByteArray &aPublicKey,
                     ulong &aSerialNumber);

// Replace public key in pair
bool replacePublicKey(int aKeyPair, const QByteArray &aPublicKey);
```

### Digital Signatures

```cpp
// Embedded signature (signature data combined with message)
bool sign(int aKeyPair,
          const QByteArray &aRequest,
          QByteArray &aSignature,                // Contains both data and signature
          QString &aErrorDescription);

bool verify(int aKeyPair,
            const QByteArray &aResponseString,   // Signed data
            QByteArray &aOriginalResponse,       // Extracted original data
            QString &aErrorDescription);

// Detached signature (signature separate from message)
bool signDetach(int aKeyPair,
                const QByteArray &aRequest,
                QByteArray &aSignature,          // Signature only
                QString &aErrorDescription);

bool verifyDetach(int aKeyPair,
                  const QByteArray &aResponseString,  // Original data
                  const QByteArray &aSignature,       // Signature to verify
                  QString &aErrorDescription);
```

### Encryption/Decryption

```cpp
// Standard encryption (max ~256 bytes)
bool encrypt(int aKeyPair,
             const QByteArray &aData,
             QByteArray &aResult,
             CCrypt::ETypeKey aKey,              // Public or Private
             QString &aErrorDescription);

bool encrypt(int aKeyPair,
             const QByteArray &aData,
             QByteArray &aResult,                // Shorthand: uses public key
             QString &aErrorDescription);

// Large data encryption
bool encryptLong(int aKeyPair,
                 const QByteArray &aData,
                 QByteArray &aResult,
                 CCrypt::ETypeKey aKey,
                 QString &aErrorDescription);

bool encryptLong(int aKeyPair,
                 const QByteArray &aData,
                 QByteArray &aResult,            // Shorthand: uses public key
                 QString &aErrorDescription);

// Standard decryption
bool decrypt(int aKeyPair,
             const QByteArray &aData,
             QByteArray &aResult,
             CCrypt::ETypeKey aKey,             // Usually Private
             QString &aErrorDescription);

bool decrypt(int aKeyPair,
             const QByteArray &aData,
             QByteArray &aResult,               // Shorthand: uses private key
             QString &aErrorDescription);

// Large data decryption
bool decryptLong(int aKeyPair,
                 const QByteArray &aData,
                 QByteArray &aResult,
                 CCrypt::ETypeKey aKey,
                 QString &aErrorDescription);

bool decryptLong(int aKeyPair,
                 const QByteArray &aData,
                 QByteArray &aResult,           // Shorthand: uses private key
                 QString &aErrorDescription);
```

### Hardware Token Support

```cpp
// Token status and initialization
CCrypt::TokenStatus getTokenStatus(CCrypt::ETypeEngine aEngine);
bool initializeToken(CCrypt::ETypeEngine aEngine);

// Password management
QByteArray generatePassword() const;              // Generate random password
QList<QByteArray> getRootPassword() const;       // Get root key passwords
```

---

## Common Patterns

### Thread Safety

All public methods are **thread-safe**. Multiple threads can safely call crypto operations without additional synchronization:

```cpp
// Safe from multiple threads
QThread::create([&crypto]() {
    QByteArray sig;
    QString err;
    crypto.sign(0, data, sig, err);
})->start();
```

### Error Handling

Always check return value and inspect error description:

```cpp
QString errorDesc;
if (!crypto.sign(0, data, signature, errorDesc)) {
    // errorDesc contains human-readable error message
    qWarning() << "Signature failed:" << errorDesc;
    return false;
}
```

### Key Pair Lifecycle

```cpp
// Load at startup
crypto.initialize();
crypto.loadKeyPair(0, CCrypt::ETypeEngine::File, ...);

// Use throughout application
crypto.sign(0, ...);
crypto.verify(0, ...);

// Cleanup at shutdown
crypto.releaseKeyPair(0);
crypto.shutdown();
```

---

## Integration

### CMake

```cmake
find_package(libipriv REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

ek_add_library(CryptEngine
    FOLDER "modules/CryptEngine"
    SOURCES ${SOURCES}
    DEPENDS libipriv Qt${QT_VERSION_MAJOR}::Core
)
```

### Dependencies

- **libipriv** (v1.0.2+): Core cryptographic library (GOST/RSA)
- **Qt5.15+ or Qt6**: Core module for thread safety (QMutex, QByteArray, QString)
  - Qt 5.15 LTS: Supported on Windows 7, Linux
  - Qt 6.x: Supported on Windows 10+, Linux, macOS
- Optional: **RuToken libraries** for hardware token support

---

## Testing

Unit tests are located in `tests/modules/CryptEngine/`.

Tests cover:

- Key pair loading and management
- Signature creation and verification (attached and detached)
- Encryption/decryption (standard and long data)
- Hardware token integration (if compiled with TC_USE_TOKEN)
- Thread-safe concurrent operations
- Error handling and recovery

**Run tests:**

```bash
ctest -R CryptEngine -V
```

---

## Migration notes

### Porting & Compatibility

- **Ported from**: TerminalClient project
- **Updated for**: EKiosk modular architecture
- **Qt compatibility**: Qt 5.15 LTS and Qt 6.x
  - Code uses version-agnostic interfaces where possible
  - Platform-specific Qt versions:
    - **Windows 7**: Qt 5.15 LTS only
    - **Windows 10+, Linux, macOS**: Qt 6.x recommended (Qt 5.15 supported)
  - No breaking API differences between versions

### Key architectural changes from TerminalClient

- Singleton instance instead of factory pattern
- Key pair ID-based operations (supports multiple simultaneous key pairs)
- Explicit engine type parameter (file vs. hardware token)
- All methods return error descriptions via reference parameter

---

## Further reading

- **Implementation details**: [src/modules/CryptEngine/README.md](../../src/modules/CryptEngine/README.md)
- **Header files**: [include/Crypt/](../../include/Crypt/)
- **libipriv documentation**: [thirdparty/libipriv/](../../thirdparty/libipriv/)</content>
  <parameter name="filePath">c:\Projects\Humo\Kiosk\ekiosk\docs\modules\cryptengine.md

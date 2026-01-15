# CryptEngine Module

## Purpose

The CryptEngine module provides comprehensive cryptographic operations for EKiosk applications, including digital signatures, encryption/decryption, hashing, and hardware token integration using the libipriv library.

---

## Quick start 🔧

```cpp
#include "CryptEngine/ICryptEngine.h"

// Get crypto instance
ICryptEngine* crypto = CryptEngineFactory::create();

// Load private key
crypto->loadPrivateKey("keys/private.key", "password");

// Sign payment data
QByteArray signature = crypto->sign(paymentData);

// Verify signature
bool valid = crypto->verify(data, signature);
```

---

## Features

- **Digital signatures**: RSA and GOST algorithm support
- **Encryption/decryption**: IDEA and AES algorithms
- **Hashing**: MD5, SHA-1, SHA-256 support
- **Hardware token integration**: Support for cryptographic hardware tokens
- **Key management**: Private/public key loading and management
- **Certificate handling**: X.509 certificate operations
- **Secure random generation**: Cryptographically secure random number generation

---

## Platform support

| Platform | Status  | Notes                                    |
| -------- | ------- | ---------------------------------------- |
| Windows  | ✅ Full | Full crypto support with hardware tokens |
| Linux    | ✅ Full | Full crypto support with hardware tokens |
| macOS    | ✅ Full | Full crypto support with hardware tokens |

---

## Configuration

The module uses libipriv as its cryptographic backend. Ensure libipriv is properly installed and configured.

---

## Usage / API highlights

### Key Classes

- `ICryptEngine`: Main cryptographic interface
- `CryptProvider`: Implementation of ICryptEngine
- `CryptEngineFactory`: Factory for creating crypto instances

### Common Operations

```cpp
// Load keys
crypto->loadPrivateKey(path, password);
crypto->loadPublicKey(path);

// Sign/verify
QByteArray sign(const QByteArray& data);
bool verify(const QByteArray& data, const QByteArray& signature);

// Encrypt/decrypt
QByteArray encrypt(const QByteArray& data, const QString& algorithm);
QByteArray decrypt(const QByteArray& data, const QString& algorithm);

// Hash
QByteArray hash(const QByteArray& data, const QString& algorithm);
```

---

## Integration

### CMake

```cmake
find_package(libipriv REQUIRED)
ek_add_library(CryptEngine
    SOURCES ${SOURCES}
    DEPENDS libipriv boost
)
```

### Dependencies

- libipriv: Core cryptographic library
- Boost: For various utilities

---

## Testing

Unit tests are located in `tests/modules/CryptEngine/`. Tests cover:

- Key loading and management
- Signature creation and verification
- Encryption/decryption operations
- Hash function correctness

Run tests with: `ctest -R CryptEngine`

---

## Migration notes

- Ported from TerminalClient project
- Updated to use EKiosk build system and dependencies
- Qt6 compatibility maintained

---

## Further reading

- Implementation details: `src/modules/CryptEngine/README.md`
- API documentation: `include/CryptEngine/` headers</content>
  <parameter name="filePath">c:\Projects\Humo\Kiosk\ekiosk\docs\modules\cryptengine.md

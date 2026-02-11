# CryptEngine Module - Implementation

**For user-facing documentation and API examples, see:** [`docs/modules/cryptengine.md`](../../../docs/modules/cryptengine.md)

This README contains implementation details, design notes, and internal architecture.

---

## Structure

```text
src/modules/CryptEngine/
├── CMakeLists.txt                      # Build configuration
├── README.md                           # This file
├── src/
│   ├── CryptEngine.cpp                 # Main singleton implementation
│   └── CryptEngine.h (private impl)    # Private implementation details
└── tests/
    └── crypt_engine_test.cpp           # Unit tests
```

Public headers are in: [`include/Crypt/`](../../../include/Crypt/)
- `ICryptEngine.h` - Abstract interface
- `CryptEngine.h` - Public implementation header

---

## Key Implementation Details

### Singleton Pattern

```cpp
// Access via singleton instance
ICryptEngine& crypto = CryptEngine::instance();
```

This ensures a single crypto engine per application, with internal state managed safely via static initialization.

### Thread Safety

All public methods use `QMutexLocker` for protection:

```cpp
bool CryptEngine::sign(int aKeyPair, ...) {
    QMutexLocker locker(&m_Mutex);  // Locks on entry, unlocks on exit
    // Safe to access m_KeyPairs
}
```

Enables safe concurrent calls from multiple threads.

### Key Pair Storage

```cpp
typedef QPair<IPRIV_KEY, IPRIV_KEY> TKeyPair;      // (Private, Public) pair
typedef QMap<int, TKeyPair> TKeyPairList;          // ID -> Keys mapping

private:
    TKeyPairList m_KeyPairs;                       // Loaded key pairs
    QRecursiveMutex m_Mutex;                       // Thread safety
```

Supports multiple simultaneous key pairs (useful for multi-account scenarios).

### Hash Algorithm Configuration

Compile-time selection in CryptEngine.cpp:

```cpp
#ifdef TC_USE_MD5
    const int IprivHashAlg = IPRIV_ALG_MD5;        // Deprecated
#else
    const int IprivHashAlg = IPRIV_ALG_SHA256;     // Recommended (default)
#endif
```

Affects signature serial numbers:
- **MD5**: `{serial}` (e.g., "12345")
- **SHA256**: `{serial}-sha256` (e.g., "12345-sha256")

### Hardware Token Support

Optional compilation with `TC_USE_TOKEN`:

```cpp
#ifdef TC_USE_TOKEN
    // RuToken smart card support enabled
    // Crypt_Ctrl(IPRIV_ENGINE_PKCS11_RUTOKEN, ...)
#endif
```

---

## Libipriv Integration

All actual cryptographic operations delegate to libipriv C API:

```cpp
// Key generation
Crypt_GenKey(aEngine, keyCard, ..., &privKey, &pubKey, 2048);

// Signing
Crypt_SignEx(data, size, signature, size, &privKey, IprivHashAlg);
Crypt_Sign2Ex(...);  // Detached signature variant

// Verification
Crypt_Verify(signedData, ...);
Crypt_Verify3(data, signature, ...);  // Detached verification

// Encryption/Decryption
Crypt_Encrypt(data, ...);
Crypt_EncryptLong(largeData, ...);     // For data > 256 bytes
Crypt_Decrypt(encrypted, ...);
Crypt_DecryptLong(encrypted, ...);
```

---

## Error Handling Strategy

All operations return:
1. **bool** - Success/failure indicator
2. **QString& aErrorDescription** - Human-readable error message

Example:
```cpp
QString error;
if (!crypto.sign(0, data, signature, error)) {
    // error contains: "key pair not found", "invalid operation", etc.
    qWarning() << "Sign failed:" << error;
}
```

Error messages come from two sources:
- **Internal validation**: "key pair not found", "mutex lock failed"
- **libipriv errors**: Converted via `errorToString(int iprivErrorCode)`

---

## Buffer Management

Allocates fixed-size buffers for operations:

```cpp
const int DecryptMaxBufferSize = 4096;  // Standard operations
const int KeyExportBufferSize = 4096;   // Key export
const int KeySize = 2048;               // RSA key size bits
```

Results are resized to actual size after operation:
```cpp
aSignature.resize(bufferSize);
int result = Crypt_SignEx(...);
aSignature.resize(result);  // Trim to actual size
```

---

## Platform-Specific Notes

### Windows
- Full support including RuToken hardware tokens
- Uses Windows native PKI when available

### Linux
- Functional with file-based keys
- Hardware token support requires additional pkcs11 libraries
- Tested on Ubuntu 20.04+

### macOS
- Functional with file-based keys
- Limited hardware token testing

---

## Known Limitations

1. **RSA key size fixed to 2048 bits** - Can be modified in code but not at runtime
2. **Standard encryption limited to ~256 bytes** - Use `encryptLong()` for larger data
3. **Serial number tracking** - Assumes properly formatted key serial numbers
4. **Single hash algorithm per build** - Cannot switch MD5↔SHA256 at runtime

---

## Dependencies

- **libipriv** (v1.0.2+): Cryptographic backend
- **Qt6::Core**: Threading, containers, string handling
  - QMutex, QMutexLocker
  - QByteArray, QString, QMap, QSet
  - QFile for key I/O
- **Optional: RuToken libraries** (for `TC_USE_TOKEN`)

---

## Build Configuration

### CMakeLists.txt

```cmake
# Standalone library
ek_add_library(CryptEngine
    FOLDER "modules/CryptEngine"
    SOURCES
        src/CryptEngine.cpp
    DEPENDS
        Qt6::Core
        libipriv
)

# Expose interfaces
target_include_directories(CryptEngine
    PUBLIC include
)
```

### Feature Flags

Pass to CMake:
```bash
cmake ... -DTD_USE_MD5=ON        # Use MD5 instead of SHA256
cmake ... -DTC_USE_TOKEN=ON      # Enable RuToken support
```

---

## Testing

Tests in `tests/modules/CryptEngine/` cover:

1. **Key operations**: Load, create, export
2. **Signatures**: Embedded and detached, sign/verify
3. **Encryption**: Standard and long data variants
4. **Error handling**: Missing keys, invalid states
5. **Threading**: Concurrent operations safety
6. **Token support**: If compiled with TC_USE_TOKEN

**Run tests:**
```bash
cmake --build . --target ctest -- -R CryptEngine -V
```

---

## Future Improvements

- [ ] Runtime configurable RSA key sizes
- [ ] Runtime hash algorithm switching
- [ ] Certificate chain validation
- [ ] Key backup/restore functionality
- [ ] Performance optimizations for bulk signing
- [ ] Qt5 compatibility removal (when Qt 6 mandatory)

---

## See Also

- **User documentation**: [`docs/modules/cryptengine.md`](../../../docs/modules/cryptengine.md)
- **Public headers**: [`include/Crypt/`](../../../include/Crypt/)
- **libipriv source**: [`thirdparty/libipriv/`](../../../thirdparty/libipriv/)

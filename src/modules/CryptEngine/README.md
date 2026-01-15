# CryptEngine

This folder contains the module implementation. The full, canonical documentation has been moved to the central docs site:

- See: `../../../docs/modules/cryptengine.md`

## Structure (implementation)

```text
src/modules/CryptEngine/
├── CMakeLists.txt                  # Module build configuration
├── include/CryptEngine/            # Public headers (installed targets)
│   ├── ICryptEngine.h
│   └── ...
├── src/
│   ├── CryptEngine.cpp             # Main implementation
│   ├── CryptProvider.cpp           # Crypto provider implementation
│   └── ...                         # internal helpers, tests, etc.
└── README.md                       # This file (short pointer to canonical docs)
```

**Contributor notes:**

- Keep high-level documentation, examples, and usage in `docs/modules/cryptengine.md`.
- Use this README for implementation notes, file layout, build or testing hints and internal design details.
- For API docs, examples, and user-facing information, see the canonical docs.

## API summary (short)

- **Primary classes:** `ICryptEngine`, `CryptProvider`, `CryptEngineFactory`.
- **Supported algorithms:** RSA, GOST, IDEA, AES, MD5, SHA-1, SHA-256.
- **Key operations:** Load/save keys, sign/verify, encrypt/decrypt, hash.

Keep this summary updated when APIs change so reviewers can quickly spot relevant changes.
| `TokenManager.cpp` | Hardware token support |

## Dependencies

- `libipriv` (thirdparty)
- `App` module
- `Log` module

## Platform Support

| Platform | Status          |
| -------- | --------------- |
| Windows  | ✅ Full support |
| Linux    | 🔬 Experimental |
| macOS    | 🔬 Experimental |

## Configuration

```ini
[Crypto]
KeyPath=./keys/private.key
UseHardwareToken=false
```

## See Also

- [Full documentation](../../../../docs/modules/engines/cryptengine.md)
- [libipriv source](../../../../thirdparty/libipriv/)

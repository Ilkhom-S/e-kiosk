# WebKitBackend Plugin

Legacy WebKit-based web rendering backend for EKiosk applications.

## Purpose

The WebKitBackend plugin provides WebKit-based web rendering capabilities for EKiosk applications. It implements web content display using Qt WebKit, providing compatibility with Qt5 applications.

## Qt Version Compatibility

**Qt5 Only** - This plugin requires Qt5 because:

- Qt WebKit was deprecated and removed in Qt6
- Qt6 uses Qt WebEngine as the modern replacement
- WebKit provides legacy compatibility for existing Qt5 applications

For Qt6 compatibility, use the **WebEngineBackend** plugin instead.

## Quick start 🔧

```cpp
// Plugin is loaded automatically by the kernel
// Access through appropriate web rendering interface

#include <SDK/Web/IWebBackend.h>

// Get web backend from kernel
auto kernel = getKernel();
auto webBackend = kernel->getInterface<SDK::Web::IWebBackend>();

// Load web content
webBackend->loadUrl("https://example.com");
```

## Features

- WebKit-based rendering engine
- Qt5 compatibility
- JavaScript execution
- CSS and HTML support
- Legacy web standards support

## Platform support

- **Windows**: ✅ Full - Qt WebKit fully supported
- **Linux**: ✅ Full - Qt WebKit fully supported
- **macOS**: 🔬 TODO - Qt WebKit support planned

## Configuration

```json
{
  "webKit": {
    "userAgent": "EKiosk/1.0 (Qt5)",
    "javascriptEnabled": true,
    "pluginsEnabled": false,
    "developerExtrasEnabled": false
  }
}
```

## Usage / API highlights

```cpp
// Load web content
webBackend->loadUrl(QUrl("https://example.com"));
webBackend->loadHtml("<html><body>Hello World</body></html>");

// JavaScript interaction
webBackend->runJavaScript("console.log('Hello from EKiosk');");

// Navigation control
webBackend->goBack();
webBackend->goForward();
webBackend->reload();
```

## Integration

The WebKitBackend integrates with:

- **Qt5 WebKit**: Core rendering engine
- **EKiosk Kernel**: Service registration and lifecycle management
- **Qt5 Framework**: Native Qt5 integration

## Testing

```bash
cmake --build build/msvc --target webkit_backend_test
```

## Troubleshooting

### Qt5 Requirement

**Error**: Plugin fails to load on Qt6
**Solution**: Use WebEngineBackend for Qt6 compatibility, or downgrade to Qt5

### WebKit Limitations

**Error**: Modern web features not supported
**Solution**: Consider migrating to WebEngineBackend with Qt6

## Migration notes

- **To Qt6**: Replace with WebEngineBackend plugin
- **API Changes**: Qt WebKit APIs differ from Qt WebEngine
- **Security**: WebKit has known security vulnerabilities; consider upgrading

## Further reading

- [Plugin System Architecture](README.md#plugin-architecture)
- [Qt WebKit Documentation](https://doc.qt.io/qt-5/qwebkit.html)
- [WebEngineBackend](webenginebackend.md) - Qt6 alternative
- [EKiosk Web Interfaces](../../../include/SDK/Web/)

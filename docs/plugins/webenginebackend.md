# WebEngineBackend Plugin

Chromium-based web rendering backend for EKiosk applications.

## Purpose

The WebEngineBackend plugin provides Chromium-based web rendering capabilities for EKiosk applications. It implements web content display using Qt WebEngine, enabling modern web technologies and standards-compliant rendering.

## Qt Version Compatibility

**Qt6 Only** - This plugin requires Qt6 because:

- Qt WebEngine is the modern web rendering engine in Qt6
- Qt5's Qt WebEngine was less stable and had limited features
- Qt6 provides better Chromium integration and security updates

For Qt5 compatibility, use the **WebKitBackend** plugin instead.

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

- Chromium-based rendering engine
- Modern web standards support
- JavaScript execution
- CSS3 and HTML5 support
- Secure web content isolation
- Qt6 integration

## Platform support

- **Windows**: ✅ Full - Qt WebEngine fully supported
- **Linux**: ✅ Full - Qt WebEngine fully supported
- **macOS**: 🔬 TODO - Qt WebEngine support planned

## Configuration

```json
{
  "webEngine": {
    "userAgent": "EKiosk/1.0",
    "javascriptEnabled": true,
    "pluginsEnabled": false,
    "webSecurityEnabled": true
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

The WebEngineBackend integrates with:

- **Qt6 WebEngine**: Core rendering engine
- **EKiosk Kernel**: Service registration and lifecycle management
- **Security Framework**: Web content isolation and sandboxing

## Testing

```bash
cmake --build build/msvc --target webengine_backend_test
```

## Troubleshooting

### Qt6 Requirement

**Error**: Plugin fails to load on Qt5
**Solution**: Use WebKitBackend for Qt5 compatibility, or upgrade to Qt6

### Chromium Dependencies

**Error**: Missing Chromium libraries
**Solution**: Ensure Qt WebEngine is properly installed with all dependencies

## Migration notes

- **From Qt5**: Migrate to WebKitBackend for Qt5, or upgrade to Qt6
- **API Changes**: Qt6 WebEngine has different APIs than Qt5 WebEngine
- **Security**: Enhanced security features in Qt6 WebEngine

## Further reading

- [Plugin System Architecture](README.md#plugin-architecture)
- [Qt WebEngine Documentation](https://doc.qt.io/qt-6/qtwebengine-overview.html)
- [WebKitBackend](webkitbackend.md) - Qt5 alternative
- [EKiosk Web Interfaces](../../../include/SDK/Web/)

# Graphic Backend Plugins

UI rendering backend plugins.

## Available Backends

| Plugin                                   | Technology   | Best For                  |
| ---------------------------------------- | ------------ | ------------------------- |
| [NativeBackend](NativeBackend/)          | Qt Widgets   | Desktop apps              |
| [QmlBackend](QmlBackend/)                | Qt Quick/QML | Touch interfaces          |
| [WebkitBackend](WebkitBackend/)          | Qt WebKit    | Web-based UI (deprecated) |
| [WebEngineBackend](../WebEngineBackend/) | Qt WebEngine | Web-based UI              |

## Backend Selection

Choose based on your needs:

| Feature       | Native | QML    | WebKit (deprecated) | WebEngine |
| ------------- | ------ | ------ | ------------------- | --------- |
| Touch UI      | ⭐⭐   | ⭐⭐⭐ | ⭐⭐⭐              | ⭐⭐⭐    |
| Performance   | ⭐⭐⭐ | ⭐⭐⭐ | ⭐⭐                | ⭐⭐⭐    |
| Customization | ⭐⭐   | ⭐⭐⭐ | ⭐⭐⭐              | ⭐⭐⭐    |
| Animations    | ⭐     | ⭐⭐⭐ | ⭐⭐⭐              | ⭐⭐⭐    |
| Memory        | ⭐⭐⭐ | ⭐⭐   | ⭐                  | ⭐⭐      |

## Creating a Backend

### 1. Implement IGraphicBackend

```cpp
#include <GUISDK/IGraphicBackend.h>

class MyBackend : public IGraphicBackend {
    Q_OBJECT
public:
    bool initialize() override;
    void shutdown() override;

    void displayScreen(const QString& name,
                       const QVariantMap& params) override;
    void hideScreen() override;

    void showWidget(const QString& id) override;
    void hideWidget(const QString& id) override;
    void updateWidget(const QString& id,
                      const QVariantMap& props) override;

signals:
    void userInput(const QString& widgetId,
                   const QVariant& value);
    void screenLoaded(const QString& name);
};
```

### 2. CMakeLists.txt

```cmake
file(GLOB SOURCES src/*.cpp src/*.h)

tc_add_graphic_backend(my_backend
    SOURCES ${SOURCES}
    QT_MODULES Core Widgets
    DEPENDS GUISDK GraphicsEngine
)
```

## NativeBackend

Qt Widgets-based rendering.

```cpp
// Uses QWidget-based screens
class NativeScreen : public QWidget {
    // Standard Qt widgets
    QPushButton* m_button;
    QLabel* m_label;
};
```

**Pros**: Fast, low memory, native look  
**Cons**: Less flexible for custom UI

## QmlBackend

Qt Quick/QML-based rendering.

```qml
// Screens defined in QML
Rectangle {
    Button {
        text: "Pay Now"
        onClicked: backend.buttonClicked("pay")
    }
}
```

**Pros**: Beautiful animations, flexible  
**Cons**: Slightly higher memory

## WebkitBackend

Web-based UI rendering.

```html
<!-- Screens defined in HTML/CSS/JS -->
<div class="screen">
  <button onclick="pay()">Pay Now</button>
</div>
```

**Pros**: Web technologies, designers can work  
**Cons**: Higher resource usage, deprecated

⚠️ **Note**: WebkitBackend is deprecated and only compatible with Qt 5.0-5.5. For Qt 5.6+ and Qt6, use WebEngineBackend instead.

## Platform Support

| Backend          | Windows | Linux | macOS |
| ---------------- | ------- | ----- | ----- |
| NativeBackend    | ✅      | ✅    | ✅    |
| QmlBackend       | ✅      | ✅    | ✅    |
| WebkitBackend    | ⚠️      | ⚠️    | ⚠️    |
| WebEngineBackend | ✅      | ✅    | 🔬    |

## Dependencies

- `GUISDK` - Graphics interfaces
- `GraphicsEngine` - Engine integration
- Qt Widgets/Quick/WebEngine modules

/* @file Графический движок. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QInputEvent>
#include <QtQml/QQmlEngine>
#include <QtQuick/QQuickItem>
#include <QtQuick/QQuickPaintedItem>
#include <QtQuick/QQuickView>
#include <QtQuick/QQuickWindow>
#include <QtQuickWidgets/QQuickWidget>

#include <Common/ILogable.h>

#include <SDK/GUI/IGraphicsBackend.h>
#include <SDK/GUI/IGraphicsEngine.h>
#include <SDK/GUI/IGraphicsHost.h>
#include <SDK/GUI/MessageBoxParams.h>

#include <memory>

namespace GUI {

//---------------------------------------------------------------------------
namespace {
const QString DefaultBackgroundColor = "#003e75";
const int DefaultAlpha = 0xd8;

class ModalBackgroundWidget : public QQuickPaintedItem {
public:
    ModalBackgroundWidget() {
        setFlag(QQuickItem::ItemHasContents);
        setFillColor(ModalBackgroundWidget::getColor(DefaultBackgroundColor));

        // Необходимо для обработки мыши
        setAcceptedMouseButtons(Qt::AllButtons);
    }

public:
    void setColor(const QString &aColor, int aAlpha = DefaultAlpha) {
        setFillColor(ModalBackgroundWidget::getColor(
            aColor.isEmpty() ? DefaultBackgroundColor : aColor, aAlpha));
    }

protected:
    void paint(QPainter *aPainter) override { Q_UNUSED(aPainter) }
    void mousePressEvent(QMouseEvent *aEvent) override { Q_UNUSED(aEvent) }

private:
    QColor getColor(const QString &aColor, int aAlpha = DefaultAlpha) {
        QColor color(aColor);
        color.setAlpha(aAlpha);
        return color;
    }
};

class DebugWidget : public QQuickItem {
public:
    DebugWidget() {
        /*setBrush(QBrush(QColor(255, 0, 255)));
        setPen(QPen(QColor(255, 255, 0)));
        setZValue(10);*/
    }

protected:
    /*virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget *
    widget)
    {
            QQuickItem::paint(painter, option, widget);

            painter->setFont(QFont("Arial", 12));
            painter->drawText(rect(), Qt::AlignCenter, getDebugInfo());
    }*/

public:
    void setPosition(const QPoint &aLeftBottom_Point) {
        // setRect(QRect(aLeftBottom_Point.x(), aLeftBottom_Point.y() - 20, 100, 20));
    }

    void updateMousePos(const QPoint &aPos) {
        m_MousePos = aPos;
        update();
    }

private:
    QString getDebugInfo() const {
        return QString("(%1; %2)").arg(m_MousePos.x()).arg(m_MousePos.y());
    }

private:
    QPoint m_MousePos;
};

} // namespace

//---------------------------------------------------------------------------
/*class KeyboardContext : public QInputContext
{
        Q_OBJECT

public:
        virtual QString identifierName();
        virtual QString language();
        virtual bool isComposing() const;
        virtual void reset();
        virtual bool filterEvent(const QEvent* aEvent);

signals:
        void requestSoftwareInputPanel();
};*/

//---------------------------------------------------------------------------
/// Графический интерфейс (канва + контейнер графических движков).
class GraphicsEngine : public QObject, public SDK::GUI::IGraphicsEngine, private ILogable {
    Q_OBJECT

public:
    GraphicsEngine();
    ~GraphicsEngine();

public:
    /// Инициализация. Вызывается после addContentDirectory() и addEngine(),
    /// инициализирует экран.
    bool initialize(
        int aDisplay, int aWidth, int aHeight, bool aShowCursor, bool aShowDebugInfo = false);

    /// Освобождение ресурсов
    bool finalize();

    /// Показывает пустой экран.
    void start();

    /// Прячет экран
    void pause();

    /// Закрывает все графические элементы, прячет экран.
    void stop();

    enum EWidgetType { All, Qml, Native, Web };

    Q_ENUMS(EWidgetType)

    /// Очистить сцену от виджетов.
    void reset(EWidgetType aType = GraphicsEngine::Qml);

    /// Добавляет каталог с графическими элементами (сразу производится поиск всех описателей и их
    /// парсинг).
    void addContentDirectory(const QString &aDirectory);

    /// Добавляет клавиши, которые желаем обрабатывать отдельно
    void addHandledKeys(const QStringList &aHandledKeyList);

    /// Добавляет графический движок.
    void addBackend(SDK::GUI::IGraphicsBackend *aBackend);

    /// Отображает виджет.
    bool show(const QString &aWidget, const QVariantMap &aParameters);

    /// Отображает popup-виджет (на уровень выше текущего).
    bool showPopup(const QString &aWidget, const QVariantMap &aParameters);

    /// Отображает модальный виджет.
    QVariantMap showModal(const QString &aWidget, const QVariantMap &aParameters);

    /// Скрывает текущий popup или модальный виджет.
    bool hidePopup(const QVariantMap &aParameters = QVariantMap());

    /// Оповещает виджет.
    void notify(const QString &aEvent, const QVariantMap &aParameters);

    /// Оповещение от popup-виджета
    void popupNotify(const QString &aEvent, const QVariantMap &aParameters);

    /// Установить графический контейнер.
    void setGraphicsHost(SDK::GUI::IGraphicsHost *aHost);

    /// IGraphicsEngine: Возвращает указатель на владельца графического контейнера.
    virtual SDK::GUI::IGraphicsHost *getGraphicsHost();

    /// IGraphicsEngine: Возвращает лог.
    virtual ILog *getLog() const;

    /// Возвращает разрешение дисплея
    QRect getDisplayRectangle(int aIndex) const;

    /// Получить снимок view
    QPixmap getScreenshot();

private slots:
    void onRequestSoftwareInputPanel();
    void setFrontWidget(QQuickItem *aNewWidget, QQuickItem *aOldWidget, int aLevel, bool aPopup);

signals:
    /// Сигнал об активности пользователя.
    void userActivity();

    /// Сигнал об нездоровой активности пользователя.
    void intruderActivity();

    /// Срабатывает при закрытии виджета, на котором рисует движок.
    void closed();

    void keyPressed(QString aText);

private: // Методы
         // Перехватываем события активности пользователя и закрытия окна.
    virtual bool eventFilter(QObject *aObject, QEvent *aEvent);
    bool showWidget(const QString &aWidget, bool aPopup, const QVariantMap &aParameters);

    // Показать/спрятать виртуальную клавиатуру
    void showVirtualKeyboard();
    void hideVirtualKeyboard();

private: // Типы
         /// Параметры дисплея.
    struct SScreen {
        int number;      /// Номер дисплея.
        bool isPrimary;  /// Флаг для главного дисплея.
        QRect geometry;  /// Геометрия.
        QWidget *widget; /// Виджет, соответствующий данному дисплею.
    };

    /// Список графических элементов.
    struct SWidget {
        SDK::GUI::GraphicsItem_Info info;
        std::weak_ptr<SDK::GUI::IGraphicsItem> graphics;
        /// QPointer автоматически обнуляется при уничтожении item Qt через parent-child механизм
        QPointer<QQuickItem> cachedItem;
    };

    typedef QMultiMap<QString, SWidget> TWidgetList;

private: // Данные
         // Интерфейс приложения.
    SDK::GUI::IGraphicsHost *m_Host;

    /// Список доступных экранов.
    QMap<int, SScreen> m_Screens;

    // Родительское окно приложения
    QWidget *m_RootView;

    QWidget *m_QuickContainer;

    // Контейнер для отображения QtQuick сцен
    QQuickWindow *m_QuickView;

    // Список каталогов с контентом.
    QStringList m_ContentDirectories;

    QStringList m_HandledKeyList;

    // Список доступных бэкэндов.
    QMap<QString, SDK::GUI::IGraphicsBackend *> m_Backends;

    // Список доступных виджетов.
    TWidgetList m_Widgets;

    QQmlEngine m_Engine;

    // Текущий виджет.
    TWidgetList::Iterator m_TopWidget;
    // Popup виджет
    TWidgetList::Iterator m_PopupWidget;

    bool m_ShowingModal;
    ModalBackgroundWidget m_ModalBackgroundWidget;
    DebugWidget m_DebugWidget;
    QVariantMap m_ModalResult;
    bool m_IsVirtualKeyboardVisible;
};

} // namespace GUI

//---------------------------------------------------------------------------

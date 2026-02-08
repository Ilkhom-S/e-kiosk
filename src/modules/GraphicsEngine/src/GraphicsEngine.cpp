/* @file Графический интерфейс. */

#include "GraphicsEngine.h"

#include <QtCore/QDebug>
#include <QtCore/QDirIterator>
#include <QtCore/QJsonDocument>
#include <QtCore/QSettings>
#include <QtGui/QGuiApplication>
#include <QtGui/QKeyEvent>
#include <QtGui/QKeySequence>
#include <QtGui/QRegion>
#include <QtGui/QWindow>
#include <QtQml/QQmlProperty>
#include <QtQuick/QQuickItem>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsProxyWidget>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

namespace GUI {

//---------------------------------------------------------------------------
namespace CGraphicsEngine {
const char DescriptorFileSuffix[] = "ini";

const char PopupClosedReason[] = "popup_closed";
const char SectionItem_Name[] = "graphics_item";
const char NameKey[] = "graphics_item/name";
const char TypeKey[] = "graphics_item/type";
const char SectionContextName[] = "context";
const char VirtualKeyboardKey[] = "useVirtualKeyboard";

const QString InputContextName = "VirtualKeyboard";
const QString DefaultLanguage = "en_US";

const int IntruderThreshold = 3;
} // namespace CGraphicsEngine

//--------------------------------------------------------------------------
/*QString KeyboardContext::identifierName()
{
        return CGraphicsEngine::InputContextName;
}

//--------------------------------------------------------------------------
QString KeyboardContext::language()
{
        return CGraphicsEngine::DefaultLanguage;
}

//--------------------------------------------------------------------------
bool KeyboardContext::isComposing() const
{
        return false;
}

//--------------------------------------------------------------------------
void KeyboardContext::reset()
{
}

//--------------------------------------------------------------------------
bool KeyboardContext::filterEvent(const QEvent* aEvent)
{
        if (aEvent->type() == QEvent::RequestSoftwareInputPanel)
        {
                emit requestSoftwareInputPanel();
                return true;
        }

        return false;
}
*/
//---------------------------------------------------------------------------
GraphicsEngine::GraphicsEngine()
    : ILogable("Interface"), m_ShowingModal(false), m_Host(nullptr),
      m_IsVirtualKeyboardVisible(false), m_QuickView(nullptr), m_RootView(nullptr),
      m_QuickContainer(nullptr) {
    QList<QScreen *> screens = QGuiApplication::screens();

    for (int i = 0, index = 1; i < screens.size(); ++i) {
        SScreen screen;
        screen.number = i;
        screen.isPrimary = screens[i] == QGuiApplication::primaryScreen();
        screen.geometry = screens[i]->geometry();
        screen.widget = nullptr;

        m_Screens[screen.isPrimary ? 0 : index++] = screen;
    }

    toLog(LogLevel::Normal, QString("Found are %1 displays.").arg(m_Screens.count()));

    // Наилучшие для QML настройки графической оптимизации (описаны в документации для
    // QDeclarativeView)
    /*m_View->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing |
    QGraphicsView::DontSavePainterState);
    m_View->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    m_View->setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
    */

    // m_View->setResizeMode(QQuickView::SizeRootObjectToView);

    m_RootView = new QWidget;

    Qt::WindowFlags flags = m_RootView->windowFlags();
    flags = flags & Qt::MSWindowsFixedSizeDialogHint & Qt::CustomizeWindowHint &
            Qt::FramelessWindowHint;
    m_RootView->setWindowFlags(flags);

    m_RootView->setFocusPolicy(Qt::StrongFocus);
    m_RootView->setMouseTracking(true);

    // для перехвата Alt+F4
    m_RootView->installEventFilter(this);
}

//---------------------------------------------------------------------------
GraphicsEngine::~GraphicsEngine() {}

//---------------------------------------------------------------------------
bool GraphicsEngine::initialize(
    int aDisplay, int aWidth, int aHeight, bool aShowCursor, bool aShowDebugInfo) {
    // Если задан отсутствующий в системе дисплей, откатываемся на нулевой
    if (aDisplay < 0 || aDisplay > m_Screens.size() - 1) {
        aDisplay = 0;
    }

    toLog(LogLevel::Debug,
          QString("Set primary display number: %1; width: %2, height: %3")
              .arg(aDisplay)
              .arg(aWidth)
              .arg(aHeight));

#ifdef _DEBUG
    m_RootView->setGeometry(
        m_Screens[aDisplay].geometry.left(), m_Screens[aDisplay].geometry.top(), 1280, 1024);
#else
    m_RootView->setGeometry(m_Screens[aDisplay].geometry);
#endif

    m_QuickView = new QQuickWindow;

    m_QuickContainer = QWidget::createWindowContainer(m_QuickView, m_RootView);
    m_QuickContainer->setMinimum_Size(m_RootView->size());

    /*
    m_DebugWidget.setPosition(QPoint(0, aHeight));
    m_Scene.addItem(&m_DebugWidget);
    m_DebugWidget.setVisible(aShowDebugInfo);
    */

    // Настраиваем курсор
    if (!aShowCursor) {
        qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
    }

    m_ModalBackgroundWidget.setZ(2);
    m_ModalBackgroundWidget.setVisible(false);
    m_ModalBackgroundWidget.setSize(QSizeF(aWidth, aHeight));
    m_ModalBackgroundWidget.setParentItem(m_QuickView->contentItem());

    // Настраиваем курсор
    if (!aShowCursor) {
        qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
    }

    // Хост должен быть задан до вызова initialize.
    // Q_ASSERT(m_Host != 0);

    m_TopWidget = m_Widgets.end();
    m_PopupWidget = m_Widgets.end();

    foreach (const SWidget &widget, m_Widgets) {
        if (!m_Backends.contains(widget.info.type)) {
            toLog(LogLevel::Error,
                  QString("Cannot find '%1' backend for '%2' widget.")
                      .arg(widget.info.type)
                      .arg(widget.info.name));
            return false;
        }
    }

    return !m_Widgets.isEmpty();
}

//---------------------------------------------------------------------------
bool GraphicsEngine::finalize() {
    foreach (const SWidget &widget, m_Widgets) {
        if (!widget.graphics.expired() && widget.graphics.lock()->isValid() &&
            widget.graphics.lock()->getWidget()->window() == m_QuickView) {
            widget.graphics.lock()->getWidget()->setParentItem(nullptr);
        }
    }

    m_Widgets.clear();
    m_PopupWidget = m_Widgets.end();
    m_ContentDirectories.clear();
    m_IsVirtualKeyboardVisible = false;

    return true;
}

//---------------------------------------------------------------------------
void GraphicsEngine::start() {
#ifndef _DEBUG
    m_RootView->setWindowFlags(m_RootView->windowFlags() | Qt::WindowStaysOnTopHint);
    m_RootView->showFullScreen();
#endif

    m_RootView->show();
}

//---------------------------------------------------------------------------
void GraphicsEngine::pause() {
    m_RootView->hide();
}

//---------------------------------------------------------------------------
void GraphicsEngine::stop() {
    m_RootView->hide();

    foreach (SWidget widget, m_Widgets.values()) {
        if (!widget.graphics.expired() && widget.graphics.lock()->isValid()) {
            widget.graphics.lock()->getWidget()->setVisible(false);
            widget.graphics.lock()->getWidget()->setZ(0);
        }
    }

    m_TopWidget = m_Widgets.end();
    m_PopupWidget = m_Widgets.end();
    m_ModalBackgroundWidget.setVisible(false);
}

//---------------------------------------------------------------------------
void GraphicsEngine::reset(EWidgetType aType) {
    auto getWidgetType = [](GraphicsEngine::EWidgetType aType) -> QString {
        const QMetaObject metaObject = GraphicsEngine::staticMetaObject;
        int enum_Index = metaObject.indexOfEnumerator("EWidgetType");
        if (enum_Index == -1) {
            return "";
        }

        QMetaEnum en = metaObject.enumerator(enum_Index);
        return QString(en.valueToKey(aType));
    };

    QString widgetType = getWidgetType(aType).toLower();

    foreach (const SWidget &widget, m_Widgets) {
        if (widget.info.type != widgetType && aType != GraphicsEngine::All) {
            continue;
        }

        if (!widget.graphics.expired() && widget.graphics.lock()->isValid() &&
            widget.graphics.lock()->getWidget()->window() == m_QuickView) {
            widget.graphics.lock()->getWidget()->setParentItem(nullptr);
            m_Backends[widget.info.type]->removeItem(widget.info);
            toLog(LogLevel::Debug, QString("REMOVE widget '%1'").arg(widget.info.name));
        }
    }

    m_TopWidget = m_Widgets.end();
    m_PopupWidget = m_Widgets.end();
    m_ModalBackgroundWidget.setVisible(false);
}

//---------------------------------------------------------------------------
void GraphicsEngine::addContentDirectory(const QString &aDirectory) {
    m_ContentDirectories << aDirectory;

    bool from_Resources = false;
    if (aDirectory.left(2) == ":/") {
        from_Resources = true;
    }

    // Ищем в данном каталоге рекурсивно описатели графических элементов и правила сценариев
    QDirIterator entry(aDirectory,
                       QStringList() << QString("*.%1").arg(CGraphicsEngine::DescriptorFileSuffix),
                       QDir::Files,
                       QDirIterator::Subdirectories);

    while (entry.hasNext()) {
        entry.next();

        QSettings file(entry.fileInfo().absoluteFilePath(), QSettings::IniFormat);

        if (file.childGroups().contains(CGraphicsEngine::SectionItem_Name)) {
            SWidget widget;

            widget.info.name = file.value(CGraphicsEngine::NameKey).toString();
            widget.info.type = file.value(CGraphicsEngine::TypeKey).toString();

            // для контента из ресурсов добавляем специальный префикс
            widget.info.directory = (from_Resources ? "qrc" : "") + entry.fileInfo().absolutePath();

            // Описатель должен содержать как минимум имя и тип
            if (widget.info.name.isEmpty() || widget.info.type.isEmpty()) {
                toLog(
                    LogLevel::Warning,
                    QString("Skipping %1: item name or type not specified.").arg(entry.filePath()));
            } else {
                // Читаем специфичные для движка параметры
                file.beginGroup(widget.info.type);
                foreach (QString key, file.childKeys()) {
                    widget.info.parameters[key] = file.value(key).toString();
                }
                file.endGroup();

                // Читаем специфичные для виджета параметры
                file.beginGroup(CGraphicsEngine::SectionContextName);
                foreach (QString key, file.childKeys()) {
                    widget.info.context[key] = file.value(key).toString();
                }
                file.endGroup();

                // Добавляем элемент в список доступных
                m_Widgets.insert(widget.info.name, widget);

                toLog(LogLevel::Normal, QString("Added graphics item '%1'.").arg(entry.filePath()));
            }
        }
    }
}

//---------------------------------------------------------------------------
void GraphicsEngine::addHandledKeys(const QStringList &aHandledKeyList) {
    m_HandledKeyList = aHandledKeyList;
}

//---------------------------------------------------------------------------
void GraphicsEngine::addBackend(SDK::GUI::IGraphicsBackend *aBackend) {
    m_Backends[aBackend->getType()] = aBackend;

    if (aBackend->getType() == "native") {
        foreach (SDK::GUI::GraphicsItem_Info info, aBackend->getItem_List()) {
            // Для "нативных" виджетов необходимо обновить параметр directory
            TWidgetList::Iterator widget = m_Widgets.find(info.name);
            if (widget != m_Widgets.end()) {
                widget->info.directory = info.directory;
            } else {
                SWidget widget;
                widget.info = info;
                m_Widgets.insert(widget.info.name, widget);
            }
        }
    }
}

//------------------------------------------------------------------------------
ILog *GraphicsEngine::getLog() const {
    return ILogable::getLog();
}

//---------------------------------------------------------------------------
QRect GraphicsEngine::getDisplayRectangle(int aIndex) const {
    return m_Screens.value(aIndex).geometry;
}

//---------------------------------------------------------------------------
QPixmap GraphicsEngine::getScreenshot() {
    QPixmap screenshot(m_RootView->size());
    m_RootView->render(&screenshot, QPoint(), QRegion(m_RootView->geometry()));
    return screenshot;
}

//---------------------------------------------------------------------------
void GraphicsEngine::onRequestSoftwareInputPanel() {
    if (m_TopWidget->info.parameters[CGraphicsEngine::VirtualKeyboardKey] == "true") {
        showVirtualKeyboard();
    }
}

//---------------------------------------------------------------------------
bool GraphicsEngine::show(const QString &aWidget, const QVariantMap &aParameters) {
    return showWidget(aWidget, false, aParameters);
}

//---------------------------------------------------------------------------
bool GraphicsEngine::showPopup(const QString &aWidget, const QVariantMap &aParameters) {
    return showWidget(aWidget, true, aParameters);
}

//---------------------------------------------------------------------------
QVariantMap GraphicsEngine::showModal(const QString &aWidget, const QVariantMap &aParameters) {
    if (showPopup(aWidget, aParameters)) {
        m_ShowingModal = true;

        while (m_ShowingModal) {
            QApplication::processEvents(QEventLoop::WaitForMoreEvents);
        }
    }

    return m_ModalResult;
}

//---------------------------------------------------------------------------
bool GraphicsEngine::hidePopup(const QVariantMap &aParameters) {
    if (m_PopupWidget == m_Widgets.end()) {
        return false;
    }

    m_ModalResult = aParameters;
    m_ModalBackgroundWidget.setVisible(false);

    m_PopupWidget->graphics.lock()->getWidget()->setVisible(false);
    m_PopupWidget->graphics.lock()->getWidget()->setScale(1.0);
    m_PopupWidget->graphics.lock()->hide();
    m_PopupWidget = m_Widgets.end();

    if (m_TopWidget != m_Widgets.end()) {
        m_TopWidget->graphics.lock()->notify(CGraphicsEngine::PopupClosedReason, aParameters);
    }

    m_ShowingModal = false;

    return true;
}

//---------------------------------------------------------------------------
void GraphicsEngine::notify(const QString &aEvent, const QVariantMap &aParameters) {
    auto formatParams = [](const QVariantMap &aParameters) -> QString {
        return QString::from_Utf8(
            QJsonDocument::from_Variant(aParameters).toJson(QJsonDocument::Compact));
    };

    TWidgetList::Iterator w = m_Widgets.end();

    if (m_PopupWidget != m_Widgets.end()) {
        w = m_PopupWidget;
        m_PopupWidget->graphics.lock()->notify(aEvent, aParameters);
    } else if (m_TopWidget != m_Widgets.end()) {
        w = m_TopWidget;
        m_TopWidget->graphics.lock()->notify(aEvent, aParameters);
    }

    toLog(LogLevel::Debug,
          QString("NOTIFY '%1'. Parameters: %2").arg(w->info.name).arg(formatParams(aParameters)));

    if (w != m_Widgets.end()) {
        toLog(LogLevel::Debug,
              QString("NOTIFY '%1'. Parameters: %2")
                  .arg(w->info.name)
                  .arg(formatParams(aParameters)));
    } else {
        toLog(LogLevel::Debug,
              QString("NOTIFY WIDGET NOT FOUND. Parameters: %1").arg(formatParams(aParameters)));
    }

    // toLog(LogLevel::Normal, QString("NOTIFY '%1'. Parameters:
    // %2").arg(w->info.name).arg(formatParams(aParameters)));
}

//---------------------------------------------------------------------------
void GraphicsEngine::popupNotify(const QString &aEvent, const QVariantMap &aParameters) {
    if (m_TopWidget != m_Widgets.end()) {
        // Оповещаем виджет, который находится под всплывающим окном
        m_TopWidget->graphics.lock()->notify(aEvent, aParameters);

        toLog(LogLevel::Normal,
              QString("NOTIFY_BY_POPUP '%1'. Parameters: %2")
                  .arg(m_TopWidget->info.name)
                  .arg(QString::from_Utf8(
                      QJsonDocument::from_Variant(aParameters).toJson(QJsonDocument::Compact))));
    }
}

//---------------------------------------------------------------------------
bool GraphicsEngine::showWidget(const QString &aWidget,
                                bool aPopup,
                                const QVariantMap &aParameters) {
    // Нативная клавиатура может остаться от предыдущего виджета. Закроем ее
    if (m_IsVirtualKeyboardVisible) {
        hideVirtualKeyboard();
    }

    // Если находимся в модальном режиме, то обычный show не отрабатываем
    if (m_ShowingModal) {
        toLog(LogLevel::Error,
              QString("Cannot show '%1': already showing a modal widget.").arg(aWidget));
        return false;
    }

    TWidgetList::Iterator newWidget = m_Widgets.find(aWidget);
    TWidgetList::Iterator oldWidget = m_TopWidget;

    // Проверяем существует ли такой виджет
    if (newWidget == m_Widgets.end()) {
        toLog(LogLevel::Error, QString("Cannot show '%1': not found.").arg(aWidget));
        return false;
    }

    // Если виджетов с одинаковым именем больше одного
    // Ищем виджет с таким же контекстом. Если не нашли, то оставляем первый c пустым контекстом
    if (m_Widgets.values(aWidget).count() > 1) {
        TWidgetList::Iterator widgetWithContext;
        TWidgetList::Iterator firstWidget = newWidget;

        for (widgetWithContext = firstWidget; widgetWithContext != m_Widgets.end();
             ++widgetWithContext) {
            if (widgetWithContext->info.name == aWidget &&
                widgetWithContext->info.context.isEmpty()) {
                newWidget = widgetWithContext;
                break;
            }
        }

        foreach (QString param_Key, aParameters.keys()) {
            for (widgetWithContext = firstWidget; widgetWithContext != m_Widgets.end();
                 ++widgetWithContext) {
                if (widgetWithContext->info.name != aWidget) {
                    continue;
                }

                foreach (QString contextKey, widgetWithContext->info.context.keys()) {
                    if (contextKey == param_Key) {
                        if (widgetWithContext->info.context[param_Key]
                                .toString()
                                .split("|")
                                .contains(aParameters[param_Key].toString())) {
                            newWidget = widgetWithContext;
                        }
                    }
                }
            }
        }
    }

    // Если ещё не создан - создаём
    if (newWidget->graphics.expired()) {
        newWidget->graphics = m_Backends[newWidget->info.type]->getItem(newWidget->info);
    }

    if (!newWidget->graphics.expired() && newWidget->graphics.lock()->isValid()) {
        if (newWidget->graphics.lock()->getWidget() &&
            !newWidget->graphics.lock()->getWidget()->window()) {
            newWidget->graphics.lock()->getWidget()->setParentItem(m_QuickView->contentItem());
        } else if (QWidget *w = newWidget->graphics.lock()->getNativeWidget()) {
        }
    } else {
        toLog(LogLevel::Error, QString("Failed to instantiate widget '%1'.").arg(aWidget));
        return false;
    }

    // По умолчанию показываем на слое для обычных виджетов
    int level = 1;

    // Если отображается всплывающее окно, то закрываем его
    if (m_PopupWidget != m_Widgets.end() && !aPopup) {
        hidePopup(QVariantMap());
    }

    // Если нужно, инициализируем новый виджет
    if (aParameters.contains("reset") ? aParameters["reset"].toBool() : true) {
        newWidget->graphics.lock()->reset(aParameters);
    }

    // Если виджет уже отображается, то только вызываем обработок showHandler
    if (newWidget == m_TopWidget) {
        m_TopWidget->graphics.lock()->show();
        return true;
    }

    // Если всплывающий или модальный виджет, то маска и всплывающее окно выводятся уровнем выше
    // текущего виджета
    if (aPopup) {
        // Масштабируем, если надо, всплывающее окно
        if (aParameters["scaled"].toBool()) {
            QRectF rect = newWidget->graphics.lock()->getWidget()->boundingRect();
            qreal scale =
                qMin(m_QuickView->width() / rect.width(), m_QuickView->height() / rect.height());
            newWidget->graphics.lock()->getWidget()->setScale(scale);
        }

        // Установим цвет фона модального окна
        QString popupOverlayColor = aParameters["popup_overlay_color"].toString();
        if (!popupOverlayColor.isEmpty()) {
            m_ModalBackgroundWidget.setColor(popupOverlayColor);
        }

        // Показываем маску
        m_ModalBackgroundWidget.setVisible(true);

        // Показываем всплывающее окно
        level = 3;
        m_PopupWidget = newWidget;
    } else {
        if (m_TopWidget != m_Widgets.end()) {
            oldWidget = m_TopWidget;
            oldWidget->graphics.lock()->hide();
        }

        m_TopWidget = newWidget;
    }

    if (!aPopup && newWidget->graphics.expired()) {
        return false;
    }

    newWidget->graphics.lock()->show();

    if (newWidget->info.type == "native") {
        m_QuickContainer->setVisible(false);

        QWidget *w = newWidget->graphics.lock()->getNativeWidget();
        w->setParent(m_RootView);
        w->setVisible(true);
    } else {
        m_QuickContainer->setVisible(true);

        // Отрисовка через очередь, чтобы showHandler/resetHandler успели отработать
        QMetaObject::invokeMethod(
            this,
            "setFrontWidget",
            Qt::QueuedConnection,
            Q_ARG(QQuickItem *,
                  dynamic_cast<QQuickItem *>(newWidget->graphics.lock()->getWidget())),
            Q_ARG(QQuickItem *,
                  (aPopup || oldWidget == m_Widgets.end())
                      ? 0
                      : dynamic_cast<QQuickItem *>(oldWidget->graphics.lock()->getWidget())),
            Q_ARG(int, level),
            Q_ARG(bool, aPopup));

        QString popupMessage =
            aPopup ? QString::from_Utf8(
                         QJsonDocument::from_Variant(aParameters).toJson(QJsonDocument::Compact))
                   : QString();

        aPopup
            ? toLog(LogLevel::Normal,
                    QString("SHOW POPUP '%1'. Parameters: %2").arg(aWidget).arg(popupMessage))
            : toLog(
                  LogLevel::Normal,
                  QString("SHOW '%1' scene. %2")
                      .arg(aWidget)
                      .arg(newWidget->info.context.isEmpty()
                               ? ""
                               : QString("CONTEXT: %1")
                                     .arg(QStringList(newWidget->info.context.keys()).join(";"))));
    }

    return true;
}

//---------------------------------------------------------------------------
void GraphicsEngine::setFrontWidget(QQuickItem *aNewWidget,
                                    QQuickItem *aOldWidget,
                                    int aLevel,
                                    bool aPopup) {
    // Всплывающее окно уже могли спрятать
    if (aPopup && m_PopupWidget == m_Widgets.end()) {
        return;
    }

    aNewWidget->setVisible(true);
    aNewWidget->setZ(aLevel);

    if (aOldWidget) {
        aOldWidget->setVisible(false);
        aOldWidget->setZ(0);
    }

    if (m_PopupWidget != m_Widgets.end()) {
        QRectF rect(aNewWidget->boundingRect());
        qreal scale = qobject_cast<QQuickItem *>(aNewWidget)->scale();
        aNewWidget->setPosition(QPointF((m_QuickView->width() - rect.width() / scale) / 2,
                                        (m_QuickView->height() - rect.height() / scale) / 2));
    }
}

//---------------------------------------------------------------------------
void GraphicsEngine::showVirtualKeyboard() {
    /*QGraphicsItem * focusItem = m_TopWidget->graphics->getWidget();
    QGraphicsItem * focusItem = m_TopWidget->graphics.lock()->getWidget();
    QQuickItem * focusItem = m_TopWidget->graphics->getWidget();
    QWidget * focusWidget = static_cast<QGraphicsProxyWidget *>(focusItem)->widget()->focusWidget();

    if (focusWidget == nullptr)
    {
            return;
    }

    // Проверяем существует ли такой виджет
    TWidgetList::Iterator widget = m_Widgets.find(CGraphicsEngine::InputContextName);

    if (widget == m_Widgets.end())
    {
            toLog(LogLevel::Error, QString("Cannot show '%1': not
    found.").arg(CGraphicsEngine::InputContextName)); return;
    }

    // Если ещё не создан - создаём
    if (widget->graphics.expired())
    {
            widget->graphics = m_Backends[widget->info.type]->getItem(widget->info);
    }

    if (!widget->graphics.expired() && widget->graphics.lock()->isValid())
    {
            if (!widget->graphics.lock()->getWidget()->scene())
            {
                    m_Scene.addItem(widget->graphics.lock()->getWidget());
            }
    }

    // Формируем список всех возможных положений виртуальной клавиатуры
    QGraphicsItem * keyboardItem = widget->graphics.lock()->getWidget();
    qreal focusItem_Scale = focusItem->scale();

    QRectF sceneRect(m_Scene.sceneRect());
    QRectF keyboardRect(keyboardItem->sceneBoundingRect());
    QPointF newPos((sceneRect.width() - keyboardRect.width()) / 2, 0.0f);

    QList<QRectF> keyboardPositions;

    keyboardRect.moveBottom_Left(focusWidget->mapToGlobal(focusWidget->rect().topLeft()) *
    focusItem_Scale); keyboardPositions << keyboardRect;

    keyboardRect.moveBottom_Right(focusWidget->mapToGlobal(focusWidget->rect().topRight()) *
    focusItem_Scale); keyboardPositions << keyboardRect;

    keyboardRect.moveTopLeft(focusWidget->mapToGlobal(focusWidget->rect().bottom_Left()) *
    focusItem_Scale); keyboardPositions << keyboardRect;

    keyboardRect.moveTopRight(focusWidget->mapToGlobal(focusWidget->rect().bottom_Right()) *
    focusItem_Scale); keyboardPositions << keyboardRect;

    keyboardRect.moveTopLeft(QPointF(newPos.x(),
    focusWidget->mapToGlobal(focusWidget->rect().bottom_Left()).y() * focusItem_Scale));
    keyboardPositions << keyboardRect;

    keyboardRect.moveBottom_Left(QPointF(newPos.x(),
    focusWidget->mapToGlobal(focusWidget->rect().topLeft()).y() * focusItem_Scale));
    keyboardPositions << keyboardRect;

    foreach(QRectF r, keyboardPositions)
    {
            if (sceneRect.contains(r))
            {
                    newPos.setX(r.x());
                    newPos.setY(r.y());
                    break;
            }
    }

    keyboardItem->setPos(newPos.x(), newPos.y());

    widget->graphics.lock()->getWidget()->setZValue(3);
    widget->graphics.lock()->getWidget()->setVisible(true);

    m_IsVirtualKeyboardVisible = true;*/
}

//---------------------------------------------------------------------------
void GraphicsEngine::hideVirtualKeyboard() {
    /*TWidgetList::Iterator widget = m_Widgets.find(CGraphicsEngine::InputContextName);
    QGraphicsItem * item = widget->graphics.lock()->getWidget();
    item->setVisible(false);

    m_IsVirtualKeyboardVisible = false;*/
}

//---------------------------------------------------------------------------
bool GraphicsEngine::eventFilter(QObject *aObject, QEvent *aEvent) {
    QEvent::Type type = aEvent->type();
    static int intruder;

    switch (type) {
    case QEvent::MouseMove: {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(aEvent);
        m_DebugWidget.updateMousePos(mouseEvent->globalPosition().toPoint());
        if (mouseEvent->buttons() != Qt::LeftButton &&
            ++intruder >= CGraphicsEngine::IntruderThreshold) {
            emit intruderActivity();
        }
    } break;

    case QEvent::Wheel:
        emit intruderActivity();
        break;

    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
        if (m_IsVirtualKeyboardVisible) {
            /*auto * mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(aEvent);
            TWidgetList::Iterator widget = m_Widgets.find(CGraphicsEngine::InputContextName);
            QGraphicsItem * keyboardItem = widget->graphics.lock()->getWidget();
            QRectF keyboardRect(keyboardItem->sceneBoundingRect());

            if (!keyboardRect.contains(mouseEvent->scenePos()))
            {
                    hideVirtualKeyboard();
            }*/
        }
    case QEvent::MouseButtonRelease:
        emit userActivity();
        intruder = 0;
        break;

    case QEvent::KeyPress: {
        if (!m_HandledKeyList.isEmpty()) {
            auto *keyEvent = static_cast<QKeyEvent *>(aEvent);

            QString key = QKeySequence(keyEvent->key()).toString();
            if (m_HandledKeyList.indexOf(key) != -1) {
                emit keyPressed(key);
                return true;
            }
        }
    } break;

    case QEvent::Close:
        emit closed();
        break;
    }

    return QObject::eventFilter(aObject, aEvent);
}

//---------------------------------------------------------------------------
SDK::GUI::IGraphicsHost *GraphicsEngine::getGraphicsHost() {
    return m_Host;
}

//---------------------------------------------------------------------------
void GraphicsEngine::setGraphicsHost(SDK::GUI::IGraphicsHost *aHost) {
    m_Host = aHost;
}

} // namespace GUI

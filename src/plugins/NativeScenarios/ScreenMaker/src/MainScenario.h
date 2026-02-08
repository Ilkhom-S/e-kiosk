/* @file Плагин сценария для создания скриншотов */

#pragma once

#include <QtCore/QDebug>
#include <QtGui/QClipboard>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QWidget>

#include <ScenarioEngine/Scenario.h>

// Plugin SDK
#include <SDK/GUI/IGraphicsItem.h>
#include <SDK/PaymentProcessor/Core/Event.h>
#include <SDK/PaymentProcessor/Core/ICore.h>
#include <SDK/PaymentProcessor/Core/IEventService.h>
#include <SDK/Plugins/IFactory.h>
#include <SDK/Plugins/IPlugin.h>

class DrawAreaWindow : public QWidget {
    Q_OBJECT

public:
    DrawAreaWindow(SDK::PaymentProcessor::ICore *aCore, const QRect &aRect)
        : m_Core(aCore), m_Pressed(false) {
        // setMinimum_Width(aRect.width()); setMinimum_Height(aRect.height());
        setGeometry(aRect);
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    }

public:
    void updateImage(const QImage &aImage) {
        m_Rects.clear();
        m_Image = aImage;
    }

public:
    virtual void keyPressEvent(QKeyEvent *event) {
        if (event->key() == Qt::Key_Return) {
            QPainter painter;

            QPen pen;
            pen.setColor(QColor(0xff, 0x00, 0x00));
            pen.setWidth(5);

            painter.begin(&m_Image);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            painter.drawRects(m_Rects);
            painter.end();

            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setImage(m_Image);

            m_FilePath =
                QFileDialog::getSaveFileName(this, "Save Scene", m_FilePath, "Image (*.png)");
            m_Image.save(m_FilePath);
        }

        finish();

        QWidget::keyPressEvent(event);
    }

    virtual void mouseMoveEvent(QMouseEvent *aEvent) {
        if (m_Pressed) {
            m_CurrentPos = aEvent->pos();
            // update();
        }
    }

    virtual void mousePressEvent(QMouseEvent *aEvent) {
        if (aEvent->button() == Qt::LeftButton) {
            m_Pressed = true;
            m_StartPos = m_CurrentPos = aEvent->pos();
        } else if (aEvent->button() == Qt::RightButton && m_Rects.count() > 0) {
            m_Rects.remove(m_Rects.count() - 1);
            update();
        }
    }

    virtual void mouseReleaseEvent(QMouseEvent *aEvent) {
        if (aEvent->button() == Qt::LeftButton) {
            m_Pressed = false;
            m_Rects.append(QRect(m_StartPos, aEvent->pos()));
        }
    }

    virtual void paintEvent(QPaintEvent * /*event*/) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, false);

        painter.drawImage(0, 0, m_Image);

        QPen pen;
        pen.setWidth(5);

        painter.setBrush(Qt::NoBrush);

        // border
        pen.setColor(QColor(0x00, 0xff, 0x00));
        painter.setPen(pen);
        painter.drawRect(0, 0, m_Image.width(), m_Image.height());

        // current frame
        pen.setColor(QColor(0xff, 0x00, 0x00));
        painter.setPen(pen);
        painter.drawRects(m_Rects);

        if (m_Pressed) {
            painter.drawRect(QRect(m_StartPos, m_CurrentPos));
        }

        update();
    }

    void closeEvent(QCloseEvent *event) { finishScenario(); }

private:
    void finish() {
        m_Rects.clear();
        finishScenario();
    }

    void finishScenario() {
        QVariantMap params;
        params["signal"] = "finish";

        SDK::PaymentProcessor::Event e(
            SDK::PaymentProcessor::EEventType::UpdateScenario, "", QVariant::from_Value(params));
        m_Core->getEventService()->sendEvent(e);
    }

private:
    QPoint m_StartPos;
    QPoint m_CurrentPos;
    bool m_Pressed;
    QImage m_Image;
    QVector<QRect> m_Rects;
    SDK::PaymentProcessor::ICore *m_Core;
    QString m_FilePath;
};

class IApplication;

namespace SDK {
namespace PaymentProcessor {
class ICore;

namespace Scripting {
class Core;
} // namespace Scripting
} // namespace PaymentProcessor

namespace Plugin {
class IEnvironment;
} // namespace Plugin
} // namespace SDK

namespace ScreenMaker {

//---------------------------------------------------------------------------
class MainScenario : public GUI::Scenario {
    Q_OBJECT

public:
    MainScenario(SDK::PaymentProcessor::ICore *aCore, ILog *aLog);
    virtual ~MainScenario();

public:
    /// Запуск сценария.
    virtual void start(const QVariantMap &aContext);

    /// Остановка сценария.
    virtual void stop();

    /// Приостановить сценарий с возможностью последующего возобновления.
    virtual void pause();

    /// Продолжение после паузы.
    virtual void resume(const QVariantMap &aContext);

    /// Инициализация сценария.
    virtual bool initialize(const QList<GUI::SScriptObject> &aScriptObjects);

    /// Обработка сигнала из активного состояния с дополнительными аргументами.
    virtual void signalTriggered(const QString &aSignal,
                                 const QVariantMap &aArguments = QVariantMap());

    /// Обработчик таймаута
    virtual void onTimeout();

    /// Возвращает false, если сценарий не может быть остановлен в текущий момент.
    virtual bool canStop();

public slots:
    /// Текущее состояние.
    virtual QString getState() const;

private:
    QVariantMap m_Context;
    QString m_LastSignal;
    SDK::PaymentProcessor::ICore *m_Core;
    DrawAreaWindow *m_DrawAreaWindow;
};

} // namespace ScreenMaker

//--------------------------------------------------------------------------

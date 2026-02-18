/**
 * @file Вспомогательный экран, закрывающий рабочий стол.
 */

#include "SplashScreen.h"

#include <QtCore/QDir>
#include <QtCore/QEasingCurve>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSequentialAnimationGroup>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtWidgets/QHBoxLayout>

#include <Common/BasicApplication.h>

#include <SDK/PaymentProcessor/Settings/DealerSettings.h>
#include <SDK/PaymentProcessor/Settings/TerminalSettings.h>

#include <SettingsManager/SettingsManager.h>
#include <algorithm>
#include <memory>

namespace CSplashScreen {
const char DefaultBackgroundStyle[] = "QWidget#wgtBackground { background-color: #f07e1b; }";
const char CustomBackgroundStyle[] = "QWidget#wgtBackground { border-image: url(%1); }";
const char StateImagesPath[] = ":/images/states/";
const char StateImageExtension[] = ".png";
const int MinStateShowSeconds = 2;
} // namespace CSplashScreen

//----------------------------------------------------------------------------
// Прозрачный overlay поверх всех child-виджетов — рисует flash-эффекты.
// Не перехватывает мышиные события (WA_TransparentForMouseEvents).
class FlashOverlay : public QWidget {
public:
    explicit FlashOverlay(SplashScreen *aParent) : QWidget(aParent), mScreen(aParent) {
        setAttribute(Qt::WA_TransparentForMouseEvents);
        setAttribute(Qt::WA_NoSystemBackground);
        setAttribute(Qt::WA_TranslucentBackground);
    }

protected:
    void paintEvent(QPaintEvent *) override {
        if (mScreen->m_activeFlashes.isEmpty()) {
            return;
        }

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        for (const auto &flash : mScreen->m_activeFlashes) {
            if (mScreen->m_showAdminFlash) {
                p.fillRect(flash.zoneRect,
                           QColor(0xFA, 0x53, 0x00, int(0.15 * 255 * flash.opacity)));
            }
            if (mScreen->m_showAdminNumbers) {
                QFont f;
                f.setPixelSize(160);
                f.setBold(true);
                p.setFont(f);
                p.setPen(QColor(255, 255, 255, int(255 * flash.opacity)));
                p.drawText(flash.zoneRect, Qt::AlignCenter, QString::number(flash.zoneNumber));
            }
        }
    }

private:
    SplashScreen *mScreen;
};

//----------------------------------------------------------------------------
// Конструктор экрана заставки
SplashScreen::SplashScreen(const QString &aLog, QWidget *aParent)
    : ILogable(aLog),
#ifdef _DEBUG
      QWidget(aParent)
#else
      QWidget(aParent, Qt::WindowStaysOnTopHint)
#endif
      ,
      m_quitRequested(false), mBurnInProtectionAnim(nullptr), mLayoutOffset(0, 0),
      m_showAdminFlash(true), m_showAdminNumbers(true), m_flashOverlay(nullptr) {
    ui.setupUi(this);

    // Overlay поверх всех child-виджетов — рисует flash-эффекты без блокировки кликов
    m_flashOverlay = new FlashOverlay(this);
    m_flashOverlay->resize(size());
    m_flashOverlay->raise();

    // Flash-таймер: ~60fps, стартует только при наличии активных флешей
    connect(&m_flashTimer, &QTimer::timeout, this, &SplashScreen::onFlashTick);
    m_flashTimer.setInterval(16);

    // FIXME: временно для #18645
    // ui.lblSupportPhone->hide();
    // ui.lblTerminalName->hide();

    QTimer::singleShot(0, this, &SplashScreen::onInit);

    installEventFilter(this);
}

//----------------------------------------------------------------------------
// Деструктор экрана заставки
SplashScreen::~SplashScreen() = default;

//----------------------------------------------------------------------------
// Инициализация экрана заставки
void SplashScreen::onInit() {
    toLog(LogLevel::Normal, "SplashScreen onInit called");

    // Connect после инициализации, чтобы не поймать сигнал до готовности
    connect(qApp, &QApplication::aboutToQuit, this, [this]() {
        toLog(LogLevel::Normal, "aboutToQuit signal received - setting m_quitRequested to true");
        m_quitRequested = true;
    });

    // Настройка защиты от выгорания экрана
    setupBurnInProtection();

    toLog(LogLevel::Normal, "SplashScreen onInit completed");
}

//----------------------------------------------------------------------------
// Разрешить закрытие окна перед вызовом quit()
void SplashScreen::requestQuit() {
    toLog(LogLevel::Normal, "requestQuit called - setting m_quitRequested to true");
    m_quitRequested = true;
    hide();
}

//----------------------------------------------------------------------------
// Обработка события закрытия окна
void SplashScreen::closeEvent(QCloseEvent *aEvent) {
    toLog(LogLevel::Normal,
          QString("Close splash screen by event. m_quitRequested=%1").arg(m_quitRequested));

    if (m_quitRequested) {
        aEvent->accept();
        toLog(LogLevel::Normal, "Quit requested - accepting close event.");
    } else {
        aEvent->ignore();
        showMinimized();
        emit hidden();
        toLog(LogLevel::Normal, "Close ignored - minimizing window instead.");
    }
}

//----------------------------------------------------------------------------
// Подгоняет overlay под актуальный размер окна
void SplashScreen::resizeEvent(QResizeEvent *aEvent) {
    QWidget::resizeEvent(aEvent);
    if (m_flashOverlay) {
        m_flashOverlay->resize(size());
        m_flashOverlay->raise();
    }
}

//----------------------------------------------------------------------------
// Фильтр событий для обработки кликов мыши
bool SplashScreen::eventFilter(QObject *aObject, QEvent *aEvent) {
    if (aEvent->type() == QEvent::MouseButtonPress) {
        auto *mouseEvent = dynamic_cast<QMouseEvent *>(aEvent);
        if (mouseEvent) {
            if (m_areas.isEmpty()) {
                // Размеры виджета до первого показа кривые
                updateAreas();
            }

            TAreas::iterator area =
                std::find_if(m_areas.begin(),
                             m_areas.end(),
                             [this, pos = mouseEvent->pos()](const TAreas::value_type &a) {
                                 return testPoint(a, pos);
                             });

            if (area != m_areas.end()) {
                // Добавляем flash — overlay перерисует поверх child-виджетов
                m_activeFlashes.append({area->first, area->second, 1.0});
                if (!m_flashTimer.isActive()) {
                    m_flashTimer.start();
                }
                m_flashOverlay->update();
                emit clicked(area->first);
            }
        }
    }

    return QWidget::eventFilter(aObject, aEvent);
}

//----------------------------------------------------------------------------
// Проверка попадания точки в область
bool SplashScreen::testPoint(const TAreas::value_type &aArea, const QPoint &aPoint) const {
    return aArea.second.contains(aPoint);
}

//----------------------------------------------------------------------------
// Обновление областей клика на экране
void SplashScreen::updateAreas() {
    int width = rect().width();
    int height = rect().height();

    m_areas << qMakePair(1, QRectF(QPointF(0, 0), QPointF(0.33 * width, 0.33 * height)))
           << qMakePair(2, QRectF(QPointF(0.66 * width, 0), QPointF(width, 0.33 * height)))
           << qMakePair(5,
                        QRectF(QPointF(0.33 * width, 0.33 * height),
                               QPointF(0.66 * width, 0.66 * height)))
           << qMakePair(3, QRectF(QPointF(0, 0.66 * height), QPointF(0.33 * width, height)))
           << qMakePair(4, QRectF(QPointF(0.66 * width, 0.66 * height), QPointF(width, height)));
}

//----------------------------------------------------------------------------
// Таймер-тик: затухание активных flash-эффектов (~300ms при 16ms тике)
void SplashScreen::onFlashTick() {
    const qreal step = 1.0 / (300.0 / 16.0);

    for (auto &flash : m_activeFlashes) {
        flash.opacity -= step;
    }

    m_activeFlashes.erase(std::remove_if(m_activeFlashes.begin(),
                                        m_activeFlashes.end(),
                                        [](const SActiveFlash &f) { return f.opacity <= 0.0; }),
                         m_activeFlashes.end());

    if (m_activeFlashes.isEmpty()) {
        m_flashTimer.stop();
    }

    m_flashOverlay->update();
}

//----------------------------------------------------------------------------
// Установка состояния экрана заставки
void SplashScreen::setState(const QString &aSender, const QString &aState) {
    TStateList::iterator it;

    QString stateCode = aState.left(aState.indexOf('_'));
    QString stateStatus = aState.right(aState.size() - stateCode.size() - 1);

    for (it = m_states.begin(); it != m_states.end(); ++it) {
        QString oldStateCode = it->state.left(it->state.indexOf('_'));
        QString oldStateStatus = it->state.right(it->state.size() - oldStateCode.size() - 1);

        if (stateCode == oldStateCode) {
            if (stateStatus == oldStateStatus) {
                // Уже установлено точно такое же состояние.
                return;
            }

            if (it->date.secsTo(QDateTime::currentDateTime()) <
                CSplashScreen::MinStateShowSeconds) {
                QTimer::singleShot(CSplashScreen::MinStateShowSeconds * 1000,
                                   it->widget,
                                   [widget = it->widget]() { widget->deleteLater(); });
            } else {
                it->widget->deleteLater();
            }

            m_states.erase(it);

            break;
        }
    }

    QString stateImagePath(CSplashScreen::StateImagesPath + aState +
                           CSplashScreen::StateImageExtension);

    if (!QFile::exists(stateImagePath)) {
        return;
    }

    QString css = QString("QWidget { background-image: url(%1); min-width: 48; max-width: 48; "
                          "min-height: 48; max-height: 48; }")
                      .arg(stateImagePath);

    std::unique_ptr<QWidget> widget(new QWidget(this));
    widget->setStyleSheet(css);

    // Вставляем иконку между двумя spacers (центрирование)
    ui.stateLayout->insertWidget(ui.stateLayout->count() - 1, widget.get());

    // Overlay должен оставаться поверх всех child-виджетов
    m_flashOverlay->raise();

    m_states << SState(aSender, aState, widget.release());
}

//----------------------------------------------------------------------------
// Удаление состояний для указанного отправителя
void SplashScreen::removeStates(const QString &aSender) {
    TStateList::iterator it;

    for (it = m_states.begin(); it != m_states.end(); ++it) {
        if (aSender == it->sender) {
            if (it->date.secsTo(QDateTime::currentDateTime()) <
                CSplashScreen::MinStateShowSeconds) {
                QTimer::singleShot(CSplashScreen::MinStateShowSeconds * 1000,
                                   it->widget,
                                   [widget = it->widget]() { widget->deleteLater(); });
            } else {
                it->widget->deleteLater();
            }

            it = m_states.erase(it);

            --it;
        }
    }
}

//----------------------------------------------------------------------------
// Установка пользовательского фона экрана заставки
void SplashScreen::setCustom_Background(const QString &aPath) {
    aPath.isEmpty() ? setStyleSheet(CSplashScreen::DefaultBackgroundStyle)
                    : setStyleSheet(QString(CSplashScreen::CustomBackgroundStyle).arg(aPath));
}

//----------------------------------------------------------------------------
// Setter для layoutOffset - применяет offset к layout margins
void SplashScreen::setLayoutOffset(const QPoint &offset) {
    mLayoutOffset = offset;
    // Применяем offset к левому/верхнему краю для drift-эффекта
    // Центрирование сохраняется благодаря spacers в layout
    ui.mainLayout->setContentsMargins(offset.x(), offset.y(), 0, 0);
}

//----------------------------------------------------------------------------
// Настройка защиты от выгорания экрана (burn-in protection)
// Использует margin animation вместо window position для совместимости с Windows 7
void SplashScreen::setupBurnInProtection() {
    mBurnInProtectionAnim = new QSequentialAnimationGroup(this);

    // Параметры движения: 6px радиус (меньше для margins), 6 минут на цикл
    const int driftRadius = 6;
    const int cycleDuration = 360000; // 6 минут = 360000 мс
    const int stepDuration = cycleDuration / 4;

    auto *anim1 = new QPropertyAnimation(this, "layoutOffset");
    anim1->setDuration(stepDuration);
    anim1->setStartValue(QPoint(0, 0));
    anim1->setEndValue(QPoint(driftRadius, driftRadius));
    anim1->setEasingCurve(QEasingCurve::InOutSine);

    auto *anim2 = new QPropertyAnimation(this, "layoutOffset");
    anim2->setDuration(stepDuration);
    anim2->setStartValue(QPoint(driftRadius, driftRadius));
    anim2->setEndValue(QPoint(-driftRadius, driftRadius));
    anim2->setEasingCurve(QEasingCurve::InOutSine);

    auto *anim3 = new QPropertyAnimation(this, "layoutOffset");
    anim3->setDuration(stepDuration);
    anim3->setStartValue(QPoint(-driftRadius, driftRadius));
    anim3->setEndValue(QPoint(-driftRadius, -driftRadius));
    anim3->setEasingCurve(QEasingCurve::InOutSine);

    auto *anim4 = new QPropertyAnimation(this, "layoutOffset");
    anim4->setDuration(stepDuration);
    anim4->setStartValue(QPoint(-driftRadius, -driftRadius));
    anim4->setEndValue(QPoint(0, 0));
    anim4->setEasingCurve(QEasingCurve::InOutSine);

    mBurnInProtectionAnim->addAnimation(anim1);
    mBurnInProtectionAnim->addAnimation(anim2);
    mBurnInProtectionAnim->addAnimation(anim3);
    mBurnInProtectionAnim->addAnimation(anim4);

    mBurnInProtectionAnim->setLoopCount(-1);
    mBurnInProtectionAnim->start();

    toLog(LogLevel::Normal,
          "Burn-in protection enabled: 6px margin drift, 6min cycle (Windows 7 compatible)");
}

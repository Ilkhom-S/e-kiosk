/**
 * @file Вспомогательный экран, закрывающий рабочий стол.
 */

#include "SplashScreen.h"

#include <QtCore/QDir>
#include <QtCore/QTimer>
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
// Конструктор экрана заставки
SplashScreen::SplashScreen(const QString &aLog, QWidget *aParent)
    : ILogable(aLog),
#ifdef _DEBUG
      QWidget(aParent)
#else
      QWidget(aParent, Qt::WindowStaysOnTopHint)
#endif
      ,
      m_QuitRequested(false) {
    ui.setupUi(this);

    // FIXME: временно для #18645
    // ui.lblSupportPhone->hide();
    // ui.lblTerminalName->hide();

    // Use new Qt5/6 compatible signal/slot syntax
    QTimer::singleShot(0, this, &SplashScreen::onInit);

    installEventFilter(this);

    // Connect to aboutToQuit signal to know when application quit is requested
    // Moved to onInit to avoid premature quit detection during startup
    // connect(qApp, &QApplication::aboutToQuit, this,
    //         [this]()
    //         {
    //             toLog(LogLevel::Normal, "aboutToQuit signal received - setting m_QuitRequested to
    //             true"); m_QuitRequested = true;
    //         });
}

//----------------------------------------------------------------------------
// Деструктор экрана заставки
SplashScreen::~SplashScreen() = default;

//----------------------------------------------------------------------------
// Инициализация экрана заставки
void SplashScreen::onInit() {
    toLog(LogLevel::Normal, "SplashScreen onInit called");

    // Connect to aboutToQuit signal after initialization to avoid premature quit detection
    connect(qApp, &QApplication::aboutToQuit, this, [this]() {
        toLog(LogLevel::Normal, "aboutToQuit signal received - setting m_QuitRequested to true");
        m_QuitRequested = true;
    });

    toLog(LogLevel::Normal, "SplashScreen onInit completed");
}

//----------------------------------------------------------------------------
// Обработка события закрытия окна
void SplashScreen::closeEvent(QCloseEvent *aEvent) {
    toLog(LogLevel::Normal,
          QString("Close splash screen by event. m_QuitRequested=%1").arg(m_QuitRequested));

    // If quit is requested (e.g., from application menu), allow the window to close
    if (m_QuitRequested) {
        aEvent->accept();
        toLog(LogLevel::Normal, "Quit requested - accepting close event.");
    } else {
        // Otherwise, just minimize the window (don't close it)
        aEvent->ignore();
        showMinimized();
        emit hidden();
        toLog(LogLevel::Normal, "Close ignored - minimizing window instead.");
    }
}

//----------------------------------------------------------------------------
// Фильтр событий для обработки кликов мыши
bool SplashScreen::eventFilter(QObject *aObject, QEvent *aEvent) {
    if (aEvent->type() == QEvent::MouseButtonPress) {
        // Проверим, что был сделан клик по некоторой области
        auto *mouseEvent = dynamic_cast<QMouseEvent *>(aEvent);
        if (mouseEvent) {
            if (m_Areas.isEmpty()) {
                // Размеры виджета до первого показа кривые
                updateAreas();
            }

            TAreas::iterator area =
                std::find_if(m_Areas.begin(),
                             m_Areas.end(),
                             [this, pos = mouseEvent->pos()](const TAreas::value_type &a) {
                                 return testPoint(a, pos);
                             });

            if (area != m_Areas.end()) {
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

    m_Areas << qMakePair(1, QRectF(QPointF(0, 0), QPointF(0.33 * width, 0.33 * height)))
            << qMakePair(2, QRectF(QPointF(0.66 * width, 0), QPointF(width, 0.33 * height)))
            << qMakePair(5,
                         QRectF(QPointF(0.33 * width, 0.33 * height),
                                QPointF(0.66 * width, 0.66 * height)))
            << qMakePair(3, QRectF(QPointF(0, 0.66 * height), QPointF(0.33 * width, height)))
            << qMakePair(4, QRectF(QPointF(0.66 * width, 0.66 * height), QPointF(width, height)));
}

//----------------------------------------------------------------------------
// Установка состояния экрана заставки
void SplashScreen::setState(const QString &aSender, const QString &aState) {
    // Проверим, есть ли такое состояние у нас в списке.
    TStateList::iterator it;

    QString stateCode = aState.left(aState.indexOf('_'));
    QString stateStatus = aState.right(aState.size() - stateCode.size() - 1);

    for (it = m_States.begin(); it != m_States.end(); ++it) {
        QString oldStateCode = it->state.left(it->state.indexOf('_'));
        QString oldStateStatus = it->state.right(it->state.size() - oldStateCode.size() - 1);

        if (stateCode == oldStateCode) {
            if (stateStatus == oldStateStatus) {
                // Уже установлено точно такое же состояние.
                return;
            }

            if (it->date.secsTo(QDateTime::currentDateTime()) <
                CSplashScreen::MinStateShowSeconds) {
                // Use new Qt5/6 compatible signal/slot syntax with lambda
                QTimer::singleShot(CSplashScreen::MinStateShowSeconds * 1000,
                                   it->widget,
                                   [widget = it->widget]() { widget->deleteLater(); });
            } else {
                it->widget->deleteLater();
            }

            m_States.erase(it);

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

    // FIXME: временно для #21549
    // ui.stateLayout->addWidget(widget.get());

    m_States << SState(aSender, aState, widget.release());
}

//----------------------------------------------------------------------------
// Удаление состояний для указанного отправителя
void SplashScreen::removeStates(const QString &aSender) {
    TStateList::iterator it;

    for (it = m_States.begin(); it != m_States.end(); ++it) {
        if (aSender == it->sender) {
            if (it->date.secsTo(QDateTime::currentDateTime()) <
                CSplashScreen::MinStateShowSeconds) {
                // Use new Qt5/6 compatible signal/slot syntax with lambda
                QTimer::singleShot(CSplashScreen::MinStateShowSeconds * 1000,
                                   it->widget,
                                   [widget = it->widget]() { widget->deleteLater(); });
            } else {
                it->widget->deleteLater();
            }

            it = m_States.erase(it);

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

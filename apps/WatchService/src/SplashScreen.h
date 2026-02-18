/* @file Вспомогательный экран, закрывающий рабочий стол. */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QWidget>

#include <Common/ILogable.h>

#include "ui_SplashScreen.h"

class QSequentialAnimationGroup;

//----------------------------------------------------------------------------
/// Вспомогательный экран, закрывающий рабочий стол. Предоставляет доступ к сервисному меню.
class SplashScreen : public QWidget, protected ILogable {
    Q_OBJECT
    Q_PROPERTY(QPoint layoutOffset READ layoutOffset WRITE setLayoutOffset)

public:
    /// Конструктор.
    SplashScreen(const QString &aLog, QWidget *aParent = 0);

    /// Деструктор.
    virtual ~SplashScreen();

    /// Сворачивает окно вместо его закрытия.
    virtual void closeEvent(QCloseEvent *aEvent) override;

    /// Отслеживает клики по экрану для доступа в сервисное меню.
    virtual bool eventFilter(QObject *aObject, QEvent *aEvent) override;

    /// Установка произвольного изображения в качестве фона для защитного экрана.
    virtual void setCustom_Background(const QString &aPath);

    /// Разрешить закрытие окна (вызвать перед quit()).
    void requestQuit();

    /// Показывать оранжевый flash при клике по зоне (аналог QML showAdminFlash).
    void setShowAdminFlash(bool aEnabled) { mShowAdminFlash = aEnabled; }

    /// Показывать номер зоны при клике (аналог QML showAdminNumbers).
    void setShowAdminNumbers(bool aEnabled) { mShowAdminNumbers = aEnabled; }

public slots:
    /// Показывает значок для состояния aState, связанный с отправителем aSender.
    virtual void setState(const QString &aSender, const QString &aState);

    /// Убирает все показанные значки для aSender.
    virtual void removeStates(const QString &aSender);

signals:
    /// Сигнал о клике в определённую зону.
    void clicked(int aArea);

    /// Сигнал о закрытии окна по внешнему сигналу
    void hidden();

private slots:
    // Выполнение полезной инициализации после загрузки.
    void onInit();

private: // Методы
    // Настройка защиты от выгорания экрана (burn-in protection)
    void setupBurnInProtection();

    // Getter/Setter для Q_PROPERTY layoutOffset
    QPoint layoutOffset() const { return mLayoutOffset; }
    void setLayoutOffset(const QPoint &offset);

    typedef QList<QPair<int, QRectF>> TAreas;

    void updateAreas();
    bool testPoint(const TAreas::value_type &aArea, const QPoint &aPoint) const;

private: // Данные
    struct SState {
        SState(const QString &aSender, const QString &aState, QWidget *aWidget)
            : sender(aSender), state(aState), widget(aWidget), date(QDateTime::currentDateTime()) {}

        QString sender;
        QString state;
        QWidget *widget;
        QDateTime date;
    };

    typedef QList<SState> TStateList;

    Ui::SplashScreenClass ui;

    TAreas m_Areas;
    TStateList m_States;
    bool m_QuitRequested;
    QSequentialAnimationGroup *mBurnInProtectionAnim;
    QPoint mLayoutOffset;   // Текущий offset для burn-in protection
    bool mShowAdminFlash;   // Показывать оранжевый flash (аналог QML showAdminFlash)
    bool mShowAdminNumbers; // Показывать номер зоны (аналог QML showAdminNumbers)
};

//----------------------------------------------------------------------------

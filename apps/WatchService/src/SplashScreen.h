/* @file Вспомогательный экран, закрывающий рабочий стол. */

#pragma once

#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QTimer>
#include <QtGui/QCloseEvent>
#include <QtWidgets/QWidget>

#include <Common/ILogable.h>

#include "ui_SplashScreen.h"

class QSequentialAnimationGroup;
class FlashOverlay;

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

    /// Сигнал о закрытии окна по внешнему сигналу.
    void hidden();

protected:
    /// Подгоняет overlay под размер окна при изменении размера.
    void resizeEvent(QResizeEvent *aEvent) override;

private slots:
    void onInit();
    void onFlashTick();

private:
    void setupBurnInProtection();

    QPoint layoutOffset() const { return mLayoutOffset; }
    void setLayoutOffset(const QPoint &offset);

    typedef QList<QPair<int, QRectF>> TAreas;

    void updateAreas();
    bool testPoint(const TAreas::value_type &aArea, const QPoint &aPoint) const;

    struct SState {
        SState(const QString &aSender, const QString &aState, QWidget *aWidget)
            : sender(aSender), state(aState), widget(aWidget), date(QDateTime::currentDateTime()) {}

        QString sender;
        QString state;
        QWidget *widget;
        QDateTime date;
    };

    typedef QList<SState> TStateList;

    // Активный flash-оверлей для одной зоны
    struct SActiveFlash {
        int zoneNumber;
        QRectF zoneRect;
        qreal opacity; // 1.0 → 0.0
    };

    typedef QList<SActiveFlash> TActiveFlashes;

    Ui::SplashScreenClass ui;

    TAreas mAreas;
    TStateList mStates;
    bool mQuitRequested;
    QSequentialAnimationGroup *mBurnInProtectionAnim;
    QPoint mLayoutOffset;
    bool mShowAdminFlash;   // Показывать оранжевый flash (аналог QML showAdminFlash)
    bool mShowAdminNumbers; // Показывать номер зоны (аналог QML showAdminNumbers)
    TActiveFlashes mActiveFlashes;
    QTimer mFlashTimer;     // ~60fps, активен только при наличии флешей
    QWidget *mFlashOverlay; // Прозрачный overlay поверх всех child-виджетов

    friend class FlashOverlay;
};

//----------------------------------------------------------------------------

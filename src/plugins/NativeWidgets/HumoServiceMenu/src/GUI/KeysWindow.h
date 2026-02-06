/* @file Окошко для генерации ключей в сервисном меню и первоначальной настройке. */

#pragma once

#include <QtCore/QFutureWatcher>

#include "ui_KeysWindow.h"

class HumoServiceBackend;

//------------------------------------------------------------------------
namespace CKeysWindow {
const QString WarningStyleSheet = "background-color: rgb(255, 192, 192);";
const QString DefaultStyleSheet = "";
const int ReservedKeyNumber = 100;
} // namespace CKeysWindow

//------------------------------------------------------------------------
class KeysWindow : public QFrame, protected Ui_KeysWindow {
    Q_OBJECT

public:
    KeysWindow(HumoServiceBackend *aBackend, QWidget *aParent);

    virtual ~KeysWindow();

    /// Начальная инициализация формы.
    virtual void initialize(bool aHasRuToken, bool aRutokenOK);

    // Сгенерировать ключи
    void doGenerate();

    /// Сохраняет сгенерированные ключи.
    bool save();

signals:
    /// Начало и конец процедуры создания ключей.
    void beginGenerating();
    void endGenerating();

    /// Сигнал об ошибке во время создания или регистрации ключей.
    void error(QString aError);

protected slots:
    void onCreateButtonClicked();
    void onRepeatButtonClicked();

    void onCheckedKeyPairChanged(int aState);

    void onGenerateTaskFinished();

private:
    void SetStyleSheet(QWidget *widget, const QString &styleSheet) {
        widget->setStyleSheet(QString(widget->metaObject()->className()) + "{" + styleSheet + "}");
    }

protected:
    QVariantMap mTaskParameters;

    QFutureWatcher<bool> mGenerateTaskWatcher;

    HumoServiceBackend *mBackend;
};

//------------------------------------------------------------------------

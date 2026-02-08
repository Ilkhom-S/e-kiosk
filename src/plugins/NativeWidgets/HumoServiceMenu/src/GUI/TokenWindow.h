/* @file Окошко для генерации ключей в сервисном меню и первоначальной настройке. */

#pragma once

#include <QtCore/QFutureWatcher>

#include <Crypt/ICryptEngine.h>

#include "ui_tokenWindow.h"

class HumoServiceBackend;

//------------------------------------------------------------------------
namespace CTokenWindow {
const QString WarningStyleSheet = "background-color: rgb(255, 192, 192);";
const QString DefaultStyleSheet = "";
} // namespace CTokenWindow

//------------------------------------------------------------------------
class TokenWindow : public QFrame, protected Ui_TokenWindow {
    Q_OBJECT

public:
    TokenWindow(HumoServiceBackend *aBackend, QWidget *aParent);

    virtual ~TokenWindow();

    /// Начальная инициализация формы.
    virtual void initialize(const CCrypt::TokenStatus &aStatus);

    // Отформатировать токен
    void doFormat();

signals:
    /// Начало и конец процедуры создания ключей.
    void beginFormat();
    void endFormat();

    /// Сигнал об ошибке во время создания или регистрации ключей.
    void error(QString aError);

protected slots:
    void onFormatButtonClicked();
    void onFormatTaskFinished();

private:
    void updateUI(const CCrypt::TokenStatus &aStatus);

protected:
    QVariantMap m_TaskParameters;

    QFutureWatcher<bool> m_FormatTaskWatcher;

    HumoServiceBackend *m_Backend;
};

//------------------------------------------------------------------------

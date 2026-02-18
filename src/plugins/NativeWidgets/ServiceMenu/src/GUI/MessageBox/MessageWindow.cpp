#include "MessageWindow.h"

#include <QtGui/QMovie>

MessageWindow::MessageWindow(QWidget *parent) : QDialog(parent, Qt::SplashScreen) {
    // Прозрачный фон окна — OS-уровень compositor обрезает углы,
    // QSS border-radius рисует тёмную карточку без прямоугольных артефактов
    setAttribute(Qt::WA_TranslucentBackground);

    ui.setupUi(this);

    // Иконки на кнопках убраны — синие PNG не соответствуют brand-стилю;
    // стиль кнопок полностью задан через QSS в .ui (градиент, border-radius)
    ui.btnOK->setIcon(QIcon());
    ui.btnCancel->setIcon(QIcon());
    ui.btnOK->setText(tr("#ok"));
    ui.btnCancel->setText(tr("#cancel"));

    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(onClickedOk()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(onClickedReject()));
}

//------------------------------------------------------------------------
MessageWindow::~MessageWindow() = default;

//------------------------------------------------------------------------
void MessageWindow::setup(const QString &aText,
                          SDK::GUI::MessageBoxParams::Enum aIcon,
                          SDK::GUI::MessageBoxParams::Enum aButton) {
    ui.lbText->setVisible(true);
    ui.lbText->setText(aText);

    ui.btnOK->setVisible(aButton == SDK::GUI::MessageBoxParams::OK);
    ui.btnCancel->setVisible(aButton == SDK::GUI::MessageBoxParams::Cancel);

    if (aIcon == SDK::GUI::MessageBoxParams::Question) {
        ui.btnOK->setVisible(true);
        ui.btnCancel->setVisible(true);

        ui.lbIcon->setPixmap(QPixmap(":/Images/MessageBox/question.svg"));
    } else if (aIcon == SDK::GUI::MessageBoxParams::Wait) {
        if (!ui.lbIcon->movie()) {
            QPointer<QMovie> gif = QPointer<QMovie>(new QMovie(":/Images/MessageBox/wait.gif"));
            ui.lbIcon->setMovie(gif);
            gif->start();
        }
    } else if (aIcon == SDK::GUI::MessageBoxParams::Info) {
        ui.lbIcon->setPixmap(QPixmap(":/Images/MessageBox/info.svg"));
    } else if (aIcon == SDK::GUI::MessageBoxParams::Warning) {
        ui.lbIcon->setPixmap(QPixmap(":/Images/MessageBox/warning.svg"));
    } else if (aIcon == SDK::GUI::MessageBoxParams::Critical) {
        ui.lbIcon->setPixmap(QPixmap(":/Images/MessageBox/critical.svg"));
    }
}

//------------------------------------------------------------------------
void MessageWindow::onClickedOk() {
    QDialog::accept();
}

//------------------------------------------------------------------------
void MessageWindow::onClickedReject() {
    QDialog::reject();
}

//------------------------------------------------------------------------
void MessageWindow::showEvent(QShowEvent *aEvent) {
    qobject_cast<QWidget *>(parent())->setAttribute(Qt::WA_TransparentForMouseEvents);
    QDialog::showEvent(aEvent);
}

//------------------------------------------------------------------------
void MessageWindow::hideEvent(QHideEvent *aEvent) {
    qobject_cast<QWidget *>(parent())->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    QDialog::hideEvent(aEvent);
}

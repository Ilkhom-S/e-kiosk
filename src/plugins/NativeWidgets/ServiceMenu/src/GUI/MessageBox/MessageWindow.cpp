#include "MessageWindow.h"

#include <QtGui/QMovie>
#include <QtGui/QPainterPath>

MessageWindow::MessageWindow(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) {
    // Qt::SplashScreen на macOS создаёт NSPanel, который блокирует compositor-прозрачность.
    // Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint — обычное безрамочное окно,
    // поддерживающее WA_TranslucentBackground корректно на всех платформах.
    setAttribute(Qt::WA_TranslucentBackground);

    // WA_TranslucentBackground автоматически выставляет WA_NoSystemBackground,
    // но мы явно сбрасываем его, чтобы QSS background:transparent работал корректно
    setAttribute(Qt::WA_NoSystemBackground, false);
    setAttribute(Qt::WA_OpaquePaintEvent, false);

    ui.setupUi(this);

    // Иконки на кнопках убраны — синие PNG не соответствуют brand-стилю;
    // стиль кнопок полностью задан через QSS в .ui (градиент, border-radius)
    ui.btnOK->setIcon(QIcon());
    ui.btnCancel->setIcon(QIcon());
    ui.btnOK->setText(tr("#ok"));
    ui.btnCancel->setText(tr("#cancel"));

    connect(ui.btnOK, &QPushButton::clicked, this, &MessageWindow::onClickedOk);
    connect(ui.btnCancel, &QPushButton::clicked, this, &MessageWindow::onClickedReject);
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
void MessageWindow::paintEvent(QPaintEvent *) {
    // Рисуем скруглённый фон напрямую через QPainter.
    // WA_TranslucentBackground делает OS-окно прозрачным — непрокрашенные углы
    // показывают содержимое родительского окна за диалогом.
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(0x3A, 0x3A, 0x3C), 4));
    painter.setBrush(QColor(0x0A, 0x0A, 0x0B));
    painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), 45, 45);
}

//------------------------------------------------------------------------
void MessageWindow::showEvent(QShowEvent *aEvent) {
    QDialog::showEvent(aEvent);
    if (auto *p = qobject_cast<QWidget *>(parent())) {
        p->setAttribute(Qt::WA_TransparentForMouseEvents);
    }
}

//------------------------------------------------------------------------
void MessageWindow::hideEvent(QHideEvent *aEvent) {
    if (auto *p = qobject_cast<QWidget *>(parent())) {
        p->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
    QDialog::hideEvent(aEvent);
}

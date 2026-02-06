#pragma once

#include <QtWidgets/QWidget>

namespace Ui {
class AdminButton;
}

class AdminButton : public QWidget {
    Q_OBJECT

public:
    explicit AdminButton(QWidget *parent = 0);
    ~AdminButton();

private:
    Ui::AdminButton *ui;

private slots:
    void on_btnClose_clicked();
    void on_btnKeyboard_clicked();
    void on_btnExplorer_clicked();

signals:
    void closeClicked();
    void keyboardClicked();
    void explorerCliked();
};

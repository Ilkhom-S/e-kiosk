#pragma once

#include <QtWidgets/QWidget>

namespace Ui {
class LoadingGprsForm;
}

class LoadingGprsForm : public QWidget {
    Q_OBJECT

public:
    explicit LoadingGprsForm(QWidget *parent = 0);
    ~LoadingGprsForm();

    void setAboutCompany(QString text);
    void setCopirightText(QString text, QString version);
    void setGprsInfo(QString text);
    void setGprsComment(QString text);
    void setSimInfo(QString text);
    void setLogo(QString path);

private:
    Ui::LoadingGprsForm *ui;
};

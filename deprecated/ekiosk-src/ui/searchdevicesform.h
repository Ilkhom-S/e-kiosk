#pragma once

#include <QtWidgets/QWidget>

namespace Ui {
class SearchDevicesForm;
}

class SearchDevicesForm : public QWidget {
    Q_OBJECT

public:
    explicit SearchDevicesForm(QWidget *parent = 0);
    ~SearchDevicesForm();

    void setAboutCompany(QString text);
    void setValidatorSearchText(int state, QString text);
    void setCoinAcceptorSearchText(int state, QString text);
    void setWDSearchText(int state, QString text);
    void setPrinterSearchText(int state, QString text);
    void setModem_SearchText(int state, QString text);
    void setCopirightText(QString text, QString version);
    void setLogo(QString path);

private:
    Ui::SearchDevicesForm *ui;

    QMovie *movie;
    QMovie *srchMovie;
};

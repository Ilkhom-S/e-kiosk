#ifndef SEARCHDEVICESFORM_H
#define SEARCHDEVICESFORM_H

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtWidgets/QWidget>
#include <Common/QtHeadersEnd.h>

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
  void setModemSearchText(int state, QString text);
  void setCopirightText(QString text, QString version);
  void setLogo(QString path);

private:
  Ui::SearchDevicesForm *ui;

  QMovie *movie;
  QMovie *srchMovie;
};

#endif // SEARCHDEVICESFORM_H

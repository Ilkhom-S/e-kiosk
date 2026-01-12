#ifndef INCASACIYAFORM_H
#define INCASACIYAFORM_H

#include <QtWidgets/QDialog>

namespace IncashCmd {
enum In {
  doIncash = 1,
  interAdmin = 2,
  testPrint = 3,
  doNullingCheck = 4,
  closeThis = 5
};

} // namespace IncashCmd

namespace Ui {
class IncasaciyaForm;
}

class IncasaciyaForm : public QDialog {
  Q_OBJECT

public:
  explicit IncasaciyaForm(QWidget *parent = 0);
  ~IncasaciyaForm();

  void setHtmlInfoBox(QString text);

private:
  Ui::IncasaciyaForm *ui;

private slots:
  void btnIncashClc();
  void btnInterAdminClc();
  void btnTestPrintClc();
  void btnDoNullingClc();
  void btnCloseClc();
  void btnChangePassClc();

signals:
  void execCommand(int cmd);
  void openDialog();
};

#endif // INCASACIYAFORM_H

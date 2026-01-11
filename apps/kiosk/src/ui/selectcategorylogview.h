#ifndef SELECTCATEGORYLOGVIEW_H
#define SELECTCATEGORYLOGVIEW_H

#include <QDialog>

namespace Ui {
class SelectCategoryLogView;
}

class SelectCategoryLogView : public QDialog {
    Q_OBJECT

  public:
    explicit SelectCategoryLogView(QWidget *parent = 0);
    ~SelectCategoryLogView();

  signals:
    void emit_SelectOptions(bool SelectValidatorJam, bool SelectMoneyOut, bool SelectERROR,
                            bool SelectPayDaemon, bool SelectStatusAso, bool SelectStatusPrinter,
                            bool SelectStatusValidator, bool SelectConnectionState,
                            bool SelectUpdater);

  private:
    Ui::SelectCategoryLogView *ui;

  private slots:
    void selectAll();
    void deSelectAll();
    void showLikeThis();
};

#endif  // SELECTCATEGORYLOGVIEW_H

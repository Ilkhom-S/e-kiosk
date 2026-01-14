#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QVariantMap>
#include <QtWidgets/QDialog>
#include <Common/QtHeadersEnd.h>

namespace Ui {
    class CreateDialupConnection;
}

class CreateDialupConnection : public QDialog {
    Q_OBJECT

  public:
    explicit CreateDialupConnection(QWidget *parent = 0);
    QStringList conList;
    QStringList devList;
    void openThis();
    ~CreateDialupConnection();

  private:
    Ui::CreateDialupConnection *ui;

  private slots:
    void btnCreateNewConClc();

  signals:
    void emitDialupParam(QVariantMap data);
};

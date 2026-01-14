#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDebug>
#include <QtCore/QMap>
#include <QtCore/QSignalMapper>
#include <QtWidgets/QWidget>
#include <Common/QtHeadersEnd.h>

namespace Ui {
    class keyPud;
}

class keyPud : public QWidget {
    Q_OBJECT

  public:
    explicit keyPud(QWidget *parent = 0);
    ~keyPud();

    void clickBackspace();

  private:
    Ui::keyPud *ui;

    //    QWidget *lastFocusedWidget;
    QSignalMapper signalMapper;

    QMap<QString, QChar> ScharMap;
    QMap<QString, QChar> BcharMap;
    QMap<QString, QString> DoublecharMap;

    bool shiftKEY;
    bool double_on;

    QString gblLang;

  private slots:

    void buttonClicked(QWidget *w);

  public slots:
    void shiftClicked();
    void fordouble(QString lang);
    void changeToRU();
    void changeToEN();

  signals:
    void characterGenerated(QChar character);
};


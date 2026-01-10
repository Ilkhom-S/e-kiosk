#ifndef KEYPUD_H
#define KEYPUD_H

#include <QWidget>
#include <QSignalMapper>
#include <QMap>
#include <QDebug>

namespace Ui {
    class keyPud;
}

class keyPud : public QWidget
{
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

#endif // KEYPUD_H

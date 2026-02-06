/* @file Окно с модемным соединением. */

#pragma once

#include "ui_DialupConnectionWindow.h"

//---------------------------------------------------------------------------
class DialupConnectionWindow : public QWidget, protected Ui::DialupConnectionWindow {
    Q_OBJECT

public:
    DialupConnectionWindow(QWidget *aParent = 0);
    ~DialupConnectionWindow();

    void initialize();
    QString getUserSelection() const;
    void fillModemList(const QList<QPair<QString, QString>> &aModems);
    void fillConnectionList(const QStringList &aConnections, const QString &aCurrent);
    void fillTemplateList(const QStringList &aConnections);

signals:
    /// Пользователь выбрал создание соединения с заданными параметрами.
    void createConnection(const QString &aConnection, const QString &aNetworkDevice);

    /// Пользователь выбрал тест соединения с заданными параметрами.
    void testConnection(const QString &aConnection);

    /// Пользователь выбрал удаление соединения с заданными параметрами.
    void removeConnection(const QString &aConnection);

    /// Пользователь изменил свой выбор.
    void userSelectionChanged(const QString &aSelectedConnection);

    /// Внутренний сигнал, изменилась отображаемая информация.
    void updated();

public slots:
    void switchToCreatePage();
    void switchToListPage();

private slots:
    void onUpdated();
    void onCreateConnection();
    void onTestConnection();
    void onRemoveConnection();
};

//---------------------------------------------------------------------------

/* @file Реализация компоненты для редактирования профилей устройств. */

#pragma once

#include <QtWidgets/QListWidgetItem>
#include <QtWidgets/QStyledItemDelegate>

//------------------------------------------------------------------------
class EditorPaneListItem_Delegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    EditorPaneListItem_Delegate(QObject *aParent = 0);
};

//------------------------------------------------------------------------
class EditorPaneListItem : public QListWidgetItem {
public:
    enum DataRole { ParameterName = Qt::UserRole + 1, ParameterValue };

    EditorPaneListItem() {}
    virtual ~EditorPaneListItem() {}

    virtual QVariant data(int aRole) const;
    virtual void setData(int aRole, const QVariant &aValue);

private:
    QString m_Value;
    QString m_Name;
};

//---------------------------------------------------------------------------

/* @file Реализация компоненты для редактирования профилей устройств. */

#include "EditorPaneListItem.h"

#include <QtWidgets/QApplication>

EditorPaneListItem_Delegate::EditorPaneListItem_Delegate(QObject *aParent)
    : QStyledItemDelegate(aParent) {}

//------------------------------------------------------------------------
QVariant EditorPaneListItem::data(int aRole) const {
    if (aRole == Qt::SizeHintRole) {
        return QSize(0, 40);
    }
    if (aRole == ParameterName) {
        return m_Name;
    }
    if (aRole == ParameterValue) {
        return m_Value;
    }
    if (aRole == Qt::DisplayRole) {
        return m_Name + ":\n" + m_Value;
    } else {
        return QListWidgetItem::data(aRole);
    }
}

//------------------------------------------------------------------------
void EditorPaneListItem::setData(int aRole, const QVariant &aValue) {
    if (aRole == ParameterName) {
        m_Name = aValue.toString();
    } else if (aRole == ParameterValue) {
        m_Value = aValue.toString();
    } else {
        QListWidgetItem::setData(aRole, aValue);
    }
}

//------------------------------------------------------------------------

/* @file Реализация компоненты для редактирования профилей устройств. */

#pragma once

#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

#include "ui_EditorPane.h"

// Plugin SDK
#include <SDK/Plugins/PluginParameters.h>

class DeviceSlot;
class IDeviceBackend;

//------------------------------------------------------------------------
/// Базовое окно для редактирования параметров устройства.
/// Окно редактора должно быть связано со слотом устройства и после
/// завершения редактирования параметров оно отошлёт слоту сигнал об
/// изменении состояния.

class EditorPane : public QObject {
    Q_OBJECT

public:
    enum EditorType { Enum = 0, Bool, Number, Text };

    EditorPane();

    virtual ~EditorPane();

    /// Связывает окно редактора со слотом устройства.
    virtual void setSlot(IDeviceBackend *aBackend, DeviceSlot *aSlot);

    /// Возвращает связанный с редактором слот устройства.
    virtual DeviceSlot *getSlot() const;

    /// Возвращает виджет окна с настройками.
    virtual QWidget *getWidget();

    /// После показа окна редактора параметры устройства обновлены?
    virtual bool isChanged() const;

    /// Возвращает список параметров устройства после редактирования.
    virtual const QVariantMap &getParameterValues() const;

protected:
    /// Создание виджета для размещения редактора настроек. Используется в getWidget().
    virtual QWidget *createWidget();

    /// Обновляет список параметров.
    virtual void updateView();

    /// Переводит фокус на первое незаполненное поле.
    virtual void selectEmptyParameter();

    /// Установить значение активному параметру.
    virtual void setCurrentParameterValue(const QString &aValue);

    /// Показывает значения для активного параметра.
    virtual void showCurrentParameterValues();

signals:
    /// Сигнал срабатывает, когда работа с редактором завершена.
    void finished();

protected slots:
    void onParameterRowChanged(QListWidgetItem *aCurrent, QListWidgetItem *aPrevious);

    /// Срабатывает, когда изменяется значение списка.
    void onEnum_ValueChanged(QListWidgetItem *aItem);

    /// Срабатывает, когда изменяется значение флага.
    void onBoolValueChanged();

    /// Срабатывает, когда изменяется значение числового поля.
    void onNumberValueChanged();

    /// Срабатывает, когда изменяется значение текстового поля.
    void onTextValueChanged();

    /// Срабатывает при нажатии кнопки "Значение по умолчанию".
    void onDefault();

    /// Срабатывает по нажатию кнопки "Ок".
    void onOk();

    /// Срабатывает по нажатию кнопки "Отмена".
    void onCancel();

private:
    Ui::frmEditorPane m_Ui{};
    DeviceSlot *m_Slot;
    QPointer<QWidget> m_Widget;

    bool m_FirstShow;

    QVariantMap m_Values;

    SDK::Plugin::TParameterList m_Parameters{};

    /// TODO: Список моделей устройств, временно здесь.
    QMap<QString, SDK::Plugin::TParameterList> m_Models;
};

//------------------------------------------------------------------------

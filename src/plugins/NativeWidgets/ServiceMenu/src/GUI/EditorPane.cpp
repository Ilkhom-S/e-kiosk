/* @file Реализация компоненты для редактирования профилей устройств. */

// Standard

#include "EditorPane.h"

#include <algorithm>
#include <memory>

#include "DeviceSlot.h"
#include "EditorPaneListItem.h"
#include "IDeviceBackend.h"
#include "SIPStyle.h"

namespace CEditorPane {
const QString SystemPrinterName = "Default Default";
} // namespace CEditorPane

//------------------------------------------------------------------------
EditorPane::EditorPane() : m_Slot(nullptr), m_FirstShow(true) {}

//------------------------------------------------------------------------
EditorPane::~EditorPane() = default;

//------------------------------------------------------------------------
void EditorPane::setSlot(IDeviceBackend *aBackend, DeviceSlot *aSlot) {
    m_Slot = aSlot;
    m_Values = aSlot->getParameterValues();
    m_FirstShow = true;

    // Обновление списка поддерживаемых моделей.
    m_Models.clear();

    QStringList models = aBackend->getModels(aSlot->getType());

    foreach (const QString &model, models) {
        m_Models[model] = aBackend->getModelParameters(aSlot->getType(), model);
    }

    if (!m_Widget.data()) {
        m_Widget = createWidget();
    }

    // Первоначально доступна только модель устройства.
    SDK::Plugin::SPluginParameter parameter;

    parameter.title = QCoreApplication::translate(
        "Hardware::CommonParameters", QT_TRANSLATE_NOOP("Hardware::CommonParameters", "#model"));
    parameter.description = QCoreApplication::translate(
        "Hardware::CommonParameters",
        QT_TRANSLATE_NOOP("Hardware::CommonParameters", "#model_howto"));
    parameter.name = "model_name";
    parameter.type = SDK::Plugin::SPluginParameter::Set;
    parameter.required = true;
    parameter.readOnly = false;

    foreach (const QString &model, m_Models.keys()) {
        if (model == CEditorPane::SystemPrinterName) {
            parameter.possibleValues.insert(
                QCoreApplication::translate(
                    "Hardware::PrinterParameters",
                    QT_TRANSLATE_NOOP("Hardware::PrinterParameters", "#system_printer_name")),
                model);
        } else {
            parameter.possibleValues.insert(model, QVariant());
        }
    }

    m_Parameters.clear();
    m_Parameters.push_back(parameter);

    updateView();
    selectEmptyParameter();
}

//------------------------------------------------------------------------
void EditorPane::updateView() {
    // Первоначальная инициализация, выбор модели.
    QVariantMap::iterator model = m_Values.find("model_name");
    if (model != m_Values.end()) {
        QString modelValue(model.value().toString());
        foreach (QString modelsValue, m_Models.keys()) {
            if (modelsValue.toLower() == modelValue.toLower()) {
                // Модель задана, можно заполнять параметры.
                SDK::Plugin::TParameterList driverParameters = m_Models[modelsValue];

                m_Parameters.resize(1);
                m_Parameters << driverParameters;
                break;
            }
        }
    }

    m_Ui.lwParameters->blockSignals(true);
    m_Ui.lwParameters->clear();
    m_Ui.lwParameters->blockSignals(false);

    foreach (const SDK::Plugin::SPluginParameter &parameter, m_Parameters) {
        // Добавляем только редактируемые параметры
        if (parameter.readOnly) {
            continue;
        }

        auto *item = new EditorPaneListItem();
        item->setData(EditorPaneListItem::ParameterName, parameter.title);

        // Заполняем параметрами по умолчанию (если есть)
        if (!parameter.defaultValue.isNull() && m_Values[parameter.name].isNull()) {
            item->setData(EditorPaneListItem::ParameterValue, parameter.defaultValue.toString());
            m_Values[parameter.name] = parameter.defaultValue;
        } else {
            item->setData(EditorPaneListItem::ParameterValue, m_Values[parameter.name]);
        }

        m_Ui.lwParameters->addItem(item);
    }
}

//------------------------------------------------------------------------
void EditorPane::selectEmptyParameter() {
    m_Ui.btnOk->setEnabled(false);

    foreach (const SDK::Plugin::SPluginParameter &parameter, m_Parameters) {
        // TODO parameter.required завязать логику на требуемый ресурс
        if (m_Values.value(parameter.name).isNull() && !parameter.readOnly &&
            parameter.defaultValue.isNull()) {
            QList<QListWidgetItem *> items = m_Ui.lwParameters->findItems(
                parameter.title, Qt::MatchCaseSensitive | Qt::MatchStartsWith);

            if (!items.isEmpty()) {
                m_Ui.lwParameters->setCurrentItem(items.first());
                return;
            }
        }
    }

    if (m_FirstShow) {
        // Всё заполнено, выделяем последний параметр.
        if (m_Ui.lwParameters->count()) {
            m_Ui.lwParameters->setCurrentItem(
                m_Ui.lwParameters->item(m_Ui.lwParameters->count() - 1));

            m_FirstShow = false;
        }
    }

    m_Ui.btnOk->setEnabled(true);
}

//------------------------------------------------------------------------
void EditorPane::setCurrentParameterValue(const QString &aValue) {
    QListWidgetItem *item = m_Ui.lwParameters->currentItem();

    if (item) {
        for (const auto &param : m_Parameters) {
            if (param.title == item->data(EditorPaneListItem::ParameterName)) {
                QVariant oldValue = m_Values[param.name];

                if (aValue.isNull() && !param.defaultValue.isNull()) {
                    m_Values[param.name] = param.defaultValue;
                } else {
                    if (!param.possibleValues[aValue].isNull()) {
                        m_Values[param.name] = param.possibleValues[aValue];
                    } else {
                        m_Values[param.name] = aValue;
                    }
                }

                item->setData(EditorPaneListItem::ParameterValue, m_Values[param.name]);

                if ((param.name == "model_name") && (oldValue != m_Values[param.name])) {
                    QVariant oldModel = m_Values["model_name"];

                    m_Values.clear();
                    m_Values["model_name"] = oldModel;

                    updateView();
                }

                m_Ui.lwValues->blockSignals(true);
                selectEmptyParameter();
                m_Ui.lwValues->blockSignals(false);

                m_Ui.lwParameters->repaint();

                break;
            }
        }
    }
}

//------------------------------------------------------------------------
void EditorPane::showCurrentParameterValues() {
    QListWidgetItem *item = m_Ui.lwParameters->currentItem();
    if (item) {
        foreach (const SDK::Plugin::SPluginParameter &parameter, m_Parameters) {
            if (parameter.title == item->data(EditorPaneListItem::ParameterName)) {
                switch (parameter.type) {
                case SDK::Plugin::SPluginParameter::Set:
                case SDK::Plugin::SPluginParameter::MultiSet: {
                    m_Ui.stackedWidget->setCurrentIndex(Enum);
                    m_Ui.lwValues->clear();

                    // Отсортируем возможные значения параметра драйвера
                    auto intOrderLessThan = [&](const QString &s1, const QString &s2) -> bool {
                        bool ok = false;
                        s1.toInt(&ok);
                        return ok ? s1.toInt() < s2.toInt() : s1 < s2;
                    };

                    QStringList possibleValues(parameter.possibleValues.keys());
                    std::sort(possibleValues.begin(), possibleValues.end(), intOrderLessThan);
                    m_Ui.lwValues->addItems(possibleValues);

                    QString title;

                    if (!m_Values[parameter.name].isNull()) {
                        if (parameter.possibleValues.contains(
                                m_Values[parameter.name].toString())) {
                            title = m_Values[parameter.name].toString();
                        } else {
                            for (QVariantMap::const_iterator it = parameter.possibleValues.begin();
                                 it != parameter.possibleValues.end();
                                 ++it) {
                                if (!it.value().isNull() &&
                                    m_Values[parameter.name] == it.value()) {
                                    title = it.key();

                                    break;
                                }
                            }
                        }
                    }

                    QList<QListWidgetItem *> items =
                        m_Ui.lwValues->findItems(title, Qt::MatchCaseSensitive);
                    if (!items.isEmpty()) {
                        m_Ui.lwValues->blockSignals(true);
                        m_Ui.lwValues->setCurrentItem(items.first());
                        m_Ui.lwValues->blockSignals(false);
                    }

                    break;
                }

                case SDK::Plugin::SPluginParameter::Bool: {
                    m_Ui.stackedWidget->setCurrentIndex(Bool);

                    if (m_Values[parameter.name].isNull()) {
                        m_Ui.rbOn->setAutoExclusive(false);
                        m_Ui.rbOff->setAutoExclusive(false);
                        m_Ui.rbOn->setChecked(false);
                        m_Ui.rbOff->setChecked(false);
                    } else {
                        m_Ui.rbOn->setAutoExclusive(true);
                        m_Ui.rbOff->setAutoExclusive(true);
                        m_Ui.rbOn->setChecked(m_Values[parameter.name].toBool());
                        m_Ui.rbOff->setChecked(!m_Values[parameter.name].toBool());
                    }

                    break;
                }

                case SDK::Plugin::SPluginParameter::Number: {
                    m_Ui.stackedWidget->setCurrentIndex(Number);

                    QString value = QString("%1").arg(
                        m_Values[parameter.name].toInt() % 1000, 3, 10, QChar('0'));

                    m_Ui.sbDigit1->setValue(QString(value[0]).toInt());
                    m_Ui.sbDigit2->setValue(QString(value[1]).toInt());
                    m_Ui.sbDigit3->setValue(QString(value[2]).toInt());
                }

                case SDK::Plugin::SPluginParameter::Text: {
                    m_Ui.stackedWidget->setCurrentIndex(Text);

                    m_Ui.leValue->setInputMask(parameter.possibleValues.value("mask").toString());
                    m_Ui.leValue->setText(m_Values[parameter.name].toString());

                    break;
                }

                default:
                    break;
                }

                m_Ui.lbDescription->setVisible(!parameter.description.isEmpty());
                m_Ui.lbDescription->setText(parameter.description);
                m_Ui.lbParameterDescription->setText(parameter.title);
            }
        }
    }
}

//------------------------------------------------------------------------
void EditorPane::onParameterRowChanged(QListWidgetItem * /*aCurrent*/,
                                       QListWidgetItem * /*aPrevious*/) {
    showCurrentParameterValues();
}

//------------------------------------------------------------------------
void EditorPane::onEnumValueChanged(QListWidgetItem *aItem) {
    setCurrentParameterValue(aItem->text());
}

//------------------------------------------------------------------------
void EditorPane::onBoolValueChanged() {
    setCurrentParameterValue(sender() == m_Ui.rbOn ? "true" : "false");
}

//------------------------------------------------------------------------
void EditorPane::onNumberValueChanged() {
    QString value = QString::number(m_Ui.sbDigit1->value()) +
                    QString::number(m_Ui.sbDigit2->value()) +
                    QString::number(m_Ui.sbDigit3->value());

    setCurrentParameterValue(QString::number(value.toInt()));
}

//------------------------------------------------------------------------
void EditorPane::onTextValueChanged() {
    setCurrentParameterValue(m_Ui.leValue->text());
}

//------------------------------------------------------------------------
DeviceSlot *EditorPane::getSlot() const {
    return m_Slot;
}

//------------------------------------------------------------------------
QWidget *EditorPane::getWidget() {
    if (!m_Widget.data()) {
        m_Widget = createWidget();
    }

    return m_Widget.data();
}

//------------------------------------------------------------------------
QWidget *EditorPane::createWidget() {
    std::unique_ptr<QWidget> widget(new QWidget());

    m_Ui.setupUi(widget.get());

    connect(m_Ui.lwParameters,
            SIGNAL(currentItem_Changed(QListWidgetItem *, QListWidgetItem *)),
            SLOT(onParameterRowChanged(QListWidgetItem *, QListWidgetItem *)));

    connect(m_Ui.lwValues,
            SIGNAL(item_Clicked(QListWidgetItem *)),
            SLOT(onEnumValueChanged(QListWidgetItem *)));

    connect(m_Ui.btnOk, SIGNAL(clicked()), SLOT(onOk()));
    connect(m_Ui.btnCancel, SIGNAL(clicked()), SLOT(onCancel()));

    connect(m_Ui.rbOn, SIGNAL(clicked()), SLOT(onBoolValueChanged()));
    connect(m_Ui.rbOff, SIGNAL(clicked()), SLOT(onBoolValueChanged()));

    connect(m_Ui.btnPlus1, SIGNAL(clicked()), SLOT(onNumberValueChanged()));
    connect(m_Ui.btnPlus2, SIGNAL(clicked()), SLOT(onNumberValueChanged()));
    connect(m_Ui.btnPlus3, SIGNAL(clicked()), SLOT(onNumberValueChanged()));

    connect(m_Ui.btnMinus1, SIGNAL(clicked()), SLOT(onNumberValueChanged()));
    connect(m_Ui.btnMinus2, SIGNAL(clicked()), SLOT(onNumberValueChanged()));
    connect(m_Ui.btnMinus3, SIGNAL(clicked()), SLOT(onNumberValueChanged()));

    connect(m_Ui.leValue, SIGNAL(returnPressed()), this, SLOT(onTextValueChanged()));
    m_Ui.leValue->setStyle(new SIPStyle);

    m_Ui.lwParameters->setItemDelegate(new EditorPaneListItem_Delegate());

    return widget.release();
}

//------------------------------------------------------------------------
bool EditorPane::isChanged() const {
    return (m_Values != m_Slot->getParameterValues());
}

//------------------------------------------------------------------------
const QVariantMap &EditorPane::getParameterValues() const {
    return m_Values;
}

//------------------------------------------------------------------------
void EditorPane::onDefault() {
    setCurrentParameterValue(QString());
}

//------------------------------------------------------------------------
void EditorPane::onOk() {
    emit finished();
}

//------------------------------------------------------------------------
void EditorPane::onCancel() {
    m_Values = m_Slot->getParameterValues();

    emit finished();
}

//------------------------------------------------------------------------

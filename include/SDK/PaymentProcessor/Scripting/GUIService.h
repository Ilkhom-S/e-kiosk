/* @file Прокси класс для работы с графическим движком в скриптах. */

#pragma once

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

namespace SDK {
namespace PaymentProcessor {

class ICore;
class IGUIService;

namespace Scripting {

//------------------------------------------------------------------------------
/// Прокси класс для работы с графическим движком в скриптах.
class GUIService : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool isDisabled READ isDisabled)
    Q_PROPERTY(int width READ getWidth CONSTANT)
    Q_PROPERTY(int height READ getHeight CONSTANT)
    Q_PROPERTY(QVariantMap ui READ getParametersUI CONSTANT)
    Q_PROPERTY(QVariantMap ad READ getParametersAd CONSTANT)
    Q_PROPERTY(QString topScene READ getTopScene CONSTANT)

public:
    /// Конструктор.
    GUIService(ICore *aCore);

public slots:
    /// Отображает виджет.
    bool show(const QString &aWidget, const QVariantMap &aParameters);

    /// Отображает popup-виджет (на уровень выше текущего).
    bool showPopup(const QString &aWidget, const QVariantMap &aParameters);

    /// Скрывает текущий popup или модальный виджет.
    bool hidePopup(const QVariantMap &aParameters = QVariantMap());

    /// Оповещает виджет.
    void notify(const QString &aEvent, const QVariantMap &aParameters);

    /// Сбросить.
    void reset();

    /// Перезагрузить.
    void reload(const QVariantMap &aParams);

    /// Получить верхнюю сцену.
    QString getTopScene() const;

    // Когда надо из qml что-то передать в скрипты
    /// Оповестить сценарий.
    void notifyScenario(const QVariantMap &aParams) { emit notifyScriptEngine(aParams); }

signals:
    /// Сигнал об изменении верхней сцены.
    void topSceneChange();
    /// Сигнал о перезагрузке скина.
    void skinReload(const QVariantMap &aParams);
    /// Сигнал об оповещении скриптового движка.
    void notifyScriptEngine(const QVariantMap &aParams);

private:
    /// Получить параметры UI.
    QVariantMap getParametersUI() const;
    /// Получить параметры рекламы.
    QVariantMap getParametersAd() const;

private:
    /// Проверить, отключен ли.
    bool isDisabled() const;
    /// Получить ширину.
    int getWidth() const;
    /// Получить высоту.
    int getHeight() const;

private:
    /// Указатель на ядро.
    ICore *mCore;
    /// Указатель на сервис GUI.
    IGUIService *mGUIService;
    /// Имя верхнего виджета.
    QString mTopWidgetName;
};

//------------------------------------------------------------------------------
} // namespace Scripting
} // namespace PaymentProcessor
} // namespace SDK

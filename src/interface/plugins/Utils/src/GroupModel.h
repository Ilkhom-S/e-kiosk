/* @file Модель для отображения списка провайдеров. */
#pragma once

#include <QtCore/QAbstractListModel>
#include <QtCore/QPointer>
#include <QtCore/QReadWriteLock>
#include <QtCore/QSharedPointer>
#include <QtCore/QStringList>
#include <QtXml/QDom_Document>

class Item;
class Item_Object;

//------------------------------------------------------------------------------
class GroupModel : public QAbstractListModel {
    Q_OBJECT

    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(QString source READ getSource WRITE setSource)
    Q_PROPERTY(qint64 category READ getCategory NOTIFY categoryChanged)
    Q_PROPERTY(QString categoryName READ getCategoryName NOTIFY categoryNameChanged)
    Q_PROPERTY(qint64 rootElement READ getRootElement WRITE setRootElement)
    Q_PROPERTY(QStringList elementFilter READ getElementFilter WRITE setElementFilter)
    Q_PROPERTY(quint16 maxNameLength READ getMaxNameLength)

    enum EntryRoles {
        IdRole = Qt::UserRole + 1,
        NameRole,
        TitleRole,
        DescriptionRole,
        TypeRole,
        ImageRole,
        IsGroupRole,
        JSONRole
    };

public:
    GroupModel();

    virtual int rowCount(const QModelIndex &aParent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &aIndex, int aRole) const;

    /// Получить ID всех провайдеров
    QSet<qint64> allProviders() const;

    /// Получить имя провайдера
    QString getProviderName(qint64 aProviderId) const;

    /// Загрузить в модель данные статистики
    void setStatistic(QMap<qint64, quint32> &aStatistic);

public slots:
    QObject *get(int aIndex);

    int getMaxNameLength() const;

    qint64 getCategory() const;
    QString getCategoryName() const;
    qint64 findCategory(qint64 aProviderId) const;
    bool isProviderInCategory(qint64 aProvider, qint64 aCategory) const;

    QString getSource() const;
    void setSource(QString aSource);

    qint64 getRootElement() const;
    void setRootElement(qint64 aRootElement);

    QStringList getElementFilter();
    void setElementFilter(QStringList aFilter);

signals:
    void rowCountChanged();
    void categoryChanged();
    void categoryNameChanged();

public:
    typedef QSharedPointer<Item> Item_Ptr;
    typedef QList<Item_Ptr> Item_List;

private:
    bool loadContent(const QString &m_FileName, QDom_Document &aDocument);

    /// Слияние двух groups
    void mergeGroups(QDom_Element aTargetGroup, QDom_Element aSourceGroup);

    void setRootElementInternal(qint64 aRootElement);

    /// Высчитываем максимальную индекс сортировки для содержимого группы
    quint32 getGroupOrder(qint64 aGroupID);

    qint64 getCategory(QDomNode aNode);

    void clearNodes();

    const Item_List &getItem_List(qint64 aGroupID);

    virtual QHash<int, QByteArray> roleNames() const;

private:
    QHash<int, QByteArray> m_Roles;

    /// Имя xml файла с списком групп
    QString m_Source;

    /// Весь документ
    QDom_Document m_Document;

    /// Список групп по их идентификаторам
    QHash<qint64, QDomNode> m_Groups;

    /// Соответствие категории для каждой группы
    QHash<qint64, qint64> m_Categories;

    /// Соответствие категории для каждого провайдера
    QHash<qint64, qint64> m_ProviderCategorys;

    /// Фильтр имён тегов xml
    QStringList m_ElementFilter;

    /// Id текущей корневой группы
    qint64 m_RootElement;

    /// Id текущей категории
    qint64 m_CurrentCategory;

    /// Список узлов внутри текущей корневой группы
    Item_List m_Nodes;
    QMap<qint64, Item_List> m_NodesCache;
    QList<QPointer<Item_Object>> m_NodesObject;

    /// Данные для сортировки кнопок, полученные по статистике платежей
    QMap<qint64, quint32> m_ProvidersStatistic;

    /// Группа - количество столбцов
    QMap<qint64, qint32> m_GroupsWidth;
};

//------------------------------------------------------------------------------
class Item {
public:
    Item(const QDomNode &aNode);

    virtual qint64 getId() const;
    virtual QString getName() const;
    virtual QString getTitle() const;
    virtual QString getDescription() const;
    virtual QString getType() const;
    virtual QString getImage() const;
    virtual QString getJSON() const;
    virtual bool isGroup() const;

    virtual QString getElementName() const;
    bool is(const QString &aElementName) const;

    virtual void setOrder(quint32 aOrder);
    virtual quint32 getOrder() const;

protected:
    QDom_NamedNodeMap m_Attributes;
    bool m_IsGroup;
    QString m_ElementName;
    quint32 m_Order;
};

//------------------------------------------------------------------------------
class Item_Object : public QObject {
    Q_OBJECT

    Q_PROPERTY(qint64 id READ getId)
    Q_PROPERTY(QString name READ getName)
    Q_PROPERTY(QString title READ getTitle)
    Q_PROPERTY(QString description READ getDescription)
    Q_PROPERTY(QString type READ getType)
    Q_PROPERTY(QString image READ getImage)
    Q_PROPERTY(QString json READ getJSON)
    Q_PROPERTY(bool isGroup READ isGroup)

public:
    Item_Object(const Item &aItem, QObject *aParent);

    qint64 getId() const;
    QString getName() const;
    QString getTitle() const;
    QString getDescription() const;
    QString getType() const;
    QString getImage() const;
    QString getJSON() const;
    bool isGroup() const;

private:
    const Item &m_Item;
};

//------------------------------------------------------------------------------

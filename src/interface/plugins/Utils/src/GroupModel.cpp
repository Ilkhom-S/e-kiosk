/* @file Модель для отображения списка провайдеров. */
#pragma once

// stl
#include "GroupModel.h"

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QTextCodec>

#include <algorithm>

#include "Log.h"
#include "ProviderConstants.h"

namespace CGroupModel {
const QString Group = "group";
const QString Operator = "operator";
const QString GroupLink = "group_link";

const QString RootGroupType = "root";

namespace Attributes {
const char Id[] = "id";
const char ExtId[] = "ext_id";
const char Name[] = "name";
const char Title[] = "title";
const char Description[] = "descr";
const char Type[] = "type";
const char Image[] = "image";
const char IsGroup[] = "isGroup";
const char JSON[] = "json";
} // namespace Attributes
} // namespace CGroupModel

//------------------------------------------------------------------------------
static QString rm_Bom(const QString &aFile) {
    QFile file(aFile);

    if (file.open(QIODevice::ReadWrite)) {
        QByteArray data = file.readAll();

        // detect utf8 BOM
        // https://codereview.qt-project.org/#/c/93658/5/src/corelib/io/qsettings.cpp
        const uchar *dd = (const uchar *)data.constData();
        if (data.size() >= 3 && dd[0] == 0xef && dd[1] == 0xbb && dd[2] == 0xbf) {
            file.resize(0);
            file.write(QString::from_Utf8(data.remove(0, 3)).toUtf8());
        }
    }

    return aFile;
};

//------------------------------------------------------------------------------
GroupModel::GroupModel() : m_RootElement(-1), m_CurrentCategory(0) {
    m_Roles[IdRole] = CGroupModel::Attributes::Id;
    m_Roles[NameRole] = CGroupModel::Attributes::Name;
    m_Roles[TitleRole] = CGroupModel::Attributes::Title;
    m_Roles[DescriptionRole] = CGroupModel::Attributes::Description;
    m_Roles[TypeRole] = CGroupModel::Attributes::Type;
    m_Roles[ImageRole] = CGroupModel::Attributes::Image;
    m_Roles[IsGroupRole] = CGroupModel::Attributes::IsGroup;
    m_Roles[JSONRole] = CGroupModel::Attributes::JSON;
}

//------------------------------------------------------------------------------
QHash<int, QByteArray> GroupModel::roleNames() const {
    return m_Roles;
}

//------------------------------------------------------------------------------
QObject *GroupModel::get(int aIndex) {
    if (m_Nodes.isEmpty() || aIndex < 0) {
        return nullptr;
    }

    Item_Object *iObject = new Item_Object(*m_Nodes.at(aIndex), this);

    m_NodesObject << iObject;

    return iObject;
}

//------------------------------------------------------------------------------
int GroupModel::getMaxNameLength() const {
    int result = 0;

    // Настройками можно выставить  ширину группы принудительно
    if (m_GroupsWidth.contains(m_RootElement)) {
        // Ширина одной колонки 60 символов, максимальная ширина - 240 символов
        return (int)(241.f / m_GroupsWidth.value(m_RootElement));
    }

    foreach (auto item, m_Nodes) {
        int length = item->getName().length();
        result = result > length ? result : length;
    }

    return result;
}

//------------------------------------------------------------------------------
int GroupModel::rowCount(const QModelIndex &) const {
    return m_Nodes.count();
}

//------------------------------------------------------------------------------
QString GroupModel::getSource() const {
    return m_Source;
}

//------------------------------------------------------------------------------
bool GroupModel::loadContent(const QString &aFileName, QDom_Document &aDocument) {
    aDocument.clear();

    QFileInfo fileInfo(aFileName);

    // Загрузка или догрузка контента в дерево групп
    auto loadXml = [this](const QString &aFileName, QDom_Document &aDocument) -> bool {
        QFile file(aFileName);
        if (!file.open(QIODevice::ReadOnly)) {
            Log(Log::Error) << QString("GroupModel: Error open file %1.").arg(aFileName);
            return false;
        }

        QByteArray sourceContent = file.readAll();
        file.close();

        QString errorMessage;
        int line, column;
        if (!aDocument.setContent(sourceContent, &errorMessage, &line, &column)) {
            Log(Log::Error)
                << QString("GroupModel: %1 in %2:%3").arg(errorMessage).arg(line).arg(column);
            return false;
        }

        // Загружаем настройки размеров для групп
        {
            QSettings ini(rm_Bom(QString(aFileName).replace(".xml", ".ini")), QSettings::IniFormat);
            ini.setIniCodec("UTF-8");
            ini.beginGroup("columns");

            foreach (QString key, ini.allKeys()) {
                m_GroupsWidth.insert(key.toLongLong(), ini.value(key).toInt());
            }
        }

        return true;
    };

    // Грузим для начала основной groups.xml
    if (!loadXml(aFileName, aDocument)) {
        return false;
    }

    m_Source = aFileName;

    // Затем грузим все остальные пользовательские группы и встраиваем их в текущее дерево хитрой
    // функцией
    QStringList filters;
    filters << fileInfo.fileName().replace(".xml", "*.xml", Qt::CaseInsensitive);

    foreach (auto file, fileInfo.dir().entryInfoList(filters, QDir::Files, QDir::Name)) {
        QDom_Document localDoc;

        if (!file.fileName().compare(fileInfo.fileName(), Qt::CaseInsensitive)) {
            continue;
        }

        if (loadXml(file.filePath(), localDoc)) {
            mergeGroups(aDocument.elementsByTagName("groups").at(0).toElement(),
                        localDoc.elementsByTagName("groups").at(0).toElement());
        } else {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------
void GroupModel::mergeGroups(QDom_Element aTargetGroup, QDom_Element aSourceGroup) {
    // auto getAttr = [](const QDom_Element & aElement, )
    auto ID = [](QDom_Element aElement) -> qint64 {
        return aElement.attribute("id", "0").toLongLong();
    };

    auto findGroup =
        [&](const QDom_Element &aElement, qint64 aID, QDom_Element &aGroupElement) -> bool {
        for (QDom_Node n = aElement.firstChild(); !n.isNull(); n = n.nextSibling()) {
            if (n.nodeName() == "group") {
                QDom_Element element = n.toElement();
                if (ID(element) == aID) {
                    aGroupElement = element;
                    return true;
                }
            }
        }
        return false;
    };

    auto copyAttr = [](const QDom_Element &aSource, QDom_Element &aDestination) {
        for (int i = 0; i < aSource.attributes().size(); i++) {
            QDom_Attr attr = aSource.attributes().item(i).toAttr();
            aDestination.setAttribute(attr.name(), attr.value());
        }
    };

    QDom_Element element = aSourceGroup.lastChildElement();
    while (!element.isNull()) {
        if (element.nodeName() == "group") {
            QDom_Element target;
            if (findGroup(aTargetGroup, ID(element), target)) {
                copyAttr(element, target);
                mergeGroups(target, element);
                element = element.previousSiblingElement();
            } else {
                // вставляем в начало списка
                auto nextElement = element.previousSiblingElement();
                aTargetGroup.insertBefore(element, QDom_Node());
                element = nextElement;
            }
        } else {
            // вставляем в начало списка
            auto nextElement = element.previousSiblingElement();
            aTargetGroup.insertBefore(element, QDom_Node());
            element = nextElement;
        }
    }
}

//------------------------------------------------------------------------------
void GroupModel::setSource(QString aSource) {
    if (m_Source == aSource) {
        return;
    }

    if (!loadContent(aSource, m_Document)) {
        return;
    }

    emit beginResetModel();

    m_Groups.clear();

    // Корневая группа
    m_Groups[0] = m_Document.documentElement();

    QDom_NodeList groups = m_Document.elementsByTagName(CGroupModel::Group);
    for (int i = 0; i < groups.count(); i++) {
        Item item(groups.at(i));

        qint64 id = item.getId();

        m_Groups[id] = groups.at(i);
        m_Categories[id] = getCategory(groups.at(i));
    }

    // Заполняем категории для каждого провайдера
    m_ProviderCategorys[Providers::AutodetectID] = 101;

    QDom_NodeList providers = m_Document.elementsByTagName(CGroupModel::Operator);
    for (int i = 0; i < providers.count(); i++) {
        Item item(providers.at(i));

        qint64 id = item.getId();

        qint64 category = getCategory(providers.at(i));

        if (category) {
            m_ProviderCategorys.insert(id, category);
        }
    }

    setRootElementInternal(0);

    emit endResetModel();
}

//------------------------------------------------------------------------------
QStringList GroupModel::getElementFilter() {
    return m_ElementFilter;
}

//------------------------------------------------------------------------------
void GroupModel::setElementFilter(QStringList aFilter) {
    m_ElementFilter = aFilter;

    emit beginResetModel();

    setRootElementInternal(m_RootElement);

    emit endResetModel();
}

//------------------------------------------------------------------------------
qint64 GroupModel::getCategory(QDom_Node aNode) {
    if (aNode.isNull()) {
        // Не нашли вышестоящей корневой группы
        return 0;
    }

    Item i(aNode);

    if (i.is(CGroupModel::Group) && i.getType() == CGroupModel::RootGroupType) {
        return i.getId();
    }

    return getCategory(aNode.parentNode());
}

//------------------------------------------------------------------------------
qint64 GroupModel::getRootElement() const {
    return m_RootElement;
}

//------------------------------------------------------------------------------
void GroupModel::clearNodes() {
    while (!m_NodesObject.isEmpty()) {
        auto item = m_NodesObject.takeFirst();
        if (item) {
            item->deleteLater();
        }
    }

    m_Nodes.clear();
}

//------------------------------------------------------------------------------
const GroupModel::Item_List &GroupModel::getItem_List(qint64 aGroupID) {
    if (m_NodesCache.contains(aGroupID)) {
        return m_NodesCache[aGroupID];
    }

    Item_List result;
    QDom_NodeList nodes = m_Groups[aGroupID].childNodes();

    for (int i = 0; i < nodes.count(); i++) {
        result << QSharedPointer<Item>(new Item(nodes.at(i)));
    }

    m_NodesCache.insert(aGroupID, result);

    // заполняем order сразу и больше не трогаем
    if (!m_ProvidersStatistic.isEmpty()) {
        for (int i = 0; i < m_NodesCache[aGroupID].size(); i++) {
            Item_Ptr &item = m_NodesCache[aGroupID][i];

            if (item->is(CGroupModel::Operator)) {
                item->setOrder(m_ProvidersStatistic.value(item->getId(), 0));
            } else if (item->is(CGroupModel::Group) || item->is(CGroupModel::GroupLink)) {
                item->setOrder(getGroupOrder(item->getId()));
            }
        }
    }

    return m_NodesCache[aGroupID];
}

//------------------------------------------------------------------------------
void GroupModel::setRootElementInternal(qint64 aRootElement) {
    clearNodes();

    QStringList filter = m_ElementFilter;

    if (filter.isEmpty()) {
        if (!aRootElement) {
            filter << CGroupModel::Operator << CGroupModel::GroupLink;
        } else {
            // перечислим все валидные теги
            filter << CGroupModel::Operator << CGroupModel::Group << CGroupModel::GroupLink;
        }
    }

    int currentCount = m_Nodes.count();

    foreach (auto item, getItem_List(aRootElement)) {
        if (filter.contains(item->getElementName())) {
            m_Nodes.push_back(item);
        }
    }

    if (aRootElement && !m_ProvidersStatistic.isEmpty()) {
        qStableSort(
            m_Nodes.begin(), m_Nodes.end(), [](const Item_Ptr &aItem_A, const Item_Ptr &aItem_B) -> bool {
                return aItem_A->getOrder() > aItem_B->getOrder();
            });
    }

    if (m_Nodes.count() > currentCount) {
        emit rowCountChanged();
    }

    m_RootElement = aRootElement;
}

//------------------------------------------------------------------------------
quint32 GroupModel::getGroupOrder(qint64 aGroupID) {
    auto lessOrder = [](const Item_Ptr &aItem_A, const Item_Ptr &aItem_B) -> bool {
        return aItem_A->getOrder() < aItem_B->getOrder();
    };

    auto items = getItem_List(aGroupID);

    auto it = std::max_element(items.begin(), items.end(), lessOrder);

    return it != items.end() ? it->data()->getOrder() : 0;
}

//------------------------------------------------------------------------------
void GroupModel::setRootElement(qint64 aRootElement) {
    if (aRootElement != m_RootElement && m_Groups.contains(aRootElement)) {
        emit beginResetModel();

        setRootElementInternal(aRootElement);

        emit endResetModel();

        if (m_Groups.contains(aRootElement) && m_CurrentCategory != m_Categories[aRootElement]) {
            m_CurrentCategory = m_Categories[aRootElement];
            emit categoryChanged();
        }
    }
}

//------------------------------------------------------------------------------
qint64 GroupModel::getCategory() const {
    return m_CurrentCategory;
}

//------------------------------------------------------------------------------
QString GroupModel::getCategoryName() const {
    return m_Groups[m_CurrentCategory].toElement().attribute("name");
}

//------------------------------------------------------------------------------
QVariant GroupModel::data(const QModelIndex &aIndex, int aRole) const {
    if (aIndex.row() >= 0 && aIndex.row() < m_Nodes.count()) {
        switch (aRole) {
        case IdRole:
            return m_Nodes[aIndex.row()]->getId();
        case NameRole:
            return m_Nodes[aIndex.row()]->getName();
        case TitleRole:
            return m_Nodes[aIndex.row()]->getTitle();
        case DescriptionRole:
            return m_Nodes[aIndex.row()]->getDescription();
        case TypeRole:
            return m_Nodes[aIndex.row()]->getType();
        case ImageRole:
            return m_Nodes[aIndex.row()]->getImage();
        case IsGroupRole:
            return m_Nodes[aIndex.row()]->isGroup();
        case JSONRole:
            return m_Nodes[aIndex.row()]->getJSON();
        }
    }

    return QVariant();
}

//------------------------------------------------------------------------------
qint64 GroupModel::findCategory(qint64 aProviderId) const {
    if (m_ProviderCategorys.contains(aProviderId)) {
        return m_ProviderCategorys[aProviderId];
    }

    return 0;
}

//------------------------------------------------------------------------------
bool GroupModel::isProviderInCategory(qint64 aProvider, qint64 aCategory) const {
    return aCategory == findCategory(aProvider);
}

//------------------------------------------------------------------------------
QSet<qint64> GroupModel::allProviders() const {
    return m_ProviderCategorys.keys().toSet();
}

//------------------------------------------------------------------------------
QString GroupModel::getProviderName(qint64 aProviderId) const {
    QDom_NodeList providers = m_Document.elementsByTagName(CGroupModel::Operator);

    for (int i = 0; i < providers.count(); i++) {
        Item item(providers.at(i));

        if (item.getId() == aProviderId) {
            return item.getName();
        }
    }

    return QString();
}

//------------------------------------------------------------------------------
void GroupModel::setStatistic(QMap<qint64, quint32> &aStatistic) {
    m_ProvidersStatistic.swap(aStatistic);
}

//------------------------------------------------------------------------------
Item::Item(const QDom_Node &aNode)
    : m_Attributes(aNode.attributes()),
      m_IsGroup(aNode.nodeName().contains(CGroupModel::Group, Qt::CaseInsensitive) == true),
      m_ElementName(aNode.nodeName().toLower()), m_Order(0) {}

//------------------------------------------------------------------------------
QString Item::getElementName() const {
    return m_ElementName;
}

//------------------------------------------------------------------------------
qint64 Item::getId() const {
    qint64 id = m_Attributes.namedItem(CGroupModel::Attributes::Id).nodeValue().toLongLong();
    qint64 extId = m_Attributes.namedItem(CGroupModel::Attributes::ExtId).nodeValue().toLongLong();

    return extId ? extId : id;
}

//------------------------------------------------------------------------------
QString Item::getName() const {
    return m_Attributes.namedItem(CGroupModel::Attributes::Name).nodeValue();
}

//------------------------------------------------------------------------------
QString Item::getTitle() const {
    return m_Attributes.namedItem(CGroupModel::Attributes::Title).nodeValue();
}

//------------------------------------------------------------------------------
QString Item::getDescription() const {
    return m_Attributes.namedItem(CGroupModel::Attributes::Description).nodeValue();
}

//------------------------------------------------------------------------------
QString Item::getType() const {
    return m_Attributes.namedItem(CGroupModel::Attributes::Type).nodeValue().toLower();
}

//------------------------------------------------------------------------------
QString Item::getImage() const {
    return m_Attributes.namedItem(CGroupModel::Attributes::Image).nodeValue();
}

//------------------------------------------------------------------------------
bool Item::isGroup() const {
    return m_IsGroup;
}

//------------------------------------------------------------------------------
QString Item::getJSON() const {
    return m_Attributes.namedItem(CGroupModel::Attributes::JSON).nodeValue();
}

//------------------------------------------------------------------------------
void Item::setOrder(quint32 aOrder) {
    m_Order = aOrder;
}

//------------------------------------------------------------------------------
quint32 Item::getOrder() const {
    return m_Order;
}

//------------------------------------------------------------------------------
bool Item::is(const QString &aElementName) const {
    return m_ElementName == aElementName;
}

//------------------------------------------------------------------------------
Item_Object::Item_Object(const Item &aItem, QObject *aParent) : QObject(aParent), m_Item(aItem) {}

//------------------------------------------------------------------------------
qint64 Item_Object::getId() const {
    return m_Item.getId();
}

//------------------------------------------------------------------------------
QString Item_Object::getName() const {
    return m_Item.getName();
}

//------------------------------------------------------------------------------
QString Item_Object::getTitle() const {
    return m_Item.getTitle();
}

//------------------------------------------------------------------------------
QString Item_Object::getDescription() const {
    return m_Item.getDescription();
}

//------------------------------------------------------------------------------
QString Item_Object::getType() const {
    return m_Item.getType();
}

//------------------------------------------------------------------------------
QString Item_Object::getImage() const {
    return m_Item.getImage();
}

//------------------------------------------------------------------------------
QString Item_Object::getJSON() const {
    return m_Item.getJSON();
}

//------------------------------------------------------------------------------
bool Item_Object::isGroup() const {
    return m_Item.isGroup();
}

//------------------------------------------------------------------------------

/* @file Графический элемент QML. */

#include "QMLGraphicsItem.h"

#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtQuick/QQuickItem>

#include <Common/ILog.h>

namespace CQMLGraphicsItem {
const char ItemKey[] = "item";

const char ShowHandlerName[] = "showHandler";
const char ShowHandlerSignature[] = "showHandler()";

const char HideHandlerName[] = "hideHandler";
const char HideHandlerSignature[] = "hideHandler()";

const char ResetHandlerName[] = "resetHandler";
const char ResetHandlerSignature[] = "resetHandler(const QVariant &)";

const char NotifyHandlerName[] = "notifyHandler";
const char NotifyHandlerSignature[] = "notifyHandler(const QVariant &, const QVariant &)";
} // namespace CQMLGraphicsItem

//---------------------------------------------------------------------------
QMLGraphicsItem::QMLGraphicsItem(const SDK::GUI::GraphicsItem_Info &aInfo,
                                 QQmlEngine *aEngine,
                                 ILog *aLog)
    : m_Log(aLog), m_Engine(aEngine), m_Item(nullptr), m_Info(aInfo) {
    QString qmlPath = QDir::toNativeSeparators(QDir::cleanPath(
        aInfo.directory + QDir::separator() + aInfo.parameters[CQMLGraphicsItem::ItemKey]));
    QQmlComponent component(
        m_Engine, qmlPath.startsWith("qrc") ? QUrl(qmlPath) : QUrl::fromLocalFile(qmlPath));

    QObject *object = component.create();
    if (object) {
        m_Item = QSharedPointer<QQuickItem>(qobject_cast<QQuickItem *>(object));
    } else {
        foreach (QQmlError error, component.errors()) {
            m_Error += error.toString() + "\n";
        }
    }
}

//---------------------------------------------------------------------------
void QMLGraphicsItem::show() {
    if (m_Item->metaObject()->indexOfMethod(CQMLGraphicsItem::ShowHandlerSignature) != -1) {
        QMetaObject::invokeMethod(
            m_Item.data(), CQMLGraphicsItem::ShowHandlerName, Qt::DirectConnection);
    }
}

//---------------------------------------------------------------------------
void QMLGraphicsItem::hide() {
    if (m_Item->metaObject()->indexOfMethod(CQMLGraphicsItem::HideHandlerSignature) != -1) {
        QMetaObject::invokeMethod(
            m_Item.data(), CQMLGraphicsItem::HideHandlerName, Qt::DirectConnection);
    }
}

//---------------------------------------------------------------------------
void QMLGraphicsItem::reset(const QVariantMap &aParameters) {
    if (m_Item->metaObject()->indexOfMethod(
            QMetaObject::normalizedSignature(CQMLGraphicsItem::ResetHandlerSignature)) != -1) {
        QVariant error;
        QMetaObject::invokeMethod(m_Item.data(),
                                  CQMLGraphicsItem::ResetHandlerName,
                                  Qt::DirectConnection,
                                  Q_RETURN_ARG(QVariant, error),
                                  Q_ARG(const QVariant &, QVariant::fromValue(aParameters)));

        if (!error.isNull()) {
            m_Log->write(LogLevel::Error, translateError(error));
        }
    }
}

//---------------------------------------------------------------------------
void QMLGraphicsItem::notify(const QString &aEvent, const QVariantMap &aParameters) {
    if (m_Item->metaObject()->indexOfMethod(
            QMetaObject::normalizedSignature(CQMLGraphicsItem::NotifyHandlerSignature)) != -1) {
        QVariant error;
        QMetaObject::invokeMethod(m_Item.data(),
                                  CQMLGraphicsItem::NotifyHandlerName,
                                  Qt::DirectConnection,
                                  Q_RETURN_ARG(QVariant, error),
                                  Q_ARG(const QVariant &, QVariant::fromValue(aEvent)),
                                  Q_ARG(const QVariant &, QVariant::fromValue(aParameters)));

        if (!error.isNull()) {
            m_Log->write(LogLevel::Error, translateError(error));
        }
    }
}

//---------------------------------------------------------------------------
QQuickItem *QMLGraphicsItem::getWidget() const {
    return m_Item.data();
}

//---------------------------------------------------------------------------
QVariantMap QMLGraphicsItem::getContext() const {
    return m_Info.context;
}

//---------------------------------------------------------------------------
bool QMLGraphicsItem::isValid() const {
    return !m_Item.isNull();
}

//---------------------------------------------------------------------------
QString QMLGraphicsItem::getError() const {
    return m_Error;
}

//---------------------------------------------------------------------------
QString QMLGraphicsItem::translateError(const QVariant &aError) const {
    auto e(aError.value<QVariantMap>());

    return QString("%1:%2 %3")
        .arg(e["fileName"].toString())
        .arg(e["lineNumber"].toString())
        .arg(e["message"].toString());
}

//---------------------------------------------------------------------------

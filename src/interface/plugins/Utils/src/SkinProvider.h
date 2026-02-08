/* @file Плагин для отрисовки изображений интерфейса пользователя */

#pragma once

#include <QtCore/QHash>
#include <QtQuick/QQuickImageProvider>

class Skin;

//------------------------------------------------------------------------------
class SkinProvider : public QObject, public QQuickImageProvider {
    Q_OBJECT

public:
    SkinProvider(const QString &aInterfacePath,
                 const QString &aLogoPath,
                 const QString &aUserLogoPath,
                 const Skin *aSkin);
    ~SkinProvider() {};

    virtual QImage requestImage(const QString &aId, QSize *aSize, const QSize &aRequestedSize);

private:
    QString getImagePath(const QString &aImageId) const;

private:
    QString m_LogoPath;
    QString m_UserLogoPath;
    const Skin *m_Skin;
    QString m_InterfacePath;

    QHash<QString, QImage> m_Backgrounds;
    QHash<QString, QImage> m_Logos;
};

//------------------------------------------------------------------------------

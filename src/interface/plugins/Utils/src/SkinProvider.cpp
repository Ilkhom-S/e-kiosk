/* @file Плагин для отрисовки изображений интерфейса пользователя */

#include "SkinProvider.h"

#include <QtCore/QDir>
#include <QtCore/QSettings>
#include <QtGui/QPainter>
#include <QtGui/QTextLayout>
#include <QtQuick/QQuickImageProvider>

#include "Log.h"
#include "Skin.h"

SkinProvider::SkinProvider(const QString &aInterfacePath,
                           const QString &aContentPath,
                           const QString &aUserPath,
                           const Skin *aSkin)
    : QQuickImageProvider(QQmlImageProviderBase::Image), m_LogoPath(aContentPath + "/logo"),
      m_UserLogoPath(aUserPath + "/logo"), m_InterfacePath(aInterfacePath), m_Skin(aSkin) {}

//------------------------------------------------------------------------------
QImage SkinProvider::requestImage(const QString &aId, QSize *aSize, const QSize &aRequestedSize) {
    QString skinName = m_Skin->getName();

    if (!aId.contains("logoprovider")) {
        QString path = getImagePath(aId);

        Log(Log::Debug) << QString("SkinProvider: LOAD texture %1 from '%2'.").arg(aId).arg(path);

        QImage image;

        if (image.load(path)) {
            if (aRequestedSize.isValid()) {
                // TODO
                image = image.scaled(aRequestedSize);
            }

            *aSize = image.size();

            return image;
        } else {
            Log(Log::Error)
                << QString("SkinProvider: failed to load texture %1 from '%2'.").arg(aId).arg(path);
        }
    }
    // Логотипы предварительно обработаем перед выдачей
    else {
        QString id = aId.section("/", 1, 1);
        QString background = aId.section("/", 2, 2);
        QString label = aId.section("/", 3);

        QHash<QString, QImage>::iterator ilogo = m_Logos.find(aId);
        if (ilogo != m_Logos.end()) {
            return *ilogo;
        }

        QHash<QString, QImage>::iterator bimage =
            background.isEmpty() ? m_Backgrounds.end() : m_Backgrounds.find(background);

        // Загружаем фон
        if (bimage == m_Backgrounds.end() && !background.isEmpty()) {
            QString path = getImagePath(background);
            QImage img;

            if (img.load(path)) {
                bimage = m_Backgrounds.insert(background, img);
            } else {
                Log(Log::Error)
                    << QString("SkinProvider: failed to load logo background '%1' from '%2'.")
                           .arg(background)
                           .arg(path);
            }
        }

        // Сперва логотипы киберплата, затем пользователские
        QImage logo(m_LogoPath + QDir::separator() + id + ".png");
        if (logo.isNull()) {
            logo = QImage(m_UserLogoPath + QDir::separator() + id + ".png");
        }

        QImage image(aRequestedSize.isValid() ? aRequestedSize : logo.size(),
                     QImage::Format_ARGB32);

        if (bimage != m_Backgrounds.end()) {
            image = *bimage;
        } else {
            image.fill(qRgba(0, 0, 0, 0));
        }

        QPainter painter(&image);

        if (!logo.isNull()) {
            painter.drawImage(
                QPoint((image.width() - logo.width()) / 2, (image.height() - logo.height()) / 2),
                logo);
        } else {
            QTextOption option(Qt::AlignHCenter);
            option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);

            QFont font("Roboto Condensed");
            font.setPixelSize(16);
            font.setBold(true);

            QString title = label.isEmpty() ? id : label;
            title.replace(title.indexOf('"'), 1, QChar(0x00AB));
            title.replace(title.lastIndexOf('"'), 1, QChar(0x00BB));
            title = title.toUpper();

            QTextLayout layout(title);
            layout.setTextOption(option);
            layout.setFont(font);
            layout.beginLayout();

            // Нарезаем строки
            qreal y = 0;
            QTextLine line = layout.createLine();

            while (line.isValid()) {
                line.setLineWidth(image.width() - 44);
                line.setPosition(QPointF(0, y));
                y += line.height();

                if (layout.lineCount() == 3) {
                    break;
                }

                line = layout.createLine();
            }

            layout.endLayout();
            layout.draw(&painter,
                        QPointF(22, (image.rect().height() - layout.boundingRect().height()) / 2));
        }

        painter.end();
        *aSize = image.size();

        m_Logos.insert(aId, image);

        return image;
    }

    return QImage();
}

//------------------------------------------------------------------------------
QString SkinProvider::getImagePath(const QString &aImageId) const {
    QVariantMap skinConfig = m_Skin->getConfiguration();
    QVariantMap::const_iterator it = skinConfig.find(aImageId);

    return it != skinConfig.end() ? it->toString() : "";
}

//------------------------------------------------------------------------------

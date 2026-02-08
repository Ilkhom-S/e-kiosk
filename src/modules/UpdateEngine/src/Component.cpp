/* @file Компонент дистрибутива. */

#include "Component.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>

#include <Common/Exception.h>

#include "Misc.h"

Component::Component(const QString &aId,
                     const QString &aVersion,
                     const TFileList &aFiles,
                     const QStringList &aActions,
                     const QString &aURL)
    : mPostActions(aActions), mURL(aURL), mId(aId), m_Version(aVersion), mSkipExisting(false),
      mOptional(false) {
    QRegularExpression removeFirstSlash("^/+");

    // Удяляем слэши в начали пути.
    foreach (auto file, aFiles) {
        mFiles.insert(File(
            QString(file.name()).remove(removeFirstSlash), file.hash(), file.url(), file.size()));
    }
}

//---------------------------------------------------------------------------
Component::~Component() {}

//---------------------------------------------------------------------------
TFileList Component::getFiles() const {
    return mFiles;
}

//---------------------------------------------------------------------------
QString Component::getId() const {
    return mId;
}

//---------------------------------------------------------------------------
QString Component::getVersion() const {
    return m_Version;
}

//---------------------------------------------------------------------------
QString Component::getTemporaryFolder() const {
    QDir dir(QDir::tempPath() + QDir::separator() + "Humo." + mId + "." + m_Version + ".temp");

    if (!dir.exists()) {
        if (!dir.mkpath(dir.path())) {
            throw Exception(ECategory::Application,
                            ESeverity::Major,
                            0,
                            QString("Failed to create path %1.").arg(dir.path()));
        }
    }

    return dir.absolutePath();
}

//---------------------------------------------------------------------------
QString Component::getURL(const File &aFile, const QString &aDefaultUrl) const {
    if (!aFile.url().isEmpty()) {
        return aFile.url();
    }

    return getURL(aFile.name(), aDefaultUrl);
}

//---------------------------------------------------------------------------
QString Component::getURL(const QString &aFileName, const QString &aDefaultUrl) const {
    if (!mURL.isEmpty()) {
        return mURL + "/" + aFileName;
    }

    return aDefaultUrl + "/" + aFileName;
}

//---------------------------------------------------------------------------
QStringList Component::getPostActions() const {
    return mPostActions;
}

//---------------------------------------------------------------------------
void Component::setSkipExisting(bool aSkipExisting) {
    mSkipExisting = aSkipExisting;
}

//---------------------------------------------------------------------------
bool Component::skipExisting() const {
    return mSkipExisting;
}

//---------------------------------------------------------------------------
void Component::setOptional(bool aOptional) {
    mOptional = aOptional;
}

//---------------------------------------------------------------------------
bool Component::optional() const {
    return mOptional;
}

//---------------------------------------------------------------------------

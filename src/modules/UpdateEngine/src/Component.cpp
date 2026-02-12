/* @file Компонент дистрибутива. */

#include "Component.h"

#include <QtCore/QCryptographicHash>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QRegularExpression>

#include <Common/Exception.h>

#include <utility>

#include "Misc.h"

Component::Component(
    QString aId, QString aVersion, const TFileList &aFiles, QStringList aActions, QString aURL)
    : m_PostActions(std::move(aActions)), m_URL(std::move(aURL)), m_Id(std::move(aId)),
      m_Version(std::move(aVersion)), m_SkipExisting(false), m_Optional(false) {
    QRegularExpression removeFirstSlash("^/+");

    // Удяляем слэши в начали пути.
    foreach (auto file, aFiles) {
        m_Files.insert(File(
            QString(file.name()).remove(removeFirstSlash), file.hash(), file.url(), file.size()));
    }
}

//---------------------------------------------------------------------------
Component::~Component() = default;

//---------------------------------------------------------------------------
TFileList Component::getFiles() const {
    return m_Files;
}

//---------------------------------------------------------------------------
QString Component::getId() const {
    return m_Id;
}

//---------------------------------------------------------------------------
QString Component::getVersion() const {
    return m_Version;
}

//---------------------------------------------------------------------------
QString Component::getTemporaryFolder() const {
    QDir dir(QDir::tempPath() + QDir::separator() + "Humo." + m_Id + "." + m_Version + ".temp");

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
    if (!m_URL.isEmpty()) {
        return m_URL + "/" + aFileName;
    }

    return aDefaultUrl + "/" + aFileName;
}

//---------------------------------------------------------------------------
QStringList Component::getPostActions() const {
    return m_PostActions;
}

//---------------------------------------------------------------------------
void Component::setSkipExisting(bool aSkipExisting) {
    m_SkipExisting = aSkipExisting;
}

//---------------------------------------------------------------------------
bool Component::skipExisting() const {
    return m_SkipExisting;
}

//---------------------------------------------------------------------------
void Component::setOptional(bool aOptional) {
    m_Optional = aOptional;
}

//---------------------------------------------------------------------------
bool Component::optional() const {
    return m_Optional;
}

//---------------------------------------------------------------------------

/* @file Базовый класс скачиваемого компонента. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QStringList>

#include "File.h"

class NetworkTask;

//---------------------------------------------------------------------------
namespace CComponent {
inline const char *OptionalTask() {
    return "OptionalTaskProperty";
}
} // namespace CComponent

//---------------------------------------------------------------------------
class Component : public QObject {
    Q_OBJECT

public:
    Component(const QString &aId,
              const QString &aVersion,
              const TFileList &aFiles,
              const QStringList &aActions,
              const QString &aURL);
    virtual ~Component();

    /// Получение полного списка файлов.
    virtual TFileList getFiles() const;

    /// Выполнить действие.
    virtual void applyPostActions(const QString &aWorkingDir) noexcept(false) = 0;

    /// Произвести установку файлов aFiles в каталог aDestination.
    virtual void deploy(const TFileList &aFiles, const QString &aDestination) noexcept(false) = 0;

    /// Производит закачку компонента, находящего по aBaseURL во временную папку.
    virtual QList<NetworkTask *> download(const QString &aBaseURL,
                                          const TFileList &aExceptions) = 0;

    /// Имя компонента.
    QString getId() const;

    /// Имя временной папки.
    QString getTemporaryFolder() const;

    /// Получить URL компонента, если задан.
    QString getURL(const File &aFile, const QString &aDefaultUrl) const;

    /// Собрать URL компоненты
    QString getURL(const QString &aFileName, const QString &aDefaultUrl) const;

    /// Версия.
    QString getVersion() const;

    /// Файлы, которые надо выполнить после установки.
    QStringList getPostActions() const;

    /// Установить флаг пропуска обновления существующих файлов
    void setSkipExisting(bool aSkipExisting);

    /// Пропуск обновления существующих файлов
    bool skipExisting() const;

    /// Необязательность обновления
    void setOptional(bool aOptional);
    bool optional() const;

private:
    QString m_Id;
    TFileList m_Files;
    QStringList m_PostActions;
    QString m_URL;
    QString m_Version;
    bool m_Optional;

protected:
    bool m_SkipExisting;
};

//---------------------------------------------------------------------------

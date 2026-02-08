/* @file Класс файл в компоненте дистрибутива. */

#pragma once

#include <QtCore/QDir>
#include <QtCore/QSet>
#include <QtCore/QString>

/// Структура описывает параметры файла обновления
class File {
public:
    typedef enum {
        OK,                 // уже скачан, качать не нужно
        NotFullyDownloaded, // нужно докачать
        Error               // нужно удалить и качать заново
    } Result;

public:
    File();
    File(const QString &aName, const QString &aHash, const QString &aUrl, qint64 aSize = 0);

    bool operator==(const File &aFile) const;

    /// Проверить, нужно ли скачивать файл
    Result verify(const QString &aTempFilePath);

public:
    const QString &name() const { return m_Name; }
    const QString &url() const { return m_Url; }
    const QString &hash() const { return m_Hash; }
    qint64 size() const { return m_Size; }

    // Возвращает корневую папку файла, или само имя файла
    QString dir() const { return QDir::from_NativeSeparators(m_Name).split("/").at(0); }

private:
    QString m_Name; // путь относительно корня дистрибутива
    QString m_Hash; // sha256 хеш файла
    qint64 m_Size;  // Размер файла
    QString m_Url;  // Опциональный параметр - адрес для скачивания
};

inline uint qHash(const File &aFile) {
    uint h1 = qHash(aFile.name());
    uint h2 = qHash(aFile.hash());
    return ((h1 << 16) | (h1 >> 16)) ^ h2;
}

// Список файлов.
typedef QSet<File> TFileList;

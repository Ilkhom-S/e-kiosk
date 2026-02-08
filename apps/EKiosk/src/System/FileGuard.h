/* @file Сторож бэкап-файлов. */

#pragma once

#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QPair>

namespace System {

//----------------------------------------------------------------------------
class FileGuard {
    typedef QPair<QString, QString> TStringPair;

public:
    //----------------------------------------------------------------------------
    // Конструктор
    FileGuard() { m_FileList.clear(); }

    //----------------------------------------------------------------------------
    // Деструктор - восстанавливает файлы из бэкапа
    ~FileGuard() {
        for (int i = 0; i < m_FileList.size(); i++) {
            TStringPair curPair = m_FileList.at(i);

            if (QFile::exists(curPair.first)) {
                if (!QFile::remove(curPair.first)) {
                    continue;
                }
            }

            QFile::rename(curPair.second, curPair.first);
        }
    }

    //----------------------------------------------------------------------------
    // Добавляет пару имён файла, для восстановления
    void addFile(const QString &aOldName, const QString &aNewName) {
        m_FileList.append(TStringPair(aOldName, aNewName));
    }

    //----------------------------------------------------------------------------
    // Очищает список файлов для восстановления
    void release() { m_FileList.clear(); }

private:
    QList<TStringPair> m_FileList;
};

//----------------------------------------------------------------------------

} // namespace System

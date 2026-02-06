#ifndef DEALERLOCALDATA_H
#define DEALERLOCALDATA_H

#include <QtCore/QMap>
#include <QtCore/QString>

//------------------------------------------------------------------------------
class DealerLocalData {
public:
    DealerLocalData();

    bool loadInfo(const QString &aFileName);

    QString getFirstField() const;

    /// Получить имя параметра, по которому будем искать запись в таблице
    QList<QPair<QString, QString>> getColumns() const;

    /// Найти запись по значению первого столбца
    bool findNumber(const QString &aFirstColumnValue, QMap<QString, QString> &aParameters);

private:
    QString mFilePath;

    QList<QPair<QString, QString>> mColumns;
};

//------------------------------------------------------------------------------
#endif // DEALERLOCALDATA_H

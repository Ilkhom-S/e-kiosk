/* @file Класс для перевода строк внутри QML модулей. */

#pragma once

#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>

//------------------------------------------------------------------------------
class Translator : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString language READ getLanguage NOTIFY languageChanged);
    Q_PROPERTY(QString defaultLanguage READ getDefaultLanguage CONSTANT FINAL);

public:
    Translator(const QString &aInterfacePath);

public slots:
    /// Перевести строку
    QString tr(const QString &aString);

    /// Установить язык интерфейса.
    void setLanguage(const QString &aLanguage);

    /// Получить текущий язык интерфейса.
    QString getLanguage() const;

    /// Получить язык по умолчанию
    QString getDefaultLanguage() const;

    /// Получить список доступных языков.
    QStringList getLanguageList() const;

signals:
    void languageChanged();

private:
    QString m_InterfacePath;

    // Список всех поддерживаемых языков <наименование, локализованное название языка>.
    QMap<QString, QString> m_Languages;

    // Набор трансляторов для каждого модуля.
    QMap<QString, QTranslator *> m_Translators;

    QString m_CurrentLanguage;
    QString m_DefaultLanguage;
};

//------------------------------------------------------------------------------

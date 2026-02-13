/* @file Базовый класс приложения. */

#pragma once

#include <type_traits>

//--------------------------------------------------------------------------------
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>

#include <Common/ILog.h>

class ILog;              // forward declaration for logging interface (project-specific)
class SingleApplication; // forward declaration

//--------------------------------------------------------------------------------
/// Класс абстрактного приложения. Реализует полезные функции (загрузка
/// конфигурации).
class BasicApplication {
public:
    /// Конструктор инициализирует имя и версию приложения и принимает
    /// аргументы командной строки (для распознавания `test` режимов и пр.)
    explicit BasicApplication(const QString &aName = QString(),
                              QString aVersion = QString(),
                              int aArgumentCount = 0,
                              char **aArguments = nullptr);
    virtual ~BasicApplication();

    /// Возвращает имя приложения.
    virtual QString getName() const;

    /// Возвращает версию приложения.
    virtual QString getVersion() const;

    /// Возвращает имя исполняемого файла.
    QString getFileName() const;

    /// Возвращает тип/версию операционной системы.
    static QString getOSVersion();

    /// Возвращает рабочий каталог приложения (может быть задан в .ini файле).
    virtual QString getWorkingDirectory() const;

    /// Возвращает текущий язык приложения (en, ru, tg, uz и т.д.).
    virtual QString getLanguage() const;

    /// Возвращает настройки приложения
    virtual QSettings &getSettings() const;

    /// Возвращает лог приложения. Uses the ILog interface from the Log module
    /// for file-based logging with daily rotation.
    virtual ILog *getLog() const;

    /// Выводит стандартный заголовок в лог (вызывается после создания QApplication).
    void writeLogHeader();

    /// Возвращает экземпляр приложения.
    static BasicApplication *getInstance();

    /// Runtime helpers
    bool isTestMode() const;

    static bool startDetachedProcess(const QString &program,
                                     const QStringList &args = QStringList());

protected:
    void setInstance();

    // Singleton instance pointer
    static BasicApplication *m_Instance;

    // Protected members accessible to derived classes and templates
    QString m_Language;                   // initialized in constructor (in template)
    QScopedPointer<QSettings> m_Settings; // initialized in constructor

private:
    // Internal helpers
    void detectTestMode();

    QString m_Name;
    QString m_Version;
    QString m_WorkingDirectory;
    int m_ArgumentCount;
    char **m_Arguments;
    ILog *m_Log = nullptr; // initialized in constructor

    // Runtime state
    bool m_TestMode = false;
};

//---------------------------------------------------------------------------
/// Класс приложения, основанного на QCoreApplication/QApplication/QSingleApplication. Вдобавок к
/// базе загружает локализацию.
template <class T> class BasicQtApplication : public BasicApplication {
public:
    /// Тип Qt приложения. Может быть QCoreApplication/QApplication/QSingleApplication.
    typedef T TApplication;

    BasicQtApplication(const QString &aName,
                       const QString &aVersion,
                       int &aArgumentCount,
                       char **aArguments);
    virtual ~BasicQtApplication() override = default;

    /// Запускает цикл обработки событий.
    int exec() { return m_QtApplication.exec(); }

    /// Возвращает аргументы командной строки.
    QStringList getArguments() const { return m_QtApplication.arguments(); }

    /// Возвращает экземпляр Qt приложения.
    TApplication &getQtApplication() { return m_QtApplication; }

private:
    TApplication m_QtApplication;
    QSharedPointer<QTranslator> m_Translator;
};

//---------------------------------------------------------------------------
// Реализация BasicQtApplication

#include <Common/BasicApplication.tpp>

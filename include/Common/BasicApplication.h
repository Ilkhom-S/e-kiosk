/* @file Базовый класс приложения. */

#pragma once

//--------------------------------------------------------------------------------
// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QProcess>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>
#include <QtWidgets/QApplication>
#include <Common/QtHeadersEnd.h>

class ILog;              // forward declaration for logging interface (project-specific)
class SingleApplication; // forward declaration

//--------------------------------------------------------------------------------
/// Класс абстрактного приложения. Реализует полезные функции (загрузка
/// конфигурации).
class BasicApplication {
  public:
    /// Конструктор инициализирует имя и версию приложения и принимает
    /// аргументы командной строки (для распознавания `test` режимов и пр.)
    explicit BasicApplication(const QString &aName = QString(), const QString &aVersion = QString(),
                              int aArgumentCount = 0, char **aArguments = nullptr);
    virtual ~BasicApplication();

    /// Возвращает имя приложения.
    virtual QString getName() const;

    /// Возвращает версию приложения.
    virtual QString getVersion() const;

    /// Возвращает имя исполняемого файла.
    QString getFileName() const;

    /// Возвращает тип/версию операционной системы.
    QString getOSVersion() const;

    /// Возвращает рабочий каталог приложения (может быть задан в .ini файле).
    virtual QString getWorkingDirectory() const;

    /// Возвращает настройки приложения
    virtual QSettings &getSettings() const;

    /// Возвращает лог приложения. Uses the ILog interface from the Log module
    /// for file-based logging with daily rotation.
    virtual ILog *getLog() const;

    /// Возвращает экземпляр приложения.
    static BasicApplication *getInstance();

    /// Runtime helpers
    bool isTestMode() const;
    bool isPrimaryInstance() const;
    bool startDetachedProcess(const QString &program, const QStringList &args = QStringList());

  protected:
    void setInstance();

  private:
    // Internal helpers
    void detectTestMode();

    QString m_name;
    QString m_version;
    QString m_workingDirectory;
    QScopedPointer<QSettings> m_settings; // initialized in constructor
    int m_argumentCount;
    char **m_arguments;
    ILog *m_log = nullptr; // initialized in constructor

    // SingleApplication for single-instance
    QScopedPointer<SingleApplication> m_singleApp;

    // Runtime state
    bool m_testMode = false;

    // Singleton instance pointer
    static BasicApplication *s_instance;
};

//---------------------------------------------------------------------------
/// Класс приложения, основанного на QCoreApplication/QApplication/QSingleApplication. Вдобавок к базе загружает
/// локализацию.
template <class T> class BasicQtApplication : public BasicApplication {
  public:
    /// Тип Qt приложения. Может быть QCoreApplication/QApplication/QSingleApplication.
    typedef T TApplication;

    BasicQtApplication(const QString &aName, const QString &aVersion, int &aArgumentCount, char **aArguments);
    virtual ~BasicQtApplication() override {
    }

    /// Запускает цикл обработки событий.
    int exec();

    /// Возвращает аргументы командной строки.
    QStringList getArguments() const;

    /// Возвращает экземпляр Qt приложения.
    TApplication &getQtApplication();

  private:
    TApplication mQtApplication;
    QScopedPointer<QTranslator> mTranslator;
};

//---------------------------------------------------------------------------
// Реализация BasicQtApplication

template <typename T>
BasicQtApplication<T>::BasicQtApplication(const QString &aName, const QString &aVersion, int &aArgumentCount,
                                          char **aArguments)
    : BasicApplication(aName, aVersion, aArgumentCount, aArguments), mQtApplication(aArgumentCount, aArguments) {
    mQtApplication.setApplicationName(aName);
    mQtApplication.setApplicationVersion(aVersion);

    QFileInfo fileInfo(mQtApplication.applicationFilePath());
    QDir translations(getWorkingDirectory(), QString("%1_*.qm").arg(fileInfo.baseName()));

    if (translations.count()) {
        QString translation = translations.entryInfoList().first().absoluteFilePath();
        mTranslator.reset(new QTranslator(&mQtApplication));

        if (mTranslator->load(translation)) {
            mQtApplication.installTranslator(mTranslator.data());
            getLog()->write(LogLevel::Normal, QString("Translation %1 loaded.").arg(translation));
        } else {
            getLog()->write(LogLevel::Warning, QString("Failed to load translation %1.").arg(translation));
        }
    }
}

//---------------------------------------------------------------------------
template <typename T> int BasicQtApplication<T>::exec() {
    return mQtApplication.exec();
}

//---------------------------------------------------------------------------
template <typename T> QStringList BasicQtApplication<T>::getArguments() const {
    return mQtApplication.arguments();
}

//---------------------------------------------------------------------------
template <typename T> T &BasicQtApplication<T>::getQtApplication() {
    return mQtApplication;
}

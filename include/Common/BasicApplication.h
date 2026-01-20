/* @file Базовый класс приложения. */

#pragma once

#include <type_traits>

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
                              int aArgumentCount = 0, char **aArguments = nullptr, bool aUseSingleApp = true);
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
    virtual bool isPrimaryInstance() const;
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
    bool m_useSingleApp = true;

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

    /// Возвращает true, если это первичный экземпляр приложения.
    virtual bool isPrimaryInstance() const override {
        if (std::is_same<T, SingleApplication>::value) {
            return static_cast<const SingleApplication &>(mQtApplication).isPrimary();
        } else {
            return BasicApplication::isPrimaryInstance();
        }
    }

    /// Запускает цикл обработки событий.
    int exec() {
        return mQtApplication.exec();
    }

    /// Возвращает аргументы командной строки.
    QStringList getArguments() const {
        return mQtApplication.arguments();
    }

    /// Возвращает экземпляр Qt приложения.
    TApplication &getQtApplication() {
        return mQtApplication;
    }

  private:
    TApplication mQtApplication;
    QScopedPointer<QTranslator> mTranslator;
};

//---------------------------------------------------------------------------
// Реализация BasicQtApplication

template <typename T>
BasicQtApplication<T>::BasicQtApplication(const QString &aName, const QString &aVersion, int &aArgumentCount,
                                          char **aArguments)
    : BasicApplication(aName, aVersion, aArgumentCount, aArguments, !std::is_same<T, SingleApplication>::value),
      mQtApplication(aArgumentCount, aArguments) {
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

#pragma once

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QProcess>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <Common/QtHeadersEnd.h>

class ILog; // forward declaration for logging interface (project-specific)
class SingleApplication; // forward declaration

/// Класс абстрактного приложения. Реализует полезные функции (загрузка
/// конфигурации).
class BasicApplication {
public:
  /// Конструктор инициализирует имя и версию приложения и принимает
  /// аргументы командной строки (для распознавания `test` режимов и пр.)
  BasicApplication(const QString &aName = QString(),
                   const QString &aVersion = QString(), int aArgumentCount = 0,
                   char **aArguments = nullptr);
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

  /// Возвращает лог приложения. NOTE: actual `ILog` implementation is not
  /// present in this repository. Keep this pointer nullable and implement
  /// concrete logging in a separate module when ported.
  virtual ILog *getLog() const;

  /// Возвращает экземпляр приложения.
  static BasicApplication *getInstance();

  /// Runtime helpers
  bool isTestMode() const;
  bool isPrimaryInstance() const;
  bool startDetachedProcess(const QString &program,
                            const QStringList &args = QStringList());

protected:
  void setInstance();

private:
  // Internal helpers
  void detectTestMode();

  QString m_name;
  QString m_version;
  mutable QScopedPointer<QSettings> m_settings; // lazy-initialised
  mutable ILog *m_log = nullptr;                // optional project logger

  // SingleApplication for single-instance
  QScopedPointer<SingleApplication> m_singleApp;

  // Runtime state
  bool m_testMode = false;

  // Singleton instance pointer
  static BasicApplication *s_instance;
};

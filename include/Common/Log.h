#pragma once

// Simple QFile-based logger inspired by TerminalClient's logging module.
// This logger is intentionally minimal and memory-friendly compared to log4cpp.

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QAtomicInt>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QMutex>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>
#include <Common/QtHeadersEnd.h>

namespace ek {

enum class LogLevel { Debug = 0, Info, Warning, Error };

class QFileLogger {
public:
  QFileLogger() = default;
  ~QFileLogger();

  // Initialise logger with file path, maxSize in bytes and maxFiles for
  // rotation
  bool init(const QString &filePath, qint64 maxSize = 10 * 1024 * 1024,
            int maxFiles = 5);

  void setLevel(LogLevel level) { m_level = static_cast<int>(level); }
  LogLevel level() const {
    return static_cast<LogLevel>(m_level.loadRelaxed());
  }

  void log(LogLevel level, const QString &msg);

private:
  void rotateIfNeeded();

  QString m_filePath;
  QFile m_file;
  qint64 m_maxSize = 10 * 1024 * 1024;
  int m_maxFiles = 5;
  QMutex m_mutex;
  QAtomicInt m_level{static_cast<int>(LogLevel::Info)};
};

// Global singleton helper wrapping QFileLogger
class Log {
public:
  static bool init(const QString &filePath, LogLevel level = LogLevel::Info,
                   qint64 maxSize = 10 * 1024 * 1024, int maxFiles = 5);

  static void setLevel(LogLevel level);

  static void debug(const QString &msg) { log(LogLevel::Debug, msg); }
  static void info(const QString &msg) { log(LogLevel::Info, msg); }
  static void warn(const QString &msg) { log(LogLevel::Warning, msg); }
  static void error(const QString &msg) { log(LogLevel::Error, msg); }

  static void log(LogLevel level, const QString &msg);

private:
  static QScopedPointer<QFileLogger> s_logger;
};

} // namespace ek

// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <Common/QtHeadersEnd.h>

// Modules
#include "Common/Log.h"

namespace ek {

QFileLogger::~QFileLogger() {
  QMutexLocker locker(&m_mutex);
  if (m_file.isOpen()) {
    m_file.flush();
    m_file.close();
  }
}

bool QFileLogger::init(const QString &filePath, qint64 maxSize, int maxFiles) {
  QMutexLocker locker(&m_mutex);
  m_filePath = filePath;
  m_maxSize = maxSize;
  m_maxFiles = maxFiles;

  QDir d = QFileInfo(m_filePath).dir();
  if (!d.exists())
    d.mkpath(".");

  m_file.setFileName(m_filePath);
  if (!m_file.open(QIODevice::Append | QIODevice::Text)) {
    return false;
  }
  return true;
}

void QFileLogger::rotateIfNeeded() {
  if (!m_file.isOpen())
    return;
  if (m_file.size() < m_maxSize)
    return;

  m_file.close();

  // Rotate files: logfile -> logfile.1, logfile.1 -> logfile.2, ...
  for (int i = m_maxFiles - 1; i >= 0; --i) {
    QString src =
        (i == 0) ? m_filePath : QString("%1.%2").arg(m_filePath).arg(i);
    QString dst = QString("%1.%2").arg(m_filePath).arg(i + 1);
    if (QFile::exists(src)) {
      // remove oldest
      if (i + 1 >= m_maxFiles) {
        QFile::remove(src);
      } else {
        QFile::rename(src, dst);
      }
    }
  }

  m_file.setFileName(m_filePath);
  m_file.open(QIODevice::WriteOnly | QIODevice::Text);
}

void QFileLogger::log(LogLevel level, const QString &msg) {
  if (static_cast<int>(level) < m_level.loadRelaxed())
    return;

  QMutexLocker locker(&m_mutex);
  if (!m_file.isOpen()) {
    // try to (re)open using default path
    m_file.setFileName(m_filePath);
    if (!m_file.open(QIODevice::Append | QIODevice::Text))
      return;
  }

  rotateIfNeeded();

  QTextStream s(&m_file);
  QString ts = QDateTime::currentDateTime().toString(Qt::ISODate);
  QString levelStr;
  switch (level) {
  case LogLevel::Debug:
    levelStr = "DEBUG";
    break;
  case LogLevel::Info:
    levelStr = "INFO";
    break;
  case LogLevel::Warning:
    levelStr = "WARN";
    break;
  case LogLevel::Error:
    levelStr = "ERROR";
    break;
  }

  s << ts << " [" << levelStr << "] " << msg << "\n";
  s.flush();
}

// Log singleton
QScopedPointer<QFileLogger> Log::s_logger;

bool Log::init(const QString &filePath, LogLevel level, qint64 maxSize,
               int maxFiles) {
  s_logger.reset(new QFileLogger());
  if (!s_logger->init(filePath, maxSize, maxFiles)) {
    s_logger.reset();
    return false;
  }
  s_logger->setLevel(level);
  return true;
}

void Log::setLevel(LogLevel level) {
  if (s_logger)
    s_logger->setLevel(level);
}

void Log::log(LogLevel level, const QString &msg) {
  if (!s_logger)
    return; // logger not initialised
  s_logger->log(level, msg);
}

} // namespace ek

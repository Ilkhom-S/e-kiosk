#pragma once

// Common includes and small helpers for EKiosk.
// Place common header includes and lightweight project-wide helpers here.

#include "QtHeadersBegin.h"
#include <QtGlobal>
#include <QString>
#include <QDebug>
#include "QtHeadersEnd.h"

// Example: centralize frequently used macros or inline helpers
#ifndef EKISK_DEBUG
#define EKISK_DEBUG(...) qDebug() << __VA_ARGS__
#endif

// Add other commonly-used includes or forward declarations here.

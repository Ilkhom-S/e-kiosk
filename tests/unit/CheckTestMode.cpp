// Qt
#include <Common/QtHeadersBegin.h>
#include <QtCore/QTextStream>
#include <Common/QtHeadersEnd.h>

// Modules
#include <Common/BasicApplication.h>

int main(int argc, char **argv) {
  BasicApplication app(QString(), QString(), argc, argv);
  QTextStream out(stdout);
  out << (app.isTestMode() ? "1" : "0");
  return 0;
}

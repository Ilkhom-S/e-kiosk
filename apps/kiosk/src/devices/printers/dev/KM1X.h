#ifndef KM1X_H
#define KM1X_H

#include "../AbstractPrinter.h"

class BasePrinterDevices;

namespace CMDKM1X {
const int charTimeOut = 50;  /// Time out
const QString DeviceName = "KM1X";

const uchar PrinterCommandFirstByte = 0x1B;  /// Первая часть команды.
}  // namespace CMDKM1X

class KM1X_PRINTER : public BasePrinterDevices {
  public:
    KM1X_PRINTER(QObject* parent = 0);

    QString printer_name;
    QString port_speed;

    bool OpenPrinterPort();
    bool isEnabled();
    bool isItYou();
    void print(const QString& aCheck);

  protected:
    bool openPort();
    bool printCheck(const QString& aCheck);

    bool initialize();
    bool cut();
    bool feed(int aCount);
    void dispense();
    void getSpecialCharecters(QByteArray& printText);
};

#endif  // KM1X_H

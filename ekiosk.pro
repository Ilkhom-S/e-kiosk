QT  += core gui widgets
QT  += sql
QT  += serialport
QT  += webkitwidgets
QT  += websockets
QT  += printsupport
QT  += network
QT  += multimedia
QT  += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14

requires qt 5.0

TARGET = ../EKiosk

#win32:LIBS += librasapi32
#win32:LIBS += winspool
#win32:LIBS += -lpsapi -lmingw32

LIBS += -lrasapi32
LIBS += -lwinspool
LIBS += -lpsapi
LIBS += -luser32
LIBS += -lAdvapi32


# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TMP_DIR = ./release
CONFIG(debug, debug|release) {
  TMP_DIR = ./debug
}

UI_DIR       = $$TMP_DIR
RCC_DIR      = $$TMP_DIR
MOC_DIR      = $$TMP_DIR
OBJECTS_DIR  = $$TMP_DIR
INCLUDEPATH += $$PWD/src ./ $$TMP_DIR
DEPENDPATH  += $$PWD/src

SOURCES += \
    src/connection/CheckConnection.cpp \
    src/connection/Connect.cpp \
    src/connection/RasConnection.cpp \
    src/devices/ClassDevice.cpp \
    src/devices/coinacceptor/AbstractAcceptor.cpp \
    src/devices/coinacceptor/ClassAcceptor.cpp \
    src/devices/coinacceptor/dev/CCTalk.cpp \
    src/devices/modems/ClassModem.cpp \
    src/devices/modems/protocol/ATProtocol.cpp \
    src/devices/modems/protocol/qatresult.cpp \
    src/devices/modems/protocol/qatresultparser.cpp \
    src/devices/modems/protocol/qatutils.cpp \
    src/devices/modems/protocol/qgsmcodec.cpp \
    src/devices/printers/AbstractPrinter.cpp \
    src/devices/printers/ClassPrinter.cpp \
    src/devices/printers/dev/AV268.cpp \
    src/devices/printers/dev/CitizenCBM1000.cpp \
    src/devices/printers/dev/CitizenCTS2000.cpp \
    src/devices/printers/dev/CitizenPPU700.cpp \
    src/devices/printers/dev/CustomTG2480.cpp \
    src/devices/printers/dev/CustomVKP80.cpp \
    src/devices/printers/dev/KM1X.cpp \
    src/devices/printers/dev/Phoenix.cpp \
    src/devices/printers/dev/StarTUP900.cpp \
    src/devices/validators/AbstractValidator.cpp \
    src/devices/validators/ClassValidator.cpp \
    src/devices/validators/dev/CCNetSM.cpp \
    src/devices/validators/dev/EBDS.cpp \
    src/devices/watchdogs/watchdogs.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/modules/AuthRequest.cpp \
    src/modules/CheckOnline.cpp \
    src/modules/CollectDaemons.cpp \
    src/modules/CommandConfirm.cpp \
    src/modules/ConfirmOtp.cpp \
    src/modules/GetBalanceAgent.cpp \
    src/modules/GetServices.cpp \
    src/modules/JsonRequest.cpp \
    src/modules/PayDaemons.cpp \
    src/modules/SendLogInfo.cpp \
    src/modules/SendOtp.cpp \
    src/modules/SendReceipt.cpp \
    src/modules/SendRequest.cpp \
    src/modules/StatusDaemons.cpp \
    src/modules/UserDaemons.cpp \
    src/other/receipt.cpp \
    src/other/utils.cpp \
    src/ui/adminbutton.cpp \
    src/ui/admindialog.cpp \
    src/ui/avtorizationtoadminin.cpp \
    src/ui/changepassword.cpp \
    src/ui/createdialupconnection.cpp \
    src/ui/incasaciyaform.cpp \
    src/ui/keypud.cpp \
    src/ui/loadinggprsform.cpp \
    src/ui/mainpageloader.cpp \
    src/ui/registrationdialog.cpp \
    src/ui/registrationform.cpp \
    src/ui/searchdevicesform.cpp \
    src/ui/selectcategorylogview.cpp \
    src/updater/textprogressbar.cpp \
    src/updater/update.cpp

HEADERS += \
    src/connection/CheckConnection.h \
    src/connection/Connect.h \
    src/connection/RasConnection.h \
    src/db/sqlconnection.h \
    src/devices/ClassDevice.h \
    src/devices/ConstantData.h \
    src/devices/coinacceptor/AbstractAcceptor.h \
    src/devices/coinacceptor/ClassAcceptor.h \
    src/devices/coinacceptor/dev/CCTalk.h \
    src/devices/modems/ClassModem.h \
    src/devices/modems/protocol/ATProtocol.h \
    src/devices/modems/protocol/qatresult.h \
    src/devices/modems/protocol/qatresultparser.h \
    src/devices/modems/protocol/qatutils.h \
    src/devices/modems/protocol/qgsmcodec.h \
    src/devices/printers/AbstractPrinter.h \
    src/devices/printers/ClassPrinter.h \
    src/devices/printers/dev/AV268.h \
    src/devices/printers/dev/CitizenCBM1000.h \
    src/devices/printers/dev/CitizenCTS2000.h \
    src/devices/printers/dev/CitizenPPU700.h \
    src/devices/printers/dev/CustomTG2480.h \
    src/devices/printers/dev/CustomVKP80.h \
    src/devices/printers/dev/KM1X.h \
    src/devices/printers/dev/Phoenix.h \
    src/devices/printers/dev/StarTUP900.h \
    src/devices/validators/AbstractValidator.h \
    src/devices/validators/ClassValidator.h \
    src/devices/validators/dev/CCNetFirmware.h \
    src/devices/validators/dev/CCNetSM.h \
    src/devices/validators/dev/CashPayment.h \
    src/devices/validators/dev/EBDS.h \
    src/devices/watchdogs/watchdogs.h \
    src/main.h \
    src/mainwindow.h \
    src/modules/AuthRequest.h \
    src/modules/CheckOnline.h \
    src/modules/CollectDaemons.h \
    src/modules/CommandConfirm.h \
    src/modules/ConfirmOtp.h \
    src/modules/GetBalanceAgent.h \
    src/modules/GetServices.h \
    src/modules/JsonRequest.h \
    src/modules/PayDaemons.h \
    src/modules/SendLogInfo.h \
    src/modules/SendOtp.h \
    src/modules/SendReceipt.h \
    src/modules/SendRequest.h \
    src/modules/StatusDaemons.h \
    src/modules/UserDaemons.h \
    src/other/jsInterface.h \
    src/other/logClean.h \
    src/other/logger.h \
    src/other/loggerValidator.h \
    src/other/receipt.h \
    src/other/systemInfo.h \
    src/other/utils.h \
    src/ui/adminbutton.h \
    src/ui/admindialog.h \
    src/ui/avtorizationtoadminin.h \
    src/ui/changepassword.h \
    src/ui/createdialupconnection.h \
    src/ui/incasaciyaform.h \
    src/ui/keypud.h \
    src/ui/loadinggprsform.h \
    src/ui/mainpageloader.h \
    src/ui/registrationdialog.h \
    src/ui/registrationform.h \
    src/ui/searchdevicesform.h \
    src/ui/selectcategorylogview.h \
    src/updater/CopyFile.h \
    src/updater/textprogressbar.h \
    src/updater/update.h

FORMS += \
    src/mainwindow.ui \
    src/ui/adminbutton.ui \
    src/ui/admindialog.ui \
    src/ui/avtorizationtoadminin.ui \
    src/ui/changepassword.ui \
    src/ui/createdialupconnection.ui \
    src/ui/incasaciyaform.ui \
    src/ui/keypud.ui \
    src/ui/loadinggprsform.ui \
    src/ui/mainpageloader.ui \
    src/ui/registrationdialog.ui \
    src/ui/registrationform.ui \
    src/ui/searchdevicesform.ui \
    src/ui/selectcategorylogview.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ekiosk.qrc

win32:RC_FILE   += version.rc

/* @file Реализация общих функций сетевого соединения для macOS. */

#include <QtCore/QStringList>
#include <QtNetwork/QNetworkInterface>

#include <Connection/IConnection.h>
#include <Connection/NetworkError.h>

ILog *IConnection::mLog = nullptr;

//--------------------------------------------------------------------------------
/// Создание экземпляра соединения.
IConnection *IConnection::create(const QString &aName,
                                 EConnectionTypes::Enum aType,
                                 NetworkTaskManager *aNetwork,
                                 ILog *aLog) {
    IConnection::mLog = aLog;

    // macOS doesn't support dial-up connections like Windows
    // Return nullptr for dial-up, or a basic connection for other types
    Q_UNUSED(aName)
    Q_UNUSED(aType)
    Q_UNUSED(aNetwork)
    Q_UNUSED(aLog)

    // For now, return nullptr as dial-up is not supported
    return nullptr;
}

//--------------------------------------------------------------------------------
/// Поиск всех установленных в системе модемов.
/// macOS typically doesn't have traditional modems, so return empty list.
QStringList IConnection::getModems() {
    return QStringList();
}

//--------------------------------------------------------------------------------
/// Получить информацию о модеме.
/// macOS doesn't have traditional modems.
QString IConnection::getModemInfo(const QString &aName) {
    Q_UNUSED(aName)
    return QString();
}

//--------------------------------------------------------------------------------
/// Поиск всех установленных в системе сетевых интерфейсов.
QStringList IConnection::getInterfaces() {
    QStringList interfaces;

    foreach (QNetworkInterface intf, QNetworkInterface::allInterfaces()) {
        interfaces.append(intf.humanReadableName());
    }

    return interfaces;
}

//--------------------------------------------------------------------------------
/// Список всех удалённых соединений в системе.
/// macOS doesn't have RAS-like remote connections.
QStringList IConnection::getRemoteConnections() {
    return QStringList();
}

//--------------------------------------------------------------------------------
/// Список всех локальных соединений в системе.
/// macOS doesn't have RAS-like local connections.
QStringList IConnection::getLocalConnections() {
    return QStringList();
}

//--------------------------------------------------------------------------------
/// Создать dialup соединение.
/// Not supported on macOS.
void IConnection::createDialupConnection(const QString &aName,
                                         const QString &aPhone,
                                         const QString &aLogin,
                                         const QString &aPassword,
                                         const QString &aDevice) {
    Q_UNUSED(aName)
    Q_UNUSED(aPhone)
    Q_UNUSED(aLogin)
    Q_UNUSED(aPassword)
    Q_UNUSED(aDevice)

    throw NetworkError(
        ECategory::Network, ESeverity::Major, 0, "Dial-up connections are not supported on macOS");
}

//--------------------------------------------------------------------------------
/// Удалить dialup соединение.
/// Not supported on macOS.
void IConnection::removeDialupConnection(const QString &aName) {
    Q_UNUSED(aName)

    throw NetworkError(
        ECategory::Network, ESeverity::Major, 0, "Dial-up connections are not supported on macOS");
}
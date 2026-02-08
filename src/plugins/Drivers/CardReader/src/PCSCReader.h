/* @file Реализация интерфейса работы с PC/SC API. */
#pragma once

// windows
#ifdef __APPLE__
#include <PCSC/winscard.h>
#include <PCSC/wintypes.h>
#else
#include <winscard.h>
#pragma comment(lib, "Winscard.lib")
#endif

#include <QtCore/QObject>
#include <QtCore/QSet>
#include <QtCore/QStringList>

#include <Common/ILogable.h>

#include "Hardware/Common/BaseStatusTypes.h"

//--------------------------------------------------------------------------------
class PCSCReader : public QObject, public ILogable {
public:
    PCSCReader();
    ~PCSCReader();

    /// Сброс карты по питанию.
    virtual bool reset(QByteArray &aAnswer);

    /// Получить список кардридеров
    QStringList getReaderList();

    /// Подключиться к ридеру
    bool connect(const QString &aReaderName);

    /// Проверка подключены ли мы к ридеру
    bool isConnected() const;

    /// Обменяться с картой или ридером пакетами APDU
    bool communicate(const QByteArray &aRequest, QByteArray &aResponse);

    /// Отключится от ридера
    void disconnect(bool aEject);

    /// Получить последние статус-коды
    TStatusCodes getStatusCodes();

private:
    /// Обработка результата выполнения функции работы с ридером/картой/SAM-модулем
    bool handleResult(const QString &aFunctionName, HRESULT aResultCode);

    /// Контекст карты
    SCARDCONTEXT m_Context;

    /// Хендл карты
    SCARDHANDLE m_Card;

    /// Используемый протокол
    unsigned m_ActiveProtocol;

    /// Заголовок протокольного запроса к ридеру
    SCARD_IO_REQUEST m_PioSendPci;

    /// Статус-коды результатов операций
    TStatusCodes m_StatusCodes;
};

//--------------------------------------------------------------------------------

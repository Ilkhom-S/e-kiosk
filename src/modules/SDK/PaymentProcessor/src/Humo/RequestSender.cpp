/* @file Класс для обмена сообщениями по http. */

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QStringConverter>
#include <QtCore/QStringDecoder>
#include <QtCore/QStringEncoder>
#else
#include <QtCore/QTextCodec>
#endif

#include <Common/ScopedPointerLaterDeleter.h>

#include <SDK/PaymentProcessor/Humo/Request.h>
#include <SDK/PaymentProcessor/Humo/RequestSender.h>
#include <SDK/PaymentProcessor/Humo/Response.h>

#include <Crypt/ICryptEngine.h>
#include <NetworkTaskManager/MemoryDataStream.h>
#include <NetworkTaskManager/NetworkTaskManager.h>
#include <functional>

using namespace std::placeholders;

namespace SDK {
namespace PaymentProcessor {
namespace Humo {

//---------------------------------------------------------------------------
namespace CRequestSender {
const int DefaultKeyPair = 0;
} // namespace CRequestSender

//---------------------------------------------------------------------------
RequestSender::RequestSender(NetworkTaskManager *aNetwork, ICryptEngine *aCryptEngine)
    : m_Network(aNetwork), m_CryptEngine(aCryptEngine), m_KeyPair(CRequestSender::DefaultKeyPair),
      m_OnlySecureConnection(true) {
#if defined(_DEBUG) || defined(DEBUG_INFO)
    m_OnlySecureConnection = false;
#endif // _DEBUG || DEBUG_INFO

    setResponseCreator([this](auto &&pH1, auto &&pH2) {
        return defaultResponseCreator(std::forward<decltype(pH1)>(pH1),
                                      std::forward<decltype(pH2)>(pH2));
    });
    setRequestEncoder([this](auto &&pH1, auto &&pH2) {
        return defaultRequestEncoder(std::forward<decltype(pH1)>(pH1),
                                     std::forward<decltype(pH2)>(pH2));
    });
    setResponseDecoder([this](auto &&pH1, auto &&pH2) {
        return defaultResponseDecoder(std::forward<decltype(pH1)>(pH1),
                                      std::forward<decltype(pH2)>(pH2));
    });
    setRequestSigner([this](auto &&pH1, auto &&pH2, auto &&pH3, auto &&pH4) {
        return defaultRequestSigner(std::forward<decltype(pH1)>(pH1),
                                    std::forward<decltype(pH2)>(pH2),
                                    std::forward<decltype(pH3)>(pH3),
                                    std::forward<decltype(pH4)>(pH4));
    });
    setResponseVerifier([this](auto &&pH1, auto &&pH2, auto &&pH3, auto &&pH4) {
        return defaultResponseVerifier(std::forward<decltype(pH1)>(pH1),
                                       std::forward<decltype(pH2)>(pH2),
                                       std::forward<decltype(pH3)>(pH3),
                                       std::forward<decltype(pH4)>(pH4));
    });
    setRequestModifier([this](auto &&pH1, auto &&pH2, auto &&pH3) {
        return defaultRequestModifier(std::forward<decltype(pH1)>(pH1),
                                      std::forward<decltype(pH2)>(pH2),
                                      std::forward<decltype(pH3)>(pH3));
    });
}

//---------------------------------------------------------------------------
RequestSender::~RequestSender() = default;

//---------------------------------------------------------------------------
void RequestSender::setNetworkTaskManager(NetworkTaskManager *aNetwork) {
    m_Network = aNetwork;
}

//---------------------------------------------------------------------------
void RequestSender::setResponseCreator(const TResponseCreator &aResponseCreator) {
    m_ResponseCreator = aResponseCreator;
}

//---------------------------------------------------------------------------
void RequestSender::setRequestEncoder(const TRequestEncoder &aRequestEncoder) {
    m_RequestEncoder = aRequestEncoder;
}

//---------------------------------------------------------------------------
void RequestSender::setResponseDecoder(const TResponseDecoder &aResponseDecoder) {
    m_ResponseDecoder = aResponseDecoder;
}

//---------------------------------------------------------------------------
void RequestSender::setRequestSigner(const TRequestSigner &aRequestSigner) {
    m_RequestSigner = aRequestSigner;
}

//---------------------------------------------------------------------------
void RequestSender::setResponseVerifier(const TResponseVerifier &aResponseVerifier) {
    m_ResponseVerifier = aResponseVerifier;
}

//---------------------------------------------------------------------------
void RequestSender::setRequestModifier(const TRequestModifier &aRequestModifier) {
    m_RequestModifier = aRequestModifier;
}

//---------------------------------------------------------------------------
void RequestSender::setCryptKeyPair(int aKeyPair) {
    m_KeyPair = aKeyPair;
}

//---------------------------------------------------------------------------
void RequestSender::setOnlySecureConnectionEnabled(bool aOnlySecure) {
    m_OnlySecureConnection = aOnlySecure;
}

//---------------------------------------------------------------------------
Response *RequestSender::request(NetworkTask::Type aType,
                                 const QUrl &aUrl,
                                 Request &aRequest,
                                 ESignatureType aSignatureType,
                                 ESendError &aError,
                                 int aTimeout) {
    if (aUrl.scheme().toLower() == "http" && m_OnlySecureConnection) {
        aError = HttpIsNotSupported;

        return nullptr;
    }

    aError = Ok;

    if (m_Network.isNull()) {
        aError = NoNetworkInterfaceSpecified;

        return nullptr;
    }

    // Подставляем серийный номер пары ключа в каждый запрос.
    QString keyPairSerial = m_CryptEngine->getKeyPairSerialNumber(m_KeyPair);

    if (keyPairSerial.isEmpty()) {
        aError = ClientCryptError;

        return nullptr;
    }

    if (aSignatureType == Solid) {
        aRequest.addParameter("ACCEPT_KEYS", keyPairSerial);
    }

    QScopedPointer<NetworkTask, ScopedPointerLaterDeleter<NetworkTask>> task(new NetworkTask());

    task->setUrl(aUrl);
    task->setType(aType);
    task->setTimeout(aTimeout);
    task->setDataStream(new MemoryDataStream());
    task->getRequestHeader().insert("Content-Type", "application/x-www-form-urlencoded");

    QByteArray encodedRequest;
    QByteArray signedRequest;
    QByteArray detachedSignature;

    if (!m_RequestEncoder(aRequest.toString(), std::ref(encodedRequest))) {
        aError = EncodeError;

        return nullptr;
    }

    if (!m_RequestSigner(
            encodedRequest, std::ref(signedRequest), aSignatureType, std::ref(detachedSignature))) {
        aError = ClientCryptError;

        return nullptr;
    }

    if (aSignatureType == Solid) {
        signedRequest = "inputmessage=" + signedRequest;

        const auto &rawParameters = aRequest.getParameters(true);
        foreach (auto name, rawParameters.keys()) {
            signedRequest += ("\n" + name + "=" + rawParameters.value(name).toString()).toUtf8();
        }
    } else if (aSignatureType == Detached) {
        task->getRequestHeader().insert("X-signature", detachedSignature);
        task->getRequestHeader().insert("X-humo-accepted-keys", keyPairSerial.toLatin1());
    } else {
        aError = UnknownSignatureType;

        return nullptr;
    }

    if (!m_RequestModifier(aRequest, std::ref(task->getRequestHeader()), std::ref(signedRequest))) {
        aError = RequestModifyError;

        return nullptr;
    }

    task->getDataStream()->write(signedRequest);

    m_Network->addTask(task.data());

    task->waitForFinished();

    if (task->getError() != NetworkTask::NoError) {
        aError = NetworkError;

        return nullptr;
    }

    QByteArray signedResponseData = task->getDataStream()->takeAll();

    // Проверим на запакованные данные
    if (task->getResponseHeader()["Content-Type"] == "application/x-gzip") {
        signedResponseData =
            qUncompress(reinterpret_cast<const uchar *>(signedResponseData.constData()),
                        signedResponseData.size());
    }

    QByteArray encodedResponseData;
    QByteArray responseSignature;
    QString responseData;

    if (aSignatureType == Detached) {
        responseSignature =
            QByteArray::fromPercentEncoding(task->getResponseHeader()["X-signature"]);
    }

    if (!m_ResponseVerifier(
            signedResponseData, std::ref(encodedResponseData), aSignatureType, responseSignature)) {
        aError = ServerCryptError;

        return nullptr;
    }

    if (!m_ResponseDecoder(encodedResponseData, std::ref(responseData))) {
        aError = DecodeError;

        return nullptr;
    }

    return m_ResponseCreator(aRequest, responseData);
}

//---------------------------------------------------------------------------
Response *RequestSender::get(const QUrl &aUrl,
                             Request &aRequest,
                             ESignatureType aSignatureType,
                             ESendError &aError,
                             int aTimeout) {
    return request(NetworkTask::Get, aUrl, aRequest, aSignatureType, aError, aTimeout);
}

//---------------------------------------------------------------------------
Response *RequestSender::post(const QUrl &aUrl,
                              Request &aRequest,
                              ESignatureType aSignatureType,
                              ESendError &aError,
                              int aTimeout) {
    return request(NetworkTask::Post, aUrl, aRequest, aSignatureType, aError, aTimeout);
}

//---------------------------------------------------------------------------
Response *RequestSender::defaultResponseCreator(const Request &aRequest, const QString &aData) {
    return new Response(aRequest, aData);
}

//---------------------------------------------------------------------------
bool RequestSender::defaultRequestEncoder(const QString &aRequest, QByteArray &aEncodedRequest) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringEncoder encoder(QStringConverter::System, QStringConverter::Flag::Default);
    aEncodedRequest = encoder.encode(aRequest);
    return !encoder.hasError();
#else
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    if (!codec) {
        return false;
    }

    aEncodedRequest = codec->fromUnicode(aRequest);
    return true;
#endif
}

//---------------------------------------------------------------------------
bool RequestSender::defaultResponseDecoder(const QByteArray &aResponse, QString &aDecodedResponse) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QStringDecoder decoder(QStringConverter::System, QStringConverter::Flag::Default);
    aDecodedResponse = decoder.decode(aResponse);
    return !decoder.hasError();
#else
    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    if (!codec) {
        return false;
    }

    aDecodedResponse = codec->toUnicode(aResponse);
    return true;
#endif
}

//---------------------------------------------------------------------------
bool RequestSender::defaultRequestSigner(const QByteArray &aRequest,
                                         QByteArray &aSignedRequest,
                                         ESignatureType aSignatureType,
                                         QByteArray &aSignature) {
    QString error;

    switch (aSignatureType) {
    case Solid: {
        if (m_CryptEngine->sign(m_KeyPair, aRequest, aSignedRequest, error)) {
            aSignedRequest = aSignedRequest.toPercentEncoding();

            return true;
        }
        return false;
    }

    case Detached: {
        if (m_CryptEngine->signDetach(m_KeyPair, aRequest, aSignature, error)) {
            aSignedRequest = aRequest;
            aSignature = aSignature.toPercentEncoding();

            return true;
        }
        return false;
    }

    default:
        return false;
    }
}

//---------------------------------------------------------------------------
bool RequestSender::defaultResponseVerifier(const QByteArray &aSignedResponse,
                                            QByteArray &aResponse,
                                            ESignatureType aSignatureType,
                                            const QByteArray &aSignature) {
    QString error;

    switch (aSignatureType) {
    case Solid:
        return m_CryptEngine->verify(m_KeyPair, aSignedResponse, aResponse, error);
    case Detached: {
        if (m_CryptEngine->verifyDetach(m_KeyPair, aSignedResponse, aSignature, error)) {
            aResponse = aSignedResponse;

            return true;
        }
        return false;
    }

    default:
        return false;
    }
}

//---------------------------------------------------------------------------
bool RequestSender::defaultRequestModifier(Request & /*aRequest*/,
                                           NetworkTask::TByteMap & /*aHeaders*/,
                                           QByteArray & /*aEncodedAndSignedData*/) {
    return true;
}

//---------------------------------------------------------------------------
QString RequestSender::translateError(ESendError aError) {
    switch (aError) {
    case Ok:
        return "ok";
    case NetworkError:
        return "network error";
    case EncodeError:
        return "request encode error";
    case DecodeError:
        return "response decode error";
    case ClientCryptError:
        return "request sign error";
    case ServerCryptError:
        return "response verify error";
    case UnknownSignatureType:
        return "signature type is not supported";
    case RequestModifyError:
        return "request modify error";
    case NoNetworkInterfaceSpecified:
        return "no network interface specified";
    case HttpIsNotSupported:
        return "only secure connection supported";

    default:
        return "unknown error";
    }
}

//---------------------------------------------------------------------------
} // namespace Humo
} // namespace PaymentProcessor
} // namespace SDK

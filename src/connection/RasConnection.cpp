#define _CRT_SECURE_NO_WARNINGS

#include "RasConnection.h"
#include <QCoreApplication>


QString RasConnection::G_Comment = "";
QString RasConnection::G_Error_Comment = "";
QString RasConnection::G_Error_Num = "";
QString RasConnection::G_State = "";

RasConnection::RasConnection(QObject *parent) : QThread(parent)
{
    Debuger = 1;
    stateDialTimer = new QTimer();
    stateDialTimer->setInterval(100);

    connect(stateDialTimer,SIGNAL(timeout()),this,SLOT(setStateDial()));
    connect(this,SIGNAL(emit_TimerStateDialStart()),stateDialTimer,SLOT(start()));

    reCallTimer = new QTimer();
    reCallTimer->setSingleShot(true);
    connect(reCallTimer,SIGNAL(timeout()),this,SLOT(reCall()));

    connect(this,SIGNAL(emit_ReCallTimer(int)),reCallTimer,SLOT(start(int)));
}


void RasConnection::setConnectionName(QString name)
{
    conName = name;
}

void RasConnection::reCall()
{
    nowCmd = DialupParam::StartDial;
    this->start();
}

void RasConnection::run()
{
    switch(nowCmd){
        case DialupParam::StartDial:
        {
            if(this->Dial()){
                emit this->emit_dialupState(Connection::conStateUpping);
            }else{
                emit this->emit_toLoging(2,"CONNECTION","Dialing return False...");
                emit this->emit_dialupState(Connection::conStateDoun);
            }
        }
        break;
    }
}

void RasConnection::setStateDial()
{
    if(G_State != nowStateDial){

        nowStateDial = G_State;
        if(G_State == "ERROR"){
            emit this->emit_ConnectionError();
            emit this->emit_errorState(G_Error_Num,G_Error_Comment);

            stateDialTimer->stop();
            return;
        }

        emit this->emit_connState(G_State,G_Comment);

        if((nowStateDial == "Logon Complete") || (nowStateDial == "Disconnected"))
            stateDialTimer->stop();

        if(nowStateDial == "Logon Complete"){

            reCallTimer->stop();

            emit this->emit_ConnectionUp();
        }
    }
}

void RasConnection::execCommand(int cmd)
{
    nowCmd = cmd;
    this->start();
}


void WINAPI RasConnection::RasCallback(HRASCONN hrasconn, UINT unMsg, RASCONNSTATE rascs, DWORD dwError, DWORD dwExtendedError)
{
    Q_UNUSED(hrasconn)
    Q_UNUSED(dwExtendedError)
    Q_UNUSED(unMsg)

    QString S = "";
    QString D = "";

    if (dwError) {

        wchar_t buff[256];

        RasGetErrorStringW( dwError, buff, sizeof(buff));

        S = "ERROR";
        G_State =  QString("%1").arg(S);

        G_Error_Num = QString("%1").arg(dwError);
        G_Error_Comment = QString::fromUtf16((ushort*)((buff)));

        return ;
    }


    switch (rascs)
    {
    case RASCS_OpenPort:
        S = "Opening Port...";
        D = "The communication port is about to be opened.";
        D += " (Идёт открытие порта.)";
        break;
    case RASCS_PortOpened:
        S = "Port Opened...";
        D = "The communication port has been opened successfully.";
        D += " (Порт успешно открыт.)";
        break;
    case RASCS_ConnectDevice:
        S = "Connect Device...";
        D = "A device is about to be connected.";
        D += " (Идёт подключение устройств.)";
        break;
    case RASCS_DeviceConnected:
        S = "Device Connected...";
        D = "A device has connected successfully.";
        D += " (Устройства подключены.)";
        break;
    case RASCS_AllDevicesConnected:
        S = "All Devices Connected...";
        D = "All devices in the device chain have successfully connected.";
        D += " (Все устройства подключены.)";
        break;
    case RASCS_Authenticate:
        S = "Authenticate...";
        D = "The authentication process is starting.";
        D += " (Начинаем процесс аутентификации.)";
        break;
    case RASCS_AuthNotify:
        S = "Auth Notify...";
        D = "An authentication event has occurred.";
        D += " (Аутентификация произошла успешно.)";
        break;
    case RASCS_AuthRetry:
        S = "Auth Retry...";
        D = "The client has requested another validation attempt with a new user name/password/domain.";
        D += " (Клиент попросил попытку для проверки другого именем пользователя / пароль / домен.)";
        break;
    case RASCS_AuthCallback:
        S = "Auth Callback...";
        D = "The remote access server has requested a callback number.";
        D += " (Сервер удаленного доступа попросил номер обратного вызова.)";
        break;
    case RASCS_AuthChangePassword:
        S = "Auth Change Password...";
        D = "The client has requested to change the password on the account.";
        D += " (Клиент обратился с просьбой изменить пароль для данного пользователя.)";
        break;
    case RASCS_AuthProject:
        S = "Auth Project...";
        D = "The projection phase is starting.";
        D += " (Регистрация пользователя в сети.)";
        break;
    case RASCS_AuthLinkSpeed:
        S = "Auth Link Speed...";
        D = "The link-speed calculation phase is starting.";
        D += " (Фаза расчета скорость-передачи начинается.)";
        break;
    case RASCS_AuthAck:
        S = "Auth Ackt...";
        D = "An authentication request is being acknowledged.";
        D += " (Запрос на проверку подлинности отправлен.)";
        break;
    case RASCS_ReAuthenticate:
        S = "ReAuthenticate...";
        D = "Reauthentication (after callback) is starting.";
        D += " (Аутентификация (после обратного вызова) начинается.)";
        break;
    case RASCS_Authenticated:
        S = "Authenticated...";
        D = "The client has successfully completed authentication.";
        D += " (Клиент сообщил об успешном завершении аутентификации.)";
        break;
    case RASCS_PrepareForCallback:
        S = "Prepare For Callback...";
        D = "The line is about to disconnect in preparation for callback.";
        D += " (Линии составляет около отключить в рамках подготовки к обратного вызова.)";
        break;
    case RASCS_WaitForModemReset:
        S = "Wait For Modem Reset...";
        D = "The client is delaying in order to give the modem time to reset itself in preparation for callback.";
        D += " (Клиент задерживает, чтобы дать время модему для сброса себя в рамках подготовки к обратного вызова.)";
        break;
    case RASCS_WaitForCallback:
        S = "Wait For Callback...";
        D = "The client is waiting for an incoming call from the remote access server.";
        D += " (Клиент ждет входящего звонка от удаленного доступа к серверу.)";
        break;
    case RASCS_Projected:
        S = "Projected...";
        D = "This state occurs after the RASCS_AuthProject state. It indicates that projection result information is available. You can access the projection result information by calling RasGetProjectionInfo.";
        D += " (Фаза проектирования закончена.)";
        break;
    case RASCS_SubEntryConnected:
        S = "SubEntry Connected...";
        D = "When dialing a multilink phone-book entry, this state indicates that a subentry has been connected during the dialing process. The dwSubEntry parameter of a RasDialFunc2 callback function indicates the index of the subentry.";
        //When the final state of all subentries in the phone-book entry has been determined, the connection state is RASCS_Connected if one or more subentries have been connected successfully.";
        D += " (При наборе многоканальной телефонной книге запись, это состояние указывает, что подменю был связан во время набора номера процесса.)";
        break;
    case RASCS_SubEntryDisconnected:
        S = "SubEntry Disconnected...";
        D = "When dialing a multilink phone-book entry, this state indicates that a subentry has been disconnected during the dialing process. The dwSubEntry parameter of a RasDialFunc2 callback function indicates the index of the subentry.";
        D += " (При наборе многоканальной телефонной книге запись, это состояние указывает, что подменю не был связан во время набора номера процесса.)";
        break;
    case RASCS_Connected:
        S = "Logon Complete";
        D = "The Dialup Connection is up...";
        D += " (Соединение установленно успешно.)";
        break;
    case RASCS_Disconnected:
        S = "Disconnected";
        D = "Disconnected or failed.";
        D += " (Разрыв соединения или ошибка.)";
        break;
    default:
        qDebug() << "Unknown state Ras con =============sssss==========";
    break;
    }


    G_State =  QString("%1").arg(S);
    G_Comment =  QString("%1").arg(D);

    return;
}

void RasConnection::HangUp()
{
    HRASCONN hRasConn = getConnection();

    if(hangUpThis(hRasConn)){
        emit this->emit_toLoging(0,"CONNECTION",QString("Соединение %1 успешно опущено").arg(this->conName));
        this->emit_dialupState(Connection::conStateDoun);
    }

    this->G_Error_Num = "";
    this->G_Error_Comment = "";

}

bool RasConnection::Dial()
{
    if (this->conName == "") {
        emit this->emit_dialupState(Connection::conStateDoun);
        return false;
    }

    emit this->emit_toLoging(0,"CONNECTION","Начинаем подымать соединение с " + this->conName);

    bool dialUp = false;

    emit this->emit_ReCallTimer(180000);

    HRASCONN hRasConn = getConnection();

    if (hangUpThis(hRasConn)) {
        emit this->emit_toLoging(0,"CONNECTION","Соединение успешно опущено");
        emit this->emit_dialupState(Connection::conStateDoun);
    }

    dialUp = true;
    G_Error_Num = "";
    G_Error_Comment = "";

    RASDIALPARAMS rasDialParams;
    rasDialParams.dwSize = sizeof(RASDIALPARAMS);

    memset(rasDialParams.szEntryName, 0, sizeof(rasDialParams.szEntryName));
    strcpy_s(rasDialParams.szEntryName, conName.toLocal8Bit().constData());

    BOOL hasSavedPassword = FALSE;

    if (RasGetEntryDialParams(0, &rasDialParams, &hasSavedPassword) == 0) {
        HRASCONN hrc = 0;
        DWORD dialError = 0;

        rasDialParams.dwSize=sizeof(rasDialParams);

        emit this->emit_TimerStateDialStart();

        dialError = RasDial(0, 0, &rasDialParams, 1, (void*)RasConnection::RasCallback, &hrc);

        if (dialError != 0) {
            if (dialError == 756) {
                emit this->emit_toLoging(2,"CONNECTION","Возникла ошибка 756 при поднятии соединения");
            }

            hangUpThis(hrc);

            wchar_t buff[256];

            RasGetErrorStringW( dialError, buff, sizeof(buff));

            G_State =  QString("ERROR");

            G_Error_Num = QString("%1").arg(dialError);
            G_Error_Comment = QString::fromUtf16((ushort*)((buff)));

            emit this->emit_errorState(G_Error_Num,G_Error_Comment);

            dialUp = false;
        } else {
            dialUp = true;
        }
    } else {
        dialUp = false;
        emit this->emit_toLoging(2,"CONNECTION","RasGetEntryDialParams false");
    }

    return dialUp;
}

int RasConnection::createNewDialupConnection(QString conName, QString devName, QString phone, QString login, QString pass)
{
    DWORD dwCb = 0;
    DWORD dwError;
    RASENTRY  tRasEntry;
    dwCb = sizeof(RASENTRY);
    RASDIALPARAMS rasDialParams;
    rasDialParams.dwSize = sizeof(RASDIALPARAMS);

    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);

    memset(rasDialParams.szEntryName, 0, sizeof(rasDialParams.szEntryName));
    strcpy_s(rasDialParams.szEntryName, conName.toLocal8Bit().constData());

    memset(&tRasEntry, 0, sizeof(tRasEntry));
    tRasEntry.dwSize = sizeof(tRasEntry);

    MultiByteToWideChar(CP_UTF8, 0, phone.toUtf8().constData(), -1, (LPWSTR)tRasEntry.szLocalPhoneNumber, sizeof(tRasEntry.szLocalPhoneNumber)/sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, devName.toUtf8().constData(), -1, (LPWSTR)tRasEntry.szDeviceName, sizeof(tRasEntry.szDeviceName)/sizeof(WCHAR));
    memcpy(tRasEntry.szDeviceType, L"RASDT_Modem", sizeof(tRasEntry.szDeviceType));
    tRasEntry.dwfOptions        = RASEO_ModemLights|RASEO_SecureLocalFiles|RASEO_RemoteDefaultGateway|RASEO_DisableLcpExtensions;
    tRasEntry.dwfOptions2       = RASEO2_Internet|RASEO2_SecureFileAndPrint|RASEO2_SecureClientForMSNet|RASEO2_DisableNbtOverIP|RASEO2_DontNegotiateMultilink;
    tRasEntry.dwfNetProtocols   = RASNP_Ip;     // TCP/IP
    tRasEntry.dwFramingProtocol = RASFP_Ppp;    //PPP
    tRasEntry.dwIdleDisconnectSeconds = RASIDS_Disabled;
    tRasEntry.dwRedialCount = 1;
    tRasEntry.dwType = RASET_Phone;
    tRasEntry.dwEncryptionType = ET_Optional;

    dwError = RasSetEntryProperties(0, rasDialParams.szEntryName, &tRasEntry, sizeof(tRasEntry), NULL, 0);

    if (dwError) {
        emit this->emit_toLoging(2,"CONNECTION","Ошибка при создание соединения");
        return ErrorDialup::rErrorCreateDialupCon;
    }

    rasDialParams.dwSize = sizeof(RASDIALPARAMS);
    LPRASDIALPARAMS lpRasDialParams = &rasDialParams;

    strcpy_s(rasDialParams.szEntryName, conName.toLocal8Bit().constData());
    strcpy_s(rasDialParams.szPhoneNumber, phone.toLocal8Bit().constData());
    strcpy_s(rasDialParams.szUserName, login.toLocal8Bit().constData());
    strcpy_s(rasDialParams.szPassword, pass.toLocal8Bit().constData());

    dwError = RasSetEntryDialParams(0, lpRasDialParams, false);

    if (dwError) {
        emit this->emit_toLoging(2,"CONNECTION","Ошибка при присвоении параметров соединению");
        return ErrorDialup::rErrorSetDialupParam;
    }

    return ErrorDialup::rNoError;
}

bool RasConnection::HasInstalledModems(QStringList &lstModemList)
{

    DWORD dwCb = sizeof(RASDEVINFO);
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwRetries = 5;
    DWORD dwDevices = 0;
    RASDEVINFO* lpRasDevInfo = NULL;
    bool bResult=false;

    while (dwRetries--)
    {
        if (NULL != lpRasDevInfo)
        {
            HeapFree(GetProcessHeap(), 0, lpRasDevInfo);
            lpRasDevInfo = NULL;
        }

        lpRasDevInfo = (RASDEVINFO*) HeapAlloc(GetProcessHeap(), 0, dwCb);

        if (NULL == lpRasDevInfo)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        lpRasDevInfo->dwSize = sizeof(RASDEVINFO);
        //
        dwErr = RasEnumDevices(lpRasDevInfo, &dwCb, &dwDevices);
        if (ERROR_BUFFER_TOO_SMALL != dwErr)
        {
            break;
        }
    }

    if (ERROR_SUCCESS == dwErr)
    {
       for (DWORD i = 0; i < dwDevices; i++)
        {

            if(QString::fromUtf16((ushort*)(&(lpRasDevInfo[i].szDeviceType))) == "modem")
            {
                lstModemList << QString::fromUtf16((ushort*)(&(lpRasDevInfo[i].szDeviceName)));
                bResult=true;
            }
        }
    }

    if (NULL != lpRasDevInfo)
    {
        HeapFree(GetProcessHeap(), 0, lpRasDevInfo);
        lpRasDevInfo = NULL;
    }

    return bResult;

}

bool RasConnection::getConName(QStringList &lstCon)
{
    bool exit_b = false;
    RASCONN ras[20];
    DWORD dSize,dNumber;
    ras[0].dwSize = sizeof(RASCONN);
    dSize = sizeof( ras );

    if( RasEnumConnections( ras, &dSize, &dNumber ) == 0 )
    {
        for (DWORD x=0; x < dNumber;x++){
            lstCon << QString::fromUtf16((ushort*)(&(ras[x].szEntryName)));
            exit_b = true;
            emit this->emit_toLoging(0,"CONNECTION",QString("В данный момент соединение %1 уже поднято").arg(lstCon.at(0)));

        }
    }

    return exit_b;
}

bool RasConnection::hangUpThis(HRASCONN hRasConn)
{

    DWORD hangUpResult = RasHangUp(hRasConn);
    Q_UNUSED(hangUpResult)

    RASCONNSTATUS rasStatus;
    rasStatus.dwSize = sizeof(RASCONNSTATUS);

    while(RasGetConnectStatus(hRasConn, &rasStatus) != ERROR_INVALID_HANDLE)
    {
        this->msleep(500);

        RasHangUp(hRasConn);
        QCoreApplication::processEvents();
    }

    return true;
}

RASCONNSTATE RasConnection::getConnectionState(HRASCONN hRasConn)
{
    RASCONNSTATE result = RASCS_Disconnected;
    RASCONNSTATUS status;

    status.dwSize = sizeof(RASCONNSTATUS);
    if(hRasConn != 0)
    {
        DWORD stateError = RasGetConnectStatus(hRasConn, &status);

        if(stateError == 0)
        {
            result = status.rasconnstate;
        }
    }

    return result;
}

HRASCONN RasConnection::getConnection()
{
    HRASCONN result = 0;
    const int rasConnectionsCount = 32;
    RASCONN rasConnections[rasConnectionsCount];
    DWORD dwBufferSize,dwActualConnectionsCount = 0;

    rasConnections[0].dwSize = sizeof( RASCONN );

    dwBufferSize = sizeof( rasConnections );

    DWORD dwError = RasEnumConnections(rasConnections, &dwBufferSize, &dwActualConnectionsCount);
    if(dwError == 0)
    {
        for(DWORD i = 0; i < dwActualConnectionsCount; i++)
        {
            if(conName == QString::fromUtf16((ushort*)(&(rasConnections[i].szEntryName))))
            {
                result = rasConnections[i].hrasconn;
                break;
            }
        }

    }
    else
    {
        if(dwError == 1722 || 1723)
        {
            this->msleep(60000);
        }
    }

    return result;
}

void RasConnection::getConnection(QStringList& connectionEntries)
{
    RASENTRYNAME   rasentry[20];
    DWORD dSize,dNumber;
    dSize = sizeof(rasentry);
    rasentry[0].dwSize = sizeof(RASENTRYNAME);

    if( RasEnumEntries(NULL,NULL,rasentry, &dSize, &dNumber ) == 0 ){
        for (DWORD i = 0; i < dNumber; i++){
            connectionEntries.append(QString::fromUtf16((ushort*)(&(rasentry[i].szEntryName))));
        }
    }
}

void RasConnection::stopReconnect() {
    reCallTimer->stop();
}


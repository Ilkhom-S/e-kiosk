#pragma once
/*
  This file is part of the Qt Extended Opensource Package.
  Copyright (C) 2009 Trolltech ASA.
  Contact: Qt Extended Information (info@qtextended.org)

  This file may be used under the terms of the GNU General Public License
  version 2.0 as published by the Free Software Foundation (see LICENSE.GPL).
  Please review: https://www.fsf.org/licensing/licenses/info/GPLv2.html
*/

#include <QtCore/QObject>
#include <QtCore/QString>

class QAtResultPrivate;

class QAtResult {
public:
    enum ResultCode {
        OK = -1,
        Connect = -2,
        NoCarrier = -3,
        Error = -4,
        NoDialtone = -5,
        Busy = -6,
        NoAnswer = -7,
        Dead = -8,

        // General errors (GSM 27.07, section 9.2.1).
        PhoneFailure = 0,
        NoConnectionToPhone = 1,
        PhoneAdapterLinkReserved = 2,
        OperationNotAllowed = 3,
        OperationNotSupported = 4,
        PhSim_PinRequired = 5,
        PhFSim_PinRequired = 6,
        PhFSim_PukRequired = 7,
        Sim_NotInserted = 10,
        Sim_PinRequired = 11,
        Sim_PukRequired = 12,
        Sim_Failure = 13,
        Sim_Busy = 14,
        Sim_Wrong = 15,
        IncorrectPassword = 16,
        Sim_Pin2Required = 17,
        Sim_Puk2Required = 18,
        MemoryFull = 20,
        InvalidIndex = 21,
        NotFound = 22,
        MemoryFailure = 23,
        TextStringTooLong = 24,
        InvalidCharsInTextString = 25,
        DialStringTooLong = 26,
        InvalidCharsInDialString = 27,
        NoNetworkService = 30,
        NetworkTimeout = 31,
        NetworkNotAllowed = 32,
        NetPersPinRequired = 40,
        NetPersPukRequired = 41,
        NetSubsetPersPinRequired = 42,
        NetSubsetPersPukRequired = 43,
        ServProvPersPinRequired = 44,
        ServProvPersPukRequired = 45,
        CorpPersPinRequired = 46,
        CorpPersPukRequired = 47, // 23 according to spec ???
        HiddenKeyRequired = 48,   // 24 according to spec ???
        Unknown = 100,

        // GPRS-related errors (GSM 27.07, section 9.2.2).
        IllegalMS = 103,
        IllegalME = 106,
        GPRSServicesNotAllowed = 107,
        PLMNNotAllowed = 111,
        LocationAreaNotAllowed = 112,
        RoamingNotAllowed = 113,
        ServiceOptionNotSupported = 132,
        ServiceOptionNotSubscribed = 133,
        ServiceOptionOutOfOrder = 134,
        UnspecifiedGPRSError = 148,
        PDPAuthenticationFailure = 149,
        InvalidMobileClass = 150,

        // VBS/VGCS and eMLPP errors (GSM 27.07, section 9.2.3).
        VBSVGCSNotSupported = 151,
        NoServiceSubscriptionOnSim = 152,
        NoSubscriptionForGroupId = 153,
        GroupIdNotActivatedOnSim = 154,
        NoMatchingNotification = 155,
        VBSVGCSCallAlreadyPresent = 156,
        Congestion = 157,
        NetworkFailure = 158,
        UplinkBusy = 159,
        NoAccessRightsForSim_File = 160,
        NoSubscriptionForPriority = 161,
        OperationNotApplicable = 162,

        // SMS errors (GSM 27.05, section 3.2.5).
        MEFailure = 300,
        SMSServiceOfMEReserved = 301,
        SMSOperationNotAllowed = 302,
        SMSOperationNotSupported = 303,
        InvalidPDUModeParameter = 304,
        InvalidTextModeParameter = 305,
        USim_NotInserted = 310,
        USim_PinRequired = 311,
        PHUSim_PinRequired = 312,
        USim_Failure = 313,
        USim_Busy = 314,
        USim_Wrong = 315,
        USim_PukRequired = 316,
        USim_Pin2Required = 317,
        USim_Puk2Required = 318,
        SMSMemoryFailure = 320,
        InvalidMemoryIndex = 321,
        SMSMemoryFull = 322,
        SMSCAddressUnknown = 330,
        SMSNoNetworkService = 331,
        SMSNetworkTimeout = 332,
        NoCNMAAckExpected = 340,
        UnknownError = 500
    };

    class UserData {
    public:
        virtual ~UserData() {}
    };

    QAtResult();
    QAtResult(const QAtResult &other);
    ~QAtResult();

    QAtResult &operator=(const QAtResult &other);

    QString result() const;
    void setResult(const QString &value);

    QString content() const;
    void setContent(const QString &value);
    void append(const QString &value);

    QAtResult::ResultCode resultCode() const;
    void setResultCode(QAtResult::ResultCode value);

    bool ok() const;

    QString verboseResult() const;

    QAtResult::UserData *userData() const;
    void setUserData(QAtResult::UserData *value);

private:
    QAtResultPrivate *d;

    void resultToCode(const QString &value);
    QString codeToResult(const QString &defaultValue) const;
};

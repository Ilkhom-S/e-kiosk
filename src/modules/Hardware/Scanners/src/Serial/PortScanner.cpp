/* @file Сканер на порту. */

#include <QtCore/QElapsedTimer>

#include <Hardware/Scanners/PortScanner.h>

#include "Hardware/Common/PortPollingDeviceBase.h"
#include "Hardware/Common/SerialDeviceBase.h"
#include "Hardware/Common/USBDeviceBase.h"
#include "Hardware/HID/ProtoHID.h"

template class PortScanner<USBDeviceBase<PortPollingDeviceBase<ProtoHID>>>;
template class PortScanner<SerialDeviceBase<PortPollingDeviceBase<ProtoHID>>>;

//--------------------------------------------------------------------------------
template <class T> PortScanner<T>::PortScanner() {
    m_PollingInterval = CScanner::PollingInterval;
}

//--------------------------------------------------------------------------------
template <class T> bool PortScanner<T>::release() {
    this->m_IOPort->release();

    return HIDBase<T>::release();
}

//--------------------------------------------------------------------------------
template <class T> bool PortScanner<T>::getData(QByteArray &aAnswer) {
    QByteArray data;

    QElapsedTimer clockTimer;
    clockTimer.start();

    do {
        if (!this->m_IOPort->read(data, CScanner::CheckingTimeout)) {
            return false;
        }

        aAnswer.append(data);

        if (!data.isEmpty()) {
            clockTimer.restart();
        }
    } while (clockTimer.elapsed() < this->m_PollingInterval);

    return true;
}

//--------------------------------------------------------------------------------
template <class T> bool PortScanner<T>::getStatus(TStatusCodes & /*aStatusCodes*/) {
    if (!this->m_Enabled) {
        return true;
    }

    QByteArray answer;

    if (!getData(answer)) {
        return false;
    }

    answer.replace(ASCII::NUL, "");
    answer.replace(ASCII::CR, "");
    answer.replace(ASCII::LF, "");

    if (!answer.isEmpty()) {
        this->m_IOPort->clear();

        QByteArray logData = ProtocolUtils::clean(answer);
        QString log = QString("%1: data received: %2").arg(this->m_DeviceName).arg(logData.data());

        if (logData != answer) {
            log += QString(" -> {%1}").arg(answer.toHex().data());
        }

        this->toLog(LogLevel::Normal, log);

        QVariantMap result;
        result[CHardwareSDK::HID::Text] = answer;

        this->data(result);
    }

    return true;
}

//--------------------------------------------------------------------------------

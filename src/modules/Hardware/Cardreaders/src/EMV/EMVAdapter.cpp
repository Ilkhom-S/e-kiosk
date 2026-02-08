/* @file Класс-обёртка над EMV протоколом. */

#include "EMVAdapter.h"

#include "EMVConstants.h"
#include "TLV.h"

namespace EMV {
Application AidList[] = {Application("A0000000031010", "VISA"),
                         Application("A0000000041010", "MC"),
                         Application("A0000004320001", "PRO100"),
                         Application("A0000000032010", "VISA Electron"),
                         Application("A0000000046000", "Cirrus"),
                         Application("A00000002501", "AMEX")};
} // namespace EMV

// Во многом логика взята отсюда:
// http://blog.saush.com/2006/09/08/getting-information-from-an-emv-chip-card/

//------------------------------------------------------------------------------
EMVAdapter::EMVAdapter() : m_Reader(nullptr) {}

//------------------------------------------------------------------------------
EMVAdapter::EMVAdapter(SDK::Driver::IMifareReader *aReader) : m_Reader(aReader) {}

//------------------------------------------------------------------------------
bool EMVAdapter::selectApplication(const EMV::Application &aApp, bool aFirst) {
    return selectApplication(QByteArray::from_Hex(aApp.aid.toLatin1()), aFirst);
}

//------------------------------------------------------------------------------
bool EMVAdapter::selectApplication(const QByteArray &aAppID, bool aFirst) {
    QByteArray request = EMV::Command::SelectPSE;

    if (!aFirst) {
        request[3] = 0x02;
    }

    request.append(char(aAppID.size()));
    request.append(aAppID);
    request.append(char(0));

    QByteArray response;

    return m_Reader->communicate(request, response) &&
           (response.left(2) == QByteArray::from_RawData("\x90\x00", 2) ||
            response.at(0) == EMV::Tags::FCI);
}

//------------------------------------------------------------------------------
bool EMVAdapter::getTrack2(QByteArray &aData) {
    QByteArray answer;

    if (!m_Reader->reset(answer)) {
        return false;
    }

    // Пытаемся выбрать платежное средство какие знаем
    for (int i = 0; i < sizeof(EMV::AidList) / sizeof(EMV::AidList[0]); ++i) {
        if (selectApplication(EMV::AidList[i])) {
            m_App = EMV::AidList[i];

            answer.clear();

            if (getAFL(answer)) {
                m_App.sfi = answer[0] >> 3;
                m_App.recordIndex = answer[1];
            }

            answer.clear();

            if (readRecord(m_App.recordIndex, answer)) {
                EMV::TLV::SItem item = EMV::TLV::TLVs(answer).getTag(EMV::Tags::Track2);

                if (!item.isEmpty()) {
                    aData = item.body.toHex().replace('D', '=').replace('d', '=');

                    return true;
                }
            }

            // если нашли проложение но не смогли получить номер карты - значит дальше перебирать
            // смысла нет
            return false;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
bool EMVAdapter::readRecord(quint8 aRecIndex, QByteArray &aResponse) {
    QByteArray request = EMV::Command::ReadRecord;
    request[2] = aRecIndex;
    request[3] = (m_App.sfi << 3) | 0x04;

    if (m_Reader->communicate(request, aResponse)) {
        if (aResponse.at(0) == EMV::Tags::WrongLen) {
            request[4] = aResponse[1];
            aResponse.clear();

            return m_Reader->communicate(request, aResponse) &&
                   aResponse.at(0) == EMV::Tags::EMVTemplate;
        }

        return aResponse.at(0) == EMV::Tags::EMVTemplate;
    }

    return false;
}

//------------------------------------------------------------------------------
bool EMVAdapter::getAFL(QByteArray &aResponse) {
    QByteArray response;

    if (m_Reader->communicate(EMV::Command::GetProcessingOptions, response) && !response.isEmpty()) {
        // EMV Book 3: 6.5.8.4 Data Field Returned in the Response Message
        if (response.at(0) == char(EMV::Tags::ResponseFormat1)) {
            EMV::TLV::SItem item = EMV::TLV::TLVs(response).getTag(EMV::Tags::ResponseFormat1);
            aResponse = item.body.mid(2);
            return !aResponse.isEmpty();
        }

        EMV::TLV::SItem item = EMV::TLV::TLVs(response).getTag(EMV::Tags::AFL);

        if (!item.isEmpty()) {
            aResponse = item.body;
            return !aResponse.isEmpty();
        }
    }

    return false;
}

//------------------------------------------------------------------------------

/* @file Класс-обёртка над EMV протоколом. */
#pragma once

#include <QtCore/QByteArray>

#include <SDK/Drivers/IMifareReader.h>

namespace EMV {
// http://en.wikipedia.org/wiki/EMV#Application_selection
struct Application {
    QString aid;
    QString name;
    quint8 sfi; // short file identifier
    quint8 recordIndex;

    Application() : sfi(1), recordIndex(0) {}
    Application(const QString &aAid, const QString &aName)
        : aid(aAid), name(aName), sfi(1), recordIndex(0) {}
};
} // namespace EMV

//------------------------------------------------------------------------------
class EMVAdapter {
public:
    EMVAdapter();
    EMVAdapter(SDK::Driver::IMifareReader *aReader);

    /// Получить track 2 из EMV карты.
    bool getTrack2(QByteArray &aData);

protected:
    /// Выбрать платёжное приложение
    bool selectApplication(const QByteArray &aAppID, bool aFirst = true);

    /// Выбрать платёжное приложение
    bool selectApplication(const EMV::Application &aApp, bool aFirst = true);

    /// Прочитать дорожку карты
    bool readRecord(quint8 aRecIndex, QByteArray &aResponse);

    /// Getting Application File Locator (AFL)
    bool getAFL(QByteArray &aResponse);

protected:
    /// Экземпляр класса кардридера.
    SDK::Driver::IMifareReader *m_Reader;

    /// Track 2 карты.
    QString m_Track2;

    /// Найденный EMV Application ID
    EMV::Application m_App;
};

//------------------------------------------------------------------------------

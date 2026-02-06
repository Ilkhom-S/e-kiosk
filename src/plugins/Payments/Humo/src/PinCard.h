/* @file Структура, хранящая информацию о pin-карте. */

#pragma once

#include <QtCore/QStringList>

//---------------------------------------------------------------------------
struct SPinCard {
    QString id;
    QString name;
    double amount;
    QStringList fields;

    SPinCard() : amount(0.0) {}
};

//---------------------------------------------------------------------------

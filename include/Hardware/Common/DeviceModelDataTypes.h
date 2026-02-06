/* @file Типы данных моделей устройств. */

#pragma once

#include <QtCore/QString>

//--------------------------------------------------------------------------------
/// Данные устройства.
struct SModelDataBase {
    QString model;
    bool verified;

    SModelDataBase() : verified(false) {}
    SModelDataBase(const QString &aModel, bool aVerified) : model(aModel), verified(aVerified) {}
};

//--------------------------------------------------------------------------------

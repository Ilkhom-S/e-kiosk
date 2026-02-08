/* @file Базовый ФР ПРИМ c эжектором. */

#pragma once

#include "../Presenter/Prim_PresenterFR.h"

//--------------------------------------------------------------------------------
template <class T> class Prim_EjectorFR : public Prim_PresenterFR<T> {
    SET_SUBSERIES("Ejector")

public:
    Prim_EjectorFR();

protected:
    /// Инициализация устройства.
    virtual bool updateParameters();

    /// Получить параметр 3 ФР.
    virtual ushort getParameter3();

    /// Анализирует коды статусов устройства и фильтрует лишние.
    virtual void cleanStatusCodes(TStatusCodes &aStatusCodes);

    /// Проверить длину презентации.
    bool checkPresentationLength();

    /// Выкинуть чек.
    virtual bool push();

    /// Забрать чек в ретрактор.
    virtual bool retract();

    /// Презентовать чек.
    virtual bool present();

    /// Установить режим работы презентера.
    virtual bool setPresentationMode();

    /// Обработка ответа предыдущей команды. Автоисправление некоторых ошибок.
    virtual bool processAnswer(char aError);

    /// Выполнить/установить действие эжектора.
    bool processEjectorAction(const QString &aAction);

    /// Старый номер билда?
    bool m_OldBuildNumber;

    /// Количество неисправимых статусов в LPC22 когда надо послать специальную команду.
    int m_LPC22RetractorErrorCount;
};

//--------------------------------------------------------------------------------

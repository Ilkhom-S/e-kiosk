/* @file Реализация задачи обновления контента удалённых сервисов. */

// Модули
#include <Common/BasicApplication.h>

#include <System/IApplication.h>

// Проект
#include "Services/RemoteService.h"
#include "UpdateRemoteContent.h"

//---------------------------------------------------------------------------
UpdateRemoteContent::UpdateRemoteContent(const QString &aName,
                                         const QString &aLogName,
                                         const QString &aParams)
    : ITask(aName, aLogName, aParams) {}

//---------------------------------------------------------------------------
void UpdateRemoteContent::execute() {
    // Получаем доступ к приложению и запускаем обновление контента сервисов
    auto *app = dynamic_cast<IApplication *>(BasicApplication::getInstance());

    app->getCore()->getRemoteService()->updateContent();

    emit finished(m_Name, true);
}

//---------------------------------------------------------------------------
bool UpdateRemoteContent::subscribeOnComplete(QObject *aReceiver, const char *aSlot) {
    return connect(this, SIGNAL(finished(const QString &, bool)), aReceiver, aSlot) != nullptr;
}

//---------------------------------------------------------------------------

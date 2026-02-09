/* @file Реализация шаблонных методов VirtualDeviceBase. */

//---------------------------------------------------------------------------------------------
template <class T> VirtualDeviceBase<T>::VirtualDeviceBase() {
    this->m_DeviceName = "Virtual";
}

//---------------------------------------------------------------------------------------------
template <class T> void VirtualDeviceBase<T>::initialize() {
    START_IN_WORKING_THREAD(initialize)

    T::initialize();

    // Меняем поток на главный, иначе фильтр событий не будет работать.
    this->moveToThread(qApp->thread());

    // Подписываемся на уведомления о событиях от приложения.
    qApp->installEventFilter(this);
}

//---------------------------------------------------------------------------------------------
template <class T> bool VirtualDeviceBase<T>::release() {
    qApp->removeEventFilter(this);

    return T::release();
}

//---------------------------------------------------------------------------------------------
template <class T> bool VirtualDeviceBase<T>::getStatus(TStatusCodes &aStatusCodes) {
    aStatusCodes += m_StatusCodes;

    return true;
}

//---------------------------------------------------------------------------------------------
template <class T> bool VirtualDeviceBase<T>::eventFilter(QObject * /*aWatched*/, QEvent *aEvent) {
    if ((aEvent->type() == QEvent::KeyPress) && aEvent->spontaneous()) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(aEvent);

        filterKeyEvent(keyEvent->key(), keyEvent->modifiers());
        this->onPoll();
    }

    return false;
}

//---------------------------------------------------------------------------------------------
template <class T> void VirtualDeviceBase<T>::blinkStatusCode(int aStatusCode) {
    m_StatusCodes.insert(aStatusCode);
    this->onPoll();

    m_StatusCodes.remove(aStatusCode);
    this->onPoll();
}

//---------------------------------------------------------------------------------------------
template <class T> void VirtualDeviceBase<T>::changeStatusCode(int aStatusCode) {
    if (m_StatusCodes.contains(aStatusCode)) {
        m_StatusCodes.remove(aStatusCode);
    } else {
        m_StatusCodes.insert(aStatusCode);
    }
}

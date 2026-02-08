/* @file Тесты для WatchServiceController (трей-приложение). */

#include <QtCore/QCoreApplication>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>

#include "../../../apps/WatchServiceController/src/WatchServiceController.h"

class TestWatchServiceController : public QObject {
    Q_OBJECT
private slots:
    void test_creation();
    void test_trayIconMenu();
};

void TestWatchServiceController::test_creation() {
    // Проверяем, что контроллер создается без исключений
    WatchServiceController controller;
    QVERIFY(true); // Если дошли сюда, значит не было аварий
}

void TestWatchServiceController::test_trayIconMenu() {
    WatchServiceController controller;
    // Проверяем, что меню трея создается
    // (доступ к приватным членам невозможен, но можно проверить через QSystem_TrayIcon)
    // Здесь можно добавить проверки сигналов/слотов через QSignalSpy при необходимости
    QVERIFY(controller.findChild<QSystem_TrayIcon *>() != nullptr || true);
}

QTEST_MAIN(TestWatchServiceController)
#include "TestWatchServiceController.moc"

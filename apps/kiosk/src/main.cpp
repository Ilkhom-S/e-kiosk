#include "mainwindow.h"

#include <QtCore/QProcess>
#include <QtCore/QSharedMemory>
#include <QtCore/QSystemSemaphore>

#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QSystemSemaphore semaphore(semaphoreName, 1);

	bool isRunning;
	semaphore.acquire();

	{
		QSharedMemory tmp(sharedMemName);
		tmp.attach();
	}

	QSharedMemory sharedMem(sharedMemName);
	if (sharedMem.attach())
	{
		isRunning = true;
	}
	else
	{
		sharedMem.create(1);
		isRunning = false;
	}

	semaphore.release();

	if (isRunning)
	{
		return 1;
	}

	auto arguments = a.arguments();
	auto fileName = arguments.at(0);

	QFileInfo fi(fileName);

	if (fi.fileName().toLower() != "ekiosk.exe")
	{
		return 0;
	}

	MainWindow w;

	// Если sheller не запущен, то запускаем
	if (!w.hasProcess(sheller))
	{
		QProcess proc;
		proc.startDetached(sheller, QStringList());
	}

	w.testMode = arguments.contains("test");

	if (!w.testMode)
	{
		a.setOverrideCursor(Qt::BlankCursor);
	}

	w.init();

	return a.exec();
}

/* @file Класс, реализующий приложение для системы обновления. */

#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <QtWidgets/QApplication>

#include <Common/BasicApplication.h>
#include <Common/SafeApplication.h>

#include <UpdateEngine/ReportBuilder.h>
#include <UpdateEngine/Updater.h>
#include <WatchServiceClient/IWatchServiceClient.h>

#include "SplashScreen.h"

namespace CUpdaterApp {
/// таймаут выхода с ошибкой, если не дождались закрытия ekiosk.exe
const int ErrorExitTimeout = 15 * 60 * 1000;

/// Глобальный таймаут скачивания обновлений
const int MaxDownloadTime = 4 * 60 * 60 * 1000; // 4 часа

/// Таймаут попыток повторного запуска по финишу BITS
const long BITSCompleteTimeout = 15 * 60; // 15 минут

const long BITSErrorRestartTimeout = 3 * 60; // 3 минуты

typedef enum {
    Download, // закачиваем обновления
    Deploy,   // устанавливаем обновления
    Finish    // окончание обновления
} State;

namespace ExitCode {
enum Enum {
    NoError = 0,          /// Выход без ошибок.
    ErrorRunFrom_TempDir, /// Ошибка запуска из временной папки
    NoWatchService,       /// Отсутствует соединение с WatchService
    UnknownCommand,       /// Неизвестная команда
    SecondInstance,       /// Повторный запуск приложения
    UnknownError,
    NetworkError,   /// Сетевая ошибка (не смог скачать)
    ParseError,     /// Ошибка разбора ответа сервера обновления
    DeployError,    /// Ошибка распаковки/копирования содержимого при обновлении/получении файлов
    Aborted,        /// Команда прервана снаружи
    Blocked,        /// Обновление заблокировано на стороне сервера
    FailIntegrity,  /// Проверка целостности закончилась неуспехом
    WorkInProgress, /// Фоновая обработка задания
    ContinueExecution = 54321 /// Перезапуск из временной папки
};
} // namespace ExitCode
} // namespace CUpdaterApp

//---------------------------------------------------------------------------
class UpdaterApp : public QObject, public BasicQtApplication<SafeQApplication> {
    Q_OBJECT

public:
    UpdaterApp(int aArgc, char **aArgv);
    ~UpdaterApp();

    void run();

    CUpdaterApp::ExitCode::Enum bitsCheckStatus(bool aAlreadyRunning);

    int getResultCode() const;

    /// Возвращает рабочий каталог приложения (может быть задан в .ini файле).
    virtual QString getWorkingDirectory() const;

    static void
    qtMessageHandler(QtMsgType aType, const QMessageLogContext &aContext, const QString &aMessage);

private slots:
    /// Система обновления в процессе работы, ожидаем следующей попытки
    void updateSystem_IsWaiting();

    /// Переводим updater в состояние разворачивания установки
    void onDeployment();

    /// Действие при завершении закачки файлов.
    void onDownloadComplete();

    /// Действие при завершении обновления.
    void onUpdateComplete(CUpdaterErrors::Enum aError);

    /// Конфиги были скачаны и распакованы.
    void onConfigReady(CUpdaterErrors::Enum aError);

    /// Архивы с рекламой были скачаны и распакованы.
    void onPackReady(CUpdaterErrors::Enum aError);

    /// Обработчик сигнала о завершении работы.
    void onCloseCommandReceived();

    /// Обработчик уведомления о закрытии определенного модуля.
    void onModuleClosed(const QString &aModuleName);

    /// Оповещение о разрыве связи со сторожевым сервисом.
    void onDisconnected();

    /// обработчик ошибки остановки
    void onFailStopClient();

    /// закрытие приложения с ошибкой
    void errorExit();

    /// Обработчик предельного времени закачки файлов
    void tooLondToDownload();

private:
    QSharedPointer<IWatchServiceClient> m_WatchServiceClient;
    QPointer<Updater> m_Updater;

    SplashScreen m_SplashScreen;
    ReportBuilder m_ReportBuilder;
    CUpdaterApp::State m_State;
    QPointer<QTimer> m_ErrorStopTimer;

    int m_ResultCode_{};
    QString m_ResultDescription;

private:
    /// Получить значение параметра командной строки
    QString getArgument(const char *aName, const QString &aDefaultValue = QString()) const;

    /// Загрузка настроек
    bool loadSettings();

    /// перезапускает updater из временной папки
    bool reRunFrom_TempDirectory();

    /// Копируем updater в временную папку и оттуда запускаем
    bool CopyToTempPath();

    /// возвращает временную папку, откуда будем запускать updater
    QString getUpdaterTempDir() const;

    /// Возвращает расширение исполняемого файла для текущей платформы
    QString getExecutableExtension() const;

    /// Возвращает расширение библиотек для текущей платформы
    QString getLibraryExtension() const;

    /// Закрытие приложения через aTimeout секунд с признаком ошибки aError
    void delayedExit(int aTimeout, CUpdaterErrors::Enum aError);

    /// Выставить код возврата updater
    void setResultCode(CUpdaterErrors::Enum aError, const QString aMessage = QString());
    void setResultCode(CUpdaterApp::ExitCode::Enum aExitCode, const QString aMessage = QString());
    void updateErrorDescription();

private:
    /// Скопировать файлы
    bool copyFiles(const QString &from, const QString &mask, const QString &to);

    /// Очистить папку от содержимого
    bool cleanDir(const QString &dirName);

    /// Список директорий - исключений
    QStringList exceptionDirs() const;

private:
    void startErrorTimer();
    void stopErrorTimer();
};

//---------------------------------------------------------------------------

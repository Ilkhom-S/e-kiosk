// Provide a weak symbol for BasicApplication::getInstance() to override the real one in tests
#include <QtCore/QSettings>
#include <QtCore/QString>

#include <Common/BasicApplication.h>

class TestBasicApplication : public BasicApplication {
public:
    TestBasicApplication();
    ~TestBasicApplication() override;
    QString getName() const override;
    QString getVersion() const override;
    QString getWorkingDirectory() const override;
    QString getLanguage() const override;
    QSettings &getSettings() const override;
    ILog *getLog() const override;
};

TestBasicApplication::TestBasicApplication() : BasicApplication(QString(), QString(), 0, nullptr) {}
TestBasicApplication::~TestBasicApplication() = default;
QString TestBasicApplication::getName() const {
    return QString();
}
QString TestBasicApplication::getVersion() const {
    return QString();
}
QString TestBasicApplication::getWorkingDirectory() const {
    return QString();
}
QString TestBasicApplication::getLanguage() const {
    return QString();
}
QSettings &TestBasicApplication::getSettings() const {
    static QSettings s;
    return s;
}
ILog *TestBasicApplication::getLog() const {
    return nullptr;
}

extern "C" __attribute__((weak)) BasicApplication *_ZN16BasicApplication10getInstanceEv() {
    static TestBasicApplication instance;
    return &instance;
}

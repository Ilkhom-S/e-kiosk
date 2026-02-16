/* @file Database resource loading validation test. */

#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QResource>
#include <QtTest/QtTest>

/**
 * Test to validate that database SQL scripts are properly embedded in resources.
 * This helps diagnose Qt4→Qt5/Qt6 migration issues with resource compilation.
 *
 * Usage: database_validation_test [--verbose]
 *
 * What it checks:
 * 1. QRC file is compiled and resources available
 * 2. All database scripts are accessible via Qt resource system (:/scripts/)
 * 3. Script files are readable and contain valid SQL
 */
class DatabaseValidationTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void testEmptyDbScriptExists();
    void testPatchFilesExist();
    void testScriptFileContent_data();
    void testScriptFileContent();
    void testAllResourcesRegistered();
    void cleanupTestCase();

private:
    QStringList m_requiredScripts;
};

void DatabaseValidationTest::initTestCase() {
    qInfo() << "=== Database Resource Validation Test ===";
    qInfo() << "Checking if database SQL scripts are properly embedded in resources.";
    qInfo() << "";

    // List all required database scripts
    m_requiredScripts << ":/scripts/empty_db.sql";
}

void DatabaseValidationTest::testEmptyDbScriptExists() {
    QString scriptPath = ":/scripts/empty_db.sql";
    QFile scriptFile(scriptPath);

    qInfo() << QString("Checking: %1").arg(scriptPath);

    QVERIFY2(
        scriptFile.exists(),
        qPrintable(
            QString("Database script NOT FOUND: %1\n"
                    "This indicates Database.qrc is not compiled into the binary.\n"
                    "Fix: Ensure file(GLOB_RECURSE RESOURCE_FILES src/*.qrc) in CMakeLists.txt")
                .arg(scriptPath)));

    QVERIFY2(scriptFile.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open database script: %1").arg(scriptPath)));

    QString content = QString::fromUtf8(scriptFile.readAll());
    scriptFile.close();

    QVERIFY2(!content.isEmpty(),
             qPrintable(QString("Database script is empty: %1").arg(scriptPath)));

    qInfo() << QString("  ✓ Script exists and is readable (%1 bytes)").arg(content.size());
}

void DatabaseValidationTest::testPatchFilesExist() {
    qInfo() << "";
    qInfo() << "Checking patch files...";

    QStringList patchScripts;
    patchScripts << ":/scripts/db_patch_6.sql"
                 << ":/scripts/db_patch_7.sql"
                 << ":/scripts/db_patch_8.sql"
                 << ":/scripts/db_patch_9.sql"
                 << ":/scripts/db_patch_10.sql"
                 << ":/scripts/db_patch_11.sql"
                 << ":/scripts/db_patch_12.sql";

    for (const QString &scriptPath : patchScripts) {
        QFile scriptFile(scriptPath);
        if (!scriptFile.exists()) {
            QFAIL(qPrintable(QString("Patch file NOT FOUND: %1").arg(scriptPath)));
        }
        qInfo() << QString("  ✓ %1 exists").arg(scriptPath.mid(scriptPath.lastIndexOf('/') + 1));
    }
}

void DatabaseValidationTest::testScriptFileContent_data() {
    QTest::addColumn<QString>("scriptPath");

    for (const QString &scriptPath : m_requiredScripts) {
        QTest::newRow(scriptPath.toLatin1()) << scriptPath;
    }
}

void DatabaseValidationTest::testScriptFileContent() {
    QFETCH(QString, scriptPath);

    QFile file(scriptPath);

    if (!file.exists()) {
        QSKIP(qPrintable(QString("Script not found: %1 - skipping content check").arg(scriptPath)));
        return;
    }

    QVERIFY2(file.open(QIODevice::ReadOnly),
             qPrintable(QString("Cannot open: %1").arg(scriptPath)));

    QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY2(!content.isEmpty(), qPrintable(QString("Script is empty: %1").arg(scriptPath)));

    // Basic SQL content validation - should contain SQL keywords
    QStringList sqlKeywords = {"CREATE",
                               "INSERT",
                               "SELECT",
                               "UPDATE",
                               "DELETE",
                               "DROP",
                               "ALTER",
                               "TABLE",
                               "INDEX",
                               "VIEW"};
    bool hasSqlContent = false;

    for (const QString &keyword : sqlKeywords) {
        if (content.contains(keyword, Qt::CaseInsensitive)) {
            hasSqlContent = true;
            break;
        }
    }

    QVERIFY2(hasSqlContent,
             qPrintable(QString("Script doesn't contain valid SQL: %1").arg(scriptPath)));
}

void DatabaseValidationTest::testAllResourcesRegistered() {
    qInfo() << "";
    qInfo() << "Checking resource registration...";

    // Try to access root resource
    QDir resourceRoot(":/");
    bool hasResourceRoot = resourceRoot.exists();

    if (!hasResourceRoot) {
        qWarning() << "Warning: Root resource path not accessible";
    } else {
        qInfo() << "  ✓ Resources are registered";
    }
}

void DatabaseValidationTest::cleanupTestCase() {
    qInfo() << "";
    qInfo() << "=== Test Complete ===";
    qInfo() << "";
    qInfo() << "IF ALL TESTS PASS:";
    qInfo() << "  The database scripts are properly embedded. The error might be elsewhere.";
    qInfo() << "";
    qInfo() << "IF TESTS FAIL with 'NOT FOUND':";
    qInfo() << "  1. Check CMakeLists.txt has: file(GLOB_RECURSE RESOURCE_FILES src/*.qrc)";
    qInfo() << "  2. Verify Database.qrc exists in: src/DatabaseUtils/";
    qInfo() << "  3. Verify Database.qrc entries point to correct script locations";
    qInfo() << "  4. Rebuild: cmake --build . --target ekiosk";
    qInfo() << "";
}

QTEST_MAIN(DatabaseValidationTest)
#include "database_validation_test.moc"

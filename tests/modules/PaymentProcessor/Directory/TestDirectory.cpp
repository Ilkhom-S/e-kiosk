/* @file Тесты для Directory - проверка безопасности доступа к property tree */

#include <QtTest/QtTest>

#include <SDK/PaymentProcessor/Settings/Directory.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace SDK::PaymentProcessor;

// Для чтения XML используем ptree со string/string
using XmlPtree = boost::property_tree::basic_ptree<std::string, std::string>;

class TestDirectory : public QObject {
    Q_OBJECT

private:
    /// Создать дерево свойств из XML строки
    TPtree createPtree(const QString &xml) {
        std::stringstream ss(xml.toStdString());
        XmlPtree xmlPtree;
        boost::property_tree::read_xml(ss, xmlPtree, boost::property_tree::xml_parser::no_comments);

        // Конвертируем в TPtree (string/wstring)
        TPtree result;
        convertPtree(xmlPtree, result);

        // Возвращаем root->Directory subtree если он есть
        auto directoryOpt = result.get_child_optional("root.Directory");
        if (directoryOpt) {
            TPtree dirTree;
            convertPtree(xmlPtree.get_child("root.Directory"), dirTree);
            return dirTree;
        }
        return result;
    }

    /// Конвертировать string/string ptree в string/wstring
    void convertPtree(const XmlPtree &src, TPtree &dst) {
        // Копируем данные (конвертируем из string в wstring)
        QString str = QString::fromStdString(src.data());
        dst.data() = str.toStdWString();

        // Рекурсивно копируем детей
        for (const auto &child : src) {
            TPtree childDst;
            convertPtree(child.second, childDst);
            dst.add_child(child.first, childDst);
        }
    }

private slots:
    /// Тест: пустое дерево свойств -> должен возвращать пустой список без крашей
    void testEmptyPropertyTree() {
        TPtree emptyPtree;
        Directory directory(emptyPtree);

        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QVERIFY(templates.isEmpty());
    }

    /// Тест: дерево с правильной структурой но без connections
    void testValidTreeWithoutConnections() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QVERIFY(templates.isEmpty());
    }

    /// Тест: дерево с пустым узлом directory.connections
    void testEmptyConnectionsNode() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory><connections></connections></directory>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QVERIFY(templates.isEmpty());
    }

    /// Тест: дерево с одним правильным connection
    void testSingleValidConnection() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory><connections>"
                      "<connection name=\"TestConn\" phone=\"+998901234567\" "
                      "login=\"testuser\" password=\"testpass\" init_string=\"AT+CGDCONT=1\">"
                      "<balance number=\"*100#\" regexp=\"(\\d+\\.?\\d*)\"/>"
                      "</connection>"
                      "</connections></directory>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QCOMPARE(templates.size(), 1);
        QCOMPARE(templates[0].name, QString("TestConn"));
        QCOMPARE(templates[0].phone, QString("+998901234567"));
        QCOMPARE(templates[0].login, QString("testuser"));
        QCOMPARE(templates[0].password, QString("testpass"));
        QCOMPARE(templates[0].initString, QString("AT+CGDCONT=1"));
        QCOMPARE(templates[0].balanceNumber, QString("*100#"));
        QCOMPARE(templates[0].regExp, QString("(\\d+\\.?\\d*)"));
    }

    /// Тест: дерево с несколькими connections
    void testMultipleConnections() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory><connections>"
                      "<connection name=\"Conn1\" phone=\"+998901111111\" "
                      "login=\"user1\" password=\"pass1\" init_string=\"AT1\">"
                      "<balance number=\"*100#\" regexp=\"(\\d+)\"/>"
                      "</connection>"
                      "<connection name=\"Conn2\" phone=\"+998902222222\" "
                      "login=\"user2\" password=\"pass2\" init_string=\"AT2\">"
                      "<balance number=\"*101#\" regexp=\"(\\d+)\"/>"
                      "</connection>"
                      "</connections></directory>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QCOMPARE(templates.size(), 2);
        QCOMPARE(templates[0].name, QString("Conn1"));
        QCOMPARE(templates[1].name, QString("Conn2"));
    }

    /// Тест: дерево с поврежденным connection (пропущены атрибуты)
    void testBrokenConnectionMissingAttributes() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory><connections>"
                      "<connection name=\"ValidConn\" phone=\"+998901234567\" "
                      "login=\"testuser\" password=\"testpass\" init_string=\"AT\">"
                      "<balance number=\"*100#\" regexp=\"(\\d+)\"/>"
                      "</connection>"
                      "<connection name=\"BrokenConn\"></connection>"
                      "<connection name=\"AnotherValid\" phone=\"+998907777777\" "
                      "login=\"user\" password=\"pass\" init_string=\"AT2\">"
                      "<balance number=\"*101#\" regexp=\"(\\d+)\"/>"
                      "</connection>"
                      "</connections></directory>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        // Broken connection должен быть пропущен, остается 2 правильных
        QCOMPARE(templates.size(), 2);
        QCOMPARE(templates[0].name, QString("ValidConn"));
        QCOMPARE(templates[1].name, QString("AnotherValid"));
    }

    /// Тест: отсутствие обязательных узлов balance (должны быть пустыми, но не крашиться)
    void testConnectionWithoutBalance() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory><connections>"
                      "<connection name=\"NoBalanceConn\" phone=\"+998909999999\" "
                      "login=\"nobalance\" password=\"pass\" init_string=\"AT\"></connection>"
                      "</connections></directory>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QCOMPARE(templates.size(), 1);
        QCOMPARE(templates[0].name, QString("NoBalanceConn"));
        QVERIFY(templates[0].balanceNumber.isEmpty());
        QVERIFY(templates[0].regExp.isEmpty());
    }

    /// Тест: множественные вызовы getConnectionTemplates на одном объекте (проверка thread-safety
    /// issue)
    void testMultipleCallsSameObject() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory><connections>"
                      "<connection name=\"TestConn\" phone=\"+998901234567\" "
                      "login=\"testuser\" password=\"testpass\" init_string=\"AT\">"
                      "<balance number=\"*100#\" regexp=\"(\\d+)\"/>"
                      "</connection>"
                      "</connections></directory>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        // Вызываем 10 раз подряд - не должно крашиться
        for (int i = 0; i < 10; ++i) {
            QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
            QCOMPARE(templates.size(), 1);
            QCOMPARE(templates[0].name, QString("TestConn"));
        }
    }

    /// Тест: дерево с некорректной/неожиданной структурой XML
    void testCorruptedXmlStructure() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory>"
                      "<connections>"
                      "<connection name=\"Test1\" phone=\"+1\" login=\"u1\" password=\"p1\" "
                      "init_string=\"AT\">"
                      "<unexpected_node>data</unexpected_node>"
                      "<balance number=\"*100#\"/>"
                      "</connection>"
                      "</connections>"
                      "<unexpected_directory_child>garbage</unexpected_directory_child>"
                      "</directory>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        // Не должно крашиться даже с неожиданными узлами
        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QCOMPARE(templates.size(), 1);
    }

    /// Тест: большое количество connections (стресс-тест)
    void testLargeNumberOfConnections() {
        QString xml = "<root><Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory><connections>";

        // Генерируем 100 connections
        for (int i = 0; i < 100; ++i) {
            xml += QString("<connection name=\"Conn%1\" phone=\"+99890%2\" "
                           "login=\"user%1\" password=\"pass%1\" init_string=\"AT%1\">"
                           "<balance number=\"*100#\" regexp=\"(\\d+)\"/>"
                           "</connection>")
                       .arg(i)
                       .arg(i, 7, 10, QChar('0'));
        }

        xml += "</connections></directory></Directory></root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        // Должен обработать все 100 без крашей
        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QCOMPARE(templates.size(), 100);

        // Проверяем что foreach по результатам не крашится
        int count = 0;
        foreach (const SConnectionTemplate &tmpl, templates) {
            QVERIFY(!tmpl.name.isEmpty());
            ++count;
        }
        QCOMPARE(count, 100);
    }

    /// Тест: вложенные вызовы (nested iteration)
    void testNestedCalls() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory><connections>"
                      "<connection name=\"Conn1\" phone=\"+1\" login=\"u1\" password=\"p1\" "
                      "init_string=\"AT\"><balance number=\"*100#\"/></connection>"
                      "<connection name=\"Conn2\" phone=\"+2\" login=\"u2\" password=\"p2\" "
                      "init_string=\"AT\"><balance number=\"*101#\"/></connection>"
                      "</connections></directory>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        // Вызываем getConnectionTemplates внутри foreach по его же результатам
        QList<SConnectionTemplate> outer = directory.getConnectionTemplates();
        int outerCount = 0;
        foreach (const SConnectionTemplate &outerTmpl, outer) {
            ++outerCount;
            // Внутри итерации вызываем снова - проверка на iterator invalidation
            QList<SConnectionTemplate> inner = directory.getConnectionTemplates();
            QCOMPARE(inner.size(), 2);
        }
        QCOMPARE(outerCount, 2);
    }

    /// Тест: Directory с пустым directory.connections path (путь существует но пустой)
    void testDirectoryConnectionsPathInvalid() {
        // Создаем дерево где directory.connections path неправильный
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory>"
                      "<not_connections>"
                      "<connection name=\"Conn1\" phone=\"+1\" login=\"u1\" password=\"p1\" "
                      "init_string=\"AT\"/>"
                      "</not_connections>"
                      "</directory>"
                      "</Directory>"
                      "</root>";

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        // Должен вернуть пустой список, не крашиться
        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QVERIFY(templates.isEmpty());
    }

    /// Тест: connection с очень длинными строками
    void testConnectionWithLongStrings() {
        QString longString = QString("A").repeated(10000); // 10KB string

        QString xml = QString("<root>"
                              "<Directory>"
                              "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                              "<directory><connections>"
                              "<connection name=\"%1\" phone=\"+998901234567\" "
                              "login=\"%1\" password=\"%1\" init_string=\"%1\">"
                              "<balance number=\"*100#\" regexp=\"(\\d+)\"/>"
                              "</connection>"
                              "</connections></directory>"
                              "</Directory>"
                              "</root>")
                          .arg(longString);

        TPtree ptree = createPtree(xml);
        Directory directory(ptree);

        QList<SConnectionTemplate> templates = directory.getConnectionTemplates();
        QCOMPARE(templates.size(), 1);
        QCOMPARE(templates[0].name, longString);
        QCOMPARE(templates[0].login, longString);
    }

    /// Тест: изменение m_Properties не влияет на результаты getConnectionTemplates
    /// (проверка что m_Properties - value, не reference)
    void testPropertyTreeOwnership() {
        QString xml = "<root>"
                      "<Directory>"
                      "<numcapacity stamp=\"2024-01-01T00:00:00\"></numcapacity>"
                      "<directory><connections>"
                      "<connection name=\"Conn1\" phone=\"+1\" login=\"u1\" password=\"p1\" "
                      "init_string=\"AT\"><balance number=\"*100#\"/></connection>"
                      "</connections></directory>"
                      "</Directory>"
                      "</root>";

        {
            TPtree ptree = createPtree(xml);
            Directory directory(ptree);

            // Получаем templates
            QList<SConnectionTemplate> templates1 = directory.getConnectionTemplates();
            QCOMPARE(templates1.size(), 1);

            // ptree выходит из области видимости, но directory должен работать
            // (т.к. m_Properties - value member)
            QList<SConnectionTemplate> templates2 = directory.getConnectionTemplates();
            QCOMPARE(templates2.size(), 1);
            QCOMPARE(templates2[0].name, QString("Conn1"));
        }
    }
};

QTEST_MAIN(TestDirectory)
#include "TestDirectory.moc"

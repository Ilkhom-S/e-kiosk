#include "MySqlDatabaseProxy.h"
#include "SQLiteDatabaseProxy.h"

IDatabaseProxy *g_databaseProxy = nullptr;

//---------------------------------------------------------------------------
IDatabaseProxy *IDatabaseProxy::getInstance(IDatabaseQueryChecker *aErrorChecker, Type aType) {
    try {
        if (!g_databaseProxy) {
            if (!aErrorChecker) {
                return nullptr;
            }

            switch (aType) {
            case MySql:
                g_databaseProxy = new MySqlDatabaseProxy();
                break;
            case SQLite:
                g_databaseProxy = new SQLiteDatabaseProxy();
                break;
            default:
                return nullptr;
            }

            g_databaseProxy->setQueryChecker(aErrorChecker);
        }

        return g_databaseProxy;
    } catch (...) {
    }

    return nullptr;
}

//---------------------------------------------------------------------------
void IDatabaseProxy::freeInstance(IDatabaseProxy *aProxy) {
    if (g_databaseProxy) {
        delete aProxy;
        g_databaseProxy = nullptr;
    }
}

//---------------------------------------------------------------------------

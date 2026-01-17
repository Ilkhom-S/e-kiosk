# Database Service

The Database Service provides database connectivity and operations for the EKiosk system.

## Overview

The Database Service (`IDatabaseService`) manages:

- Database connections and connection pooling
- SQL query execution
- Transaction management
- Database schema operations
- Connection health monitoring

## Interface

```cpp
class IDatabaseService : public QObject {
    Q_OBJECT

public:
    /// Execute SELECT query and return results
    virtual QSqlQuery executeQuery(const QString &query,
                                 const QVariantMap &parameters = QVariantMap()) = 0;

    /// Execute INSERT/UPDATE/DELETE query
    virtual bool executeCommand(const QString &query,
                              const QVariantMap &parameters = QVariantMap()) = 0;

    /// Start database transaction
    virtual bool beginTransaction() = 0;

    /// Commit database transaction
    virtual bool commitTransaction() = 0;

    /// Rollback database transaction
    virtual bool rollbackTransaction() = 0;

    /// Check if database connection is valid
    virtual bool isConnected() const = 0;

    /// Get database connection info
    virtual QString getConnectionInfo() const = 0;

    // ... additional methods for schema management
};
```

## Connection Management

### Database Connection

```cpp
// Get database service from core
auto dbService = core->getDatabaseService();

if (!dbService) {
    LOG(log, LogLevel::Error, "Database service not available");
    return;
}

// Check connection status
if (!dbService->isConnected()) {
    LOG(log, LogLevel::Error, "Database connection lost");
    return;
}
```

### Connection Information

```cpp
// Get connection details for logging
QString connInfo = dbService->getConnectionInfo();
LOG(log, LogLevel::Info, QString("Database connection: %1").arg(connInfo));
```

## Query Execution

### SELECT Queries

```cpp
// Execute SELECT query with parameters
QVariantMap params;
params["userId"] = userId;
params["status"] = "active";

QSqlQuery query = dbService->executeQuery(
    "SELECT id, name, email FROM users WHERE id = :userId AND status = :status",
    params
);

if (query.isActive()) {
    while (query.next()) {
        int id = query.value("id").toInt();
        QString name = query.value("name").toString();
        QString email = query.value("email").toString();

        // Process user data
        processUser(id, name, email);
    }
} else {
    LOG(log, LogLevel::Error, "Failed to execute user query");
}
```

### INSERT/UPDATE/DELETE Operations

```cpp
// Execute INSERT command
QVariantMap insertParams;
insertParams["name"] = "John Doe";
insertParams["email"] = "john@example.com";
insertParams["status"] = "active";

bool success = dbService->executeCommand(
    "INSERT INTO users (name, email, status) VALUES (:name, :email, :status)",
    insertParams
);

if (!success) {
    LOG(log, LogLevel::Error, "Failed to insert user");
}
```

### Parameterized Queries

```cpp
// Update user with multiple parameters
QVariantMap updateParams;
updateParams["userId"] = 123;
updateParams["name"] = "Jane Doe";
updateParams["email"] = "jane@example.com";

bool updated = dbService->executeCommand(
    "UPDATE users SET name = :name, email = :email WHERE id = :userId",
    updateParams
);

if (!updated) {
    LOG(log, LogLevel::Error, "Failed to update user");
}
```

## Transaction Management

### Transaction Scope

```cpp
// Execute operations within transaction
if (!dbService->beginTransaction()) {
    LOG(log, LogLevel::Error, "Failed to begin transaction");
    return;
}

try {
    // Execute multiple operations
    QVariantMap params1, params2;

    // Update user balance
    params1["userId"] = userId;
    params1["amount"] = -100.00;
    dbService->executeCommand(
        "UPDATE accounts SET balance = balance + :amount WHERE user_id = :userId",
        params1
    );

    // Log transaction
    params2["userId"] = userId;
    params2["amount"] = 100.00;
    params2["type"] = "withdrawal";
    dbService->executeCommand(
        "INSERT INTO transactions (user_id, amount, type) VALUES (:userId, :amount, :type)",
        params2
    );

    // Commit transaction
    if (!dbService->commitTransaction()) {
        LOG(log, LogLevel::Error, "Failed to commit transaction");
        dbService->rollbackTransaction();
    }

} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Transaction failed: %1").arg(e.what()));
    dbService->rollbackTransaction();
}
```

## Usage in Plugins

Database Service is commonly used in data management plugins:

```cpp
class UserManagerPlugin : public SDK::Plugin::IPlugin {
public:
    bool initialize(SDK::Plugin::IKernel *kernel) override {
        mCore = dynamic_cast<SDK::PaymentProcessor::ICore*>(
            kernel->getInterface(SDK::PaymentProcessor::CInterfaces::ICore));

        if (mCore) {
            mDatabaseService = mCore->getDatabaseService();
            mLog = kernel->getLog("UserManager");
        }

        return true;
    }

    QList<User> getActiveUsers() {
        QList<User> users;

        if (!mDatabaseService || !mDatabaseService->isConnected()) {
            LOG(mLog, LogLevel::Error, "Database service unavailable");
            return users;
        }

        try {
            QSqlQuery query = mDatabaseService->executeQuery(
                "SELECT id, name, email FROM users WHERE status = 'active' ORDER BY name"
            );

            while (query.next()) {
                User user;
                user.id = query.value("id").toInt();
                user.name = query.value("name").toString();
                user.email = query.value("email").toString();
                users.append(user);
            }

        } catch (const std::exception &e) {
            LOG(mLog, LogLevel::Error, QString("Failed to load users: %1").arg(e.what()));
        }

        return users;
    }

    bool createUser(const QString &name, const QString &email) {
        if (!mDatabaseService || !mDatabaseService->isConnected()) {
            LOG(mLog, LogLevel::Error, "Database service unavailable");
            return false;
        }

        QVariantMap params;
        params["name"] = name;
        params["email"] = email;
        params["status"] = "active";

        return mDatabaseService->executeCommand(
            "INSERT INTO users (name, email, status) VALUES (:name, :email, :status)",
            params
        );
    }

private:
    IDatabaseService *mDatabaseService;
    ILog *mLog;
};
```

## Error Handling

```cpp
try {
    QSqlQuery query = dbService->executeQuery("SELECT * FROM users");

    if (!query.isActive()) {
        LOG(log, LogLevel::Error, QString("Query failed: %1").arg(query.lastError().text()));
        return;
    }

    // Process results...

} catch (const QSqlError &e) {
    LOG(log, LogLevel::Error, QString("Database error: %1").arg(e.text()));
} catch (const std::exception &e) {
    LOG(log, LogLevel::Error, QString("Unexpected database error: %1").arg(e.what()));
}
```

## Connection Pooling

The Database Service automatically manages connection pooling:

```cpp
// Service handles connection pooling internally
// No manual connection management needed in plugins
auto dbService = core->getDatabaseService();

// Each call uses an available connection from the pool
QSqlQuery query = dbService->executeQuery("SELECT COUNT(*) FROM users");
```

## Schema Operations

```cpp
// Check if table exists
QSqlQuery checkQuery = dbService->executeQuery(
    "SELECT name FROM sqlite_master WHERE type='table' AND name='users'"
);

if (!checkQuery.next()) {
    // Create table if it doesn't exist
    bool created = dbService->executeCommand(
        "CREATE TABLE users (id INTEGER PRIMARY KEY, name TEXT, email TEXT, status TEXT)"
    );

    if (!created) {
        LOG(log, LogLevel::Error, "Failed to create users table");
    }
}
```

## Thread Safety

Database operations are thread-safe. Each thread gets its own connection from the pool.

## Dependencies

- Settings Service (for database configuration)
- Logging Service (for query logging and errors)

## See Also

- [Settings Service](settings.md) - Database configuration
- [Network Service](network.md) - Remote database connections
- [Event Service](event.md) - Database change notifications

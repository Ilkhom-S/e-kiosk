/\* @file Database Management

## Overview

The EKiosk database uses SQLite with a versioning system to track and apply schema changes through database migrations.

## Architecture

- **Base Schema**: `empty_db.sql` - Complete database schema for fresh installations
- **Versioning**: Tracked via `device_param` table (`name='db_patch'`, `value=version`)
- **Migrations**: Future patches stored in `scripts/` folder (e.g., `db_patch_1.sql`, `db_patch_2.sql`)
- **Resource System**: All SQL scripts compiled into the binary via Qt Resource System (Database.qrc)

## File Structure

```
DatabaseUtils/
├── DatabaseUtils.cpp               # Database initialization and query execution
├── DatabaseUtils.h                 # Database interface
├── Database.qrc                    # Qt resource file (embeds SQL scripts)
├── scripts/
│   └── empty_db.sql               # Complete base schema
└── README.md                       # This file
```

## How the Database System Works

### Fresh Installation

1. Application starts, checks if database file exists
2. `DatabaseUtils::initialize()` detects 0 tables
3. Applies `empty_db.sql` - complete schema in one script
4. All tables, indexes, and constraints created at once
5. `device_param` table initialized with `db_patch` version

```
[Fresh Start] → empty_db.sql → Ready to use
```

### Upgrade with Migrations

1. Existing database with version < latest
2. `DatabaseUtils::initialize()` detects tables and reads current version
3. For each registered migration in `Patches[]` array:
   - If `currentVersion < migrationVersion`, apply migration
4. Continues until database reaches latest version

```
[Old Version 1] → db_patch_2.sql → db_patch_3.sql → [Current Version 3]
```

### Code Flow (DatabaseUtils.cpp)

```cpp
bool DatabaseUtils::initialize() {
    // 1. Check if tables exist
    if (databaseTableCount() == 0) {
        // Fresh start: apply base schema
        updateDatabase(empty_db.sql);     // Creates v0 or v1
    }

    // 2. Apply any registered migrations
    for (const auto &patch : CDatabaseUtils::Patches) {
        if (databasePatch() < patch.version) {
            // Only apply if not already applied
            updateDatabase(patch.script);
        }
    }
}
```

## Database Schema Overview

### Core Tables

#### `device`

- `id`: INTEGER PRIMARY KEY
- `name`: VARCHAR(255) UNIQUE
- `type`: INTEGER (device type identifier)
- Device information and configuration

#### `device_param`

- `id`: INTEGER PRIMARY KEY
- `name`: VARCHAR(100) - parameter name
- `value`: TEXT - parameter value
- `fk_device_id`: INTEGER FOREIGN KEY
- Key-value parameters per device (including `db_patch` version)

#### `payment`

- `id`: INTEGER PRIMARY KEY
- `amount`: DECIMAL(10,2)
- `nominal`: DECIMAL(10,2)
- `currency`: VARCHAR(10)
- Payment records and history

#### `payment_note`

- `id`: INTEGER PRIMARY KEY
- `date`: DATETIME
- `fk_payment_id`: INTEGER FOREIGN KEY
- Payment notes with timestamp index

#### `dispensed_note`

- `id`: INTEGER PRIMARY KEY
- `fk_payment_id`: INTEGER FOREIGN KEY
- Dispensed item records

#### `encashment_param`

- `id`: INTEGER PRIMARY KEY
- `fk_device_id`: INTEGER FOREIGN KEY
- Encashment (cash removal) parameters

See `empty_db.sql` for complete schema definition.

## How to Add a Database Migration

### Step 1: Create Migration File

Name: `db_patch_N.sql` (where N is the next version number)
Location: `apps/EKiosk/src/DatabaseUtils/scripts/`

### Step 2: Write SQL Migration

```sql
--
-- Migration N: Description of what this migration does
-- Affected tables: table1, table2
-- Backward compatibility notes if any
--

-- Your SQL statements
ALTER TABLE `payment` ADD COLUMN `new_field` VARCHAR(255);
CREATE INDEX IF NOT EXISTS `i__payment__new_field` ON `payment` (`new_field`);

-- IMPORTANT: Update version at the end
UPDATE `device_param` SET `value` = N WHERE `name` = 'db_patch';
```

### Step 3: Register Migration in Database.qrc

Edit `Database.qrc` and add your patch file:

```xml
<qresource prefix="/scripts">
    <file>empty_db.sql</file>
    <file>db_patch_1.sql</file>     <!-- Add new patches in order -->
    <file>db_patch_2.sql</file>
</qresource>
```

### Step 4: Register in DatabaseUtils.cpp

Edit `DatabaseUtils.cpp` and update the `Patches[]` array:

```cpp
const struct {
    int version;
    QString script;
} Patches[] = {
    {1, ":/scripts/db_patch_1.sql"},
    {2, ":/scripts/db_patch_2.sql"},
    // Add new patches here in order
};
```

The migration loop will apply them automatically when `databasePatch() < patch.version`.

### Step 5: Test Your Migration

**Fresh Install Test** (should include patch changes):

```bash
rm build/macos-qt6/bin/user/data.db
cmake --build build/macos-qt6 --target ekiosk
# Verify database structure includes new migration changes
sqlite3 build/macos-qt6/bin/user/data.db ".schema"
```

**Upgrade Test** (if you have old databases):

```bash
# Keep old database at version 1
# Run app - should apply patch 2
sqlite3 build/macos-qt6/bin/user/data.db \
  "SELECT value FROM device_param WHERE name='db_patch';"
# Should show: 2
```

## Migration Best Practices

### Schema Design

- Use UNIQUE constraints where appropriate (e.g., device names)
- Add indexes for foreign keys and frequently queried columns
- Use PRAGMA foreign_keys = ON for referential integrity
- Use DECIMAL(10,2) for currency/amount fields

### Naming Conventions

- **Tables**: snake_case (e.g., `payment_note`, `dispensed_note`)
- **Columns**: snake_case (e.g., `fk_device_id`, `created_date`)
- **Indexes**: `i__tablename__column` (e.g., `i__payment__date`)
- **Constraints**: descriptive names (e.g., `unique_device_name`)

### Data Migration

- Always use INSERT INTO ... SELECT for copying data (not manual values)
- For renaming columns:
  1. Create new table with correct schema
  2. Copy data: `INSERT INTO new_table SELECT ... FROM old_table`
  3. Drop old table and rename: `ALTER TABLE new_table RENAME TO old_table`
- Use `IF NOT EXISTS` and `IF EXISTS` for idempotency

### Version Control

- Each migration is independent and atomic
- Migrations must be idempotent (safe to run twice)
- Use `INSERT OR REPLACE` / `ON CONFLICT` for safe updates
- This allows recovery by reapplying migrations

### Example: Complete Migration

```sql
--
-- Migration 1: Add payment notes tracking
-- Affected tables: payment_note (new table)
--

CREATE TABLE IF NOT EXISTS `payment_note` (
  `id`                INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `payment_id`        INTEGER NOT NULL,
  `value`             TEXT,
  `date`              DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  FOREIGN KEY (`payment_id`) REFERENCES `payment` (`id`)
);

CREATE INDEX IF NOT EXISTS `i__payment_note__date` ON `payment_note` (`date`);
CREATE INDEX IF NOT EXISTS `i__payment_note__payment_id` ON `payment_note` (`payment_id`);

UPDATE `device_param` SET `value` = 1 WHERE `name` = 'db_patch';
```

## Testing Database Migrations

### Inspect Database Structure

```bash
# Connect to database
sqlite3 build/macos-qt6/bin/user/data.db

# Show all tables
.tables

# Show schema for a table
.schema payment

# Show all indexes
SELECT name FROM sqlite_master WHERE type='index' ORDER BY name;

# Show table structure details
PRAGMA table_info(payment);

# Show current version
SELECT value FROM device_param WHERE name='db_patch';
```

### Verify Migrations Applied

```bash
# Check version
sqlite3 build/macos-qt6/bin/user/data.db \
  "SELECT value FROM device_param WHERE name='db_patch';"

# Check for specific table (after adding it with a migration)
sqlite3 build/macos-qt6/bin/user/data.db \
  "SELECT name FROM sqlite_master WHERE type='table' AND name='new_table';"
```

## Troubleshooting

### Migration Not Applied?

1. **Check version**:

   ```bash
   sqlite3 data.db "SELECT value FROM device_param WHERE name='db_patch';"
   ```

   - Should show the latest migration version

2. **Check for SQL errors**: Look in application logs for SQL execution errors

3. **Verify Database.qrc**: Ensure patch file is listed and compiled

   ```bash
   # Rebuild to ensure resources are compiled
   cmake --build build/macos-qt6 --target ekiosk
   ```

4. **Check Patches[] array**: Verify migration is registered in DatabaseUtils.cpp

### Schema Issues?

```bash
# Inspect current schema
sqlite3 data.db ".schema" | less

# Get specific table info
sqlite3 data.db "PRAGMA table_info(table_name);"

# Check indexes
sqlite3 data.db ".indexes table_name"
```

### Database Locked/Corrupted?

```bash
# Backup current database
cp build/macos-qt6/bin/user/data.db build/macos-qt6/bin/user/data.db.backup

# Check integrity
sqlite3 build/macos-qt6/bin/user/data.db "PRAGMA integrity_check;"

# Vacuum to optimize
sqlite3 build/macos-qt6/bin/user/data.db "VACUUM;"
```

## Resource System & Compilation

SQL scripts are compiled into the binary via Qt Resource System:

**File**: `Database.qrc`

- Lists all SQL script files
- Qt moc compiler processes this file during build
- Scripts become embedded resources: `:/scripts/empty_db.sql`, `:/scripts/db_patch_1.sql`, etc.

**Code Access**: `QFile(":/scripts/db_patch_1.sql")` loads from embedded resources

**Build Requirements**:

- CMakeLists.txt must include: `qt_add_resources(ekiosk Database.qrc)`
- Scripts must be listed in Database.qrc
- CMake runs Qt moc compiler to generate qrc_Database.cpp

## References

- Implementation: [DatabaseUtils.cpp](DatabaseUtils.cpp)
- Resource Configuration: [Database.qrc](Database.qrc)
- SQLite Documentation: https://www.sqlite.org/docs.html
- SQLite Pragma Statements: https://www.sqlite.org/pragma.html

## TODO

Future enhancements for the database system:

- [ ] **Database Encryption**: Implement password-based encryption for SQLite database file
  - Use SQLite encryption extension (e.g., SQLCipher)
  - Add password configuration to initialization
  - Secure credential storage for database key

- [ ] **Migration Rollback Mechanism**: Implement safe migration rollback on patch failure
  - Add transaction checkpoints before migration
  - Automatic backup before applying patches
  - Rollback capability if migration SQL execution fails
  - Maintain migration history/audit log
*/


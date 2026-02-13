/\* @file Database Scripts README

## Overview

The EKiosk database uses SQLite with a versioning system to track and apply schema changes.

### Current Version: 12 (Consolidated)

After Q t4→Qt5/Qt6 migration, all patches (6-12) have been consolidated into `empty_db.sql`.

## File Structure

```
scripts/
├── empty_db.sql                    # Full schema at version 12 (fresh start)
├── db_patch_6.sql                  # Historical: Unique index on device.name
├── db_patch_7.sql                  # Historical: Add external column
├── db_patch_8.sql                  # Historical: Create dispensed_note table
├── db_patch_9.sql                  # Historical: Add payment_note date index
├── db_patch_10.sql                 # Historical: Table structure improvements
├── db_patch_11.sql                 # Historical: Add encashment_param table
├── db_patch_12.sql                 # Historical: Cleanup old tables
└── MIGRATION_GUIDE.md              # Guide for writing future patches
```

## How It Works

### Fresh Installation

1. Database file doesn't exist
2. `DatabaseUtils::initialize()` detects 0 tables
3. Applies `empty_db.sql` → Full schema at version 12
4. No patches needed

```
Database Lifecycle:
[Fresh Start] → empty_db.sql → Version 12

[Old Qt4 DB] → empty_db.sql → patches 6-12 → Version 12
              (v5 initial)    (if needed)
```

### Upgrade from Old Version (Qt4 Legacy)

1. Existing database has version < 5
2. \`DatabaseUtils::initialize()\` checks patch version
3. Applies patches 6-12 sequentially
4. Reaches version 12

```cpp
// Code in DatabaseUtils.cpp:
if (!m_Database.isConnected()) throw error;
if (databaseTableCount() == 0) {
    updateDatabase(empty_db.sql);     // New: v12, Upgrade: v5 → v6
}
for (each patch) {
    if (databasePatch() < patch.version) {
        updateDatabase(patch.script);  // Only if needed
    }
}
```

## Schema Consolidation History

### Before (Qt4 Migration)

- empty_db.sql: Basic schema at version 5
- 7 patches: Gradual updates (6 → 12) applied sequentially

**Problem**: New installations needed to apply 7 patches

### After (Fresh Start)

- empty_db.sql: Complete schema at version 12 (patches 6-12 merged)
- Patches: Kept for backward compatibility (upgrade path only)

**Benefit**: New installations are complete from start, no patches needed

## Key Changes in Consolidated Schema (empty_db.sql v12)

1. **Patch 6**: Added UNIQUE index on device.name
2. **Patch 7**: Added `external` column to payment_param
3. **Patch 8**: Created `dispensed_note` table, added `dispenser_report` column
4. **Patch 9**: Added index on payment_note.date
5. **Patch 10**: Changed payment_note.nominal to DECIMAL(10,2)
6. **Patch 11**: Created `encashment_param` table
7. **Patch 12**: Cleanup and version finalization

## Future Database Changes

For schema changes after consolidation:

1. **New versions**: db_patch_13.sql, db_patch_14.sql, etc.
2. **Location**: Same scripts/ folder
3. **Format**: See [MIGRATION_GUIDE.md](MIGRATION_GUIDE.md)
4. **Resource**: Add to Database.qrc as:
   ```xml
   <file>scripts/db_patch_13.sql</file>
   ```
5. **Code**: Update Patches[] array in DatabaseUtils.cpp:
   ```cpp
   {13, ":/scripts/db_patch_13.sql"},
   // etc.
   ```

## Database Versioning Table

| Version | Source                      | Status      | Notes                   |
| ------- | --------------------------- | ----------- | ----------------------- |
| ≤5      | Legacy Qt4                  | Deprecated  | Pre-migration schema    |
| 6       | db_patch_6.sql              | Historical  | Unique device index     |
| 7       | db_patch_7.sql              | Historical  | payment_param.external  |
| 8       | db_patch_8.sql              | Historical  | dispensed_note table    |
| 9       | db_patch_9.sql              | Historical  | payment_note.date index |
| 10      | db_patch_10.sql             | Historical  | DECIMAL precision       |
| 11      | db_patch_11.sql             | Historical  | encashment_param table  |
| 12      | empty_db.sql (consolidated) | **Current** | Complete schema         |

## Testing Database Migrations

### Test Fresh Installation

```bash
# Remove old database
rm build/macos-qt6/bin/user/data.db

# Run ekiosk - should create v12 from empty_db.sql
./build/macos-qt6/bin/ekiosk.app/Contents/MacOS/ekiosk

# Verify version
sqlite3 build/macos-qt6/bin/user/data.db \
  "SELECT value FROM device_param WHERE name = 'db_patch';"
# Output should be: 12
```

### Test Upgrade (Simulate Old DB)

```bash
# Create old-version database
sqlite3 build/macos-qt6/bin/user/data.db < scripts/empty_db_v5.sql

# Run ekiosk - should apply patches to reach v12
./build/macos-qt6/bin/ekiosk.app/Contents/MacOS/ekiosk

# Verify version
sqlite3 build/macos-qt6/bin/user/data.db \
  "SELECT value FROM device_param WHERE name = 'db_patch';"
# Output should be: 12
```

## Database Resource Compilation

The scripts are compiled into the binary via Qt Resource System:

**File**: `Database.qrc`

```xml
<RCC>
    <qresource prefix="/">
        <file>scripts/empty_db.sql</file>
        <file>scripts/db_patch_6.sql</file>
        ...
        <file>scripts/db_patch_12.sql</file>
    </qresource>
</RCC>
```

**Build Process**:

1. CMakeLists.txt with `file(GLOB_RECURSE RESOURCE_FILES src/*.qrc)` finds Database.qrc
2. Qt moc compiler generates qrc_Database.cpp
3. Scripts become embedded resources: `:/scripts/empty_db.sql`, etc.
4. Code accesses via `QFile(":/scripts/empty_db.sql")`

## Troubleshooting

### Database Patch Not Applied?

1. Check version:
   ```bash
   sqlite3 data.db "SELECT value FROM device_param WHERE name='db_patch';"
   ```
2. Check logs for SQL errors
3. Verify Database.qrc was compiled and scripts found
4. Rebuild: `cmake --build . --target ekiosk`

### Schema Mismatch?

1. Inspect schema:
   ```bash
   sqlite3 data.db ".schema" | less
   sqlite3 data.db "SELECT sql FROM sqlite_master WHERE type='table';"
   ```
2. Compare with empty_db.sql / patches
3. Restore backup and retry upgrade

### Adding Debug Info?

Edit DatabaseUtils.cpp lines 60-75 to log applied patches:

```cpp
LOG(m_Log, LogLevel::Normal,
    QString("Applied patch version %1: %2").arg(patch.version).arg(patch.script));
```

## References

- Implementation: [DatabaseUtils.cpp](DatabaseUtils.cpp)
- Migration Guide: [MIGRATION_GUIDE.md](MIGRATION_GUIDE.md)
- Resource System: [Database.qrc](Database.qrc)
- SQLite Docs: https://www.sqlite.org/docs.html
  \

2. Compare with empty_db.sql / patches
3. Restore backup and retry upgrade

### Adding Debug Info?

Edit DatabaseUtils.cpp lines 60-75 to log applied patches:

```cpp
LOG(m_Log, LogLevel::Normal,
    QString("Applied patch version %1: %2").arg(patch.version).arg(patch.script));
```

## References

- Implementation: [DatabaseUtils.cpp](DatabaseUtils.cpp)
- Migration Guide: [MIGRATION_GUIDE.md](MIGRATION_GUIDE.md)
- Resource System: [Database.qrc](Database.qrc)
- SQLite Docs: <https://www.sqlite.org/docs.html>
  \*/

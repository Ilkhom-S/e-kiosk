/\* @file Database Migration Guide - How to Write Future Patches

## Overview

The EKiosk database supports versioning through the `device_param` table.
The current version is 12 (stored in device_param where name='db_patch' and value=12).

## Fresh Start Architecture (v12+)

After consolidating patches 6-12 into empty_db.sql:

- **New installations**: Start with complete schema at version 12
- **Upgrades**: Apply patches starting from whatever version is running
- **Future patches**: Add db_patch_13.sql, db_patch_14.sql, etc. following this guide

## How to Write a New Database Patch

### Step 1: Create the Patch File

Name the file: `db_patch_NN.sql` (where NN is the next version number)

Location: `apps/EKiosk/src/DatabaseUtils/scripts/db_patch_NN.sql`

### Step 2: Implement Your Changes

Write all SQL migrations needed for this version:

```sql
-- Start with a comment describing the patch
--
-- Description of what this patch does
-- Affected tables: table1, table2
-- Backward compatibility: list any compatibility notes
--

-- Your SQL statements here
ALTER TABLE `payment` ADD COLUMN `new_field` VARCHAR(255);
CREATE INDEX IF NOT EXISTS `i__payment__new_field` ON `payment` (`new_field`);

-- Update the patch version at the end
INSERT OR REPLACE INTO `device_param` (`value`, `name`, `fk_device_id`)
VALUES(NN, 'db_patch', (SELECT id FROM device WHERE type = 6 LIMIT 1));
```

### Step 3: Register in Database.qrc

Add the new patch to `apps/EKiosk/src/DatabaseUtils/Database.qrc`:

```xml
<qresource prefix="/scripts">
    <file>empty_db.sql</file>
    <file>db_patch_6.sql</file>
    ...
    <file>db_patch_NN.sql</file>  <!-- Add here in order -->
</qresource>
```

### Step 4: Update C++ Database Code

If your change requires logic changes, update:

- `apps/EKiosk/src/DatabaseUtils/DatabaseUtils.cpp` - addUpdateDatabase() method
- Handle version progression: if (currentVersion < NN) applyPatchNN()

### Step 5: Test Your Patch

1. **Fresh install test**:

   ```bash
   rm build/macos-qt6/bin/user/data.db
   cmake --build build/macos-qt6 --target ekiosk
   ```

   Verify empty_db.sql includes patch content

2. **Upgrade test** (from older version):
   - Keep user/data.db at version 11 (old version)
   - Run ekiosk
   - Verify patch executes and version increments to 12

### Step 6: Update empty_db.sql (When Consolidating)

Periodically consolidate patches into empty_db.sql:

1. Combine all patch changes into empty_db.sql
2. Set initial version to final patch number
3. Move old patches to `deprecated/` folder
4. Update Database.qrc to remove old patches
5. Document the consolidation in CHANGELOG

## Best Practices

### Schema Design

- Use UNIQUE constraints where appropriate
- Add indexes for foreign keys and frequently queried columns
- Use PRAGMA foreign_keys = ON for validation
- Use DECIMAL(10,2) for currency/amount fields

### Naming Conventions

- Table names: snake_case (e.g., `payment_note`)
- Column names: snake_case (e.g., `fk_payment_id`)
- Index names: `i__tablename__column` (e.g., `i__payment__date`)
- Constraints: `[unique_desc_UNIQUE_COLUMNS]` (e.g., `[unique_param_for_fk_device_id]`)

### Data Migration

- Always use INSERT INTO ... SELECT for data migration (not manual values)
- Rename old tables before creating new ones: `ALTER TABLE old RENAME TO old_backup`
- Copy data from old to new table
- Drop old tables in a separate patch (safer rollback path)

### Example: Renaming a Column with Data Migration

```sql
-- Step 1: Create new table with correct schema
CREATE TABLE IF NOT EXISTS `payment_new` (
  `id` INTEGER PRIMARY KEY,
  `new_column_name` VARCHAR(255),
  ...
);

-- Step 2: Copy data from old table
INSERT INTO `payment_new` SELECT id, old_column_name, ... FROM `payment`;

-- Step 3: In a later patch, drop old table
DROP TABLE `payment`;
ALTER TABLE `payment_new` RENAME TO `payment`;
```

### Version Control

- Each patch is atomic and independent
- Patches must be idempotent: running twice should be safe
- Use `IF NOT EXISTS` and `IF EXISTS` clauses
- Use `OR REPLACE` and `ON CONFLICT` for safe updates

## Example: Complete Patch

```sql
--
-- Patch 13: Add user audit logging
-- Affected tables: device_param (new)
-- Backward compatibility: No breaking changes
--

CREATE TABLE IF NOT EXISTS `audit_log` (
  `id`              INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `timestamp`       DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `user_id`         INTEGER,
  `action`          VARCHAR(50) NOT NULL,
  `table_name`      VARCHAR(50),
  `record_id`       INTEGER,
  `old_value`       TEXT,
  `new_value`       TEXT
);

CREATE INDEX IF NOT EXISTS `i__audit_log__timestamp` ON `audit_log` (`timestamp`);
CREATE INDEX IF NOT EXISTS `i__audit_log__user_id` ON `audit_log` (`user_id`);
CREATE INDEX IF NOT EXISTS `i__audit_log__action` ON `audit_log` (`action`);

INSERT OR REPLACE INTO `device_param` (`value`, `name`, `fk_device_id`)
VALUES(13, 'db_patch', (SELECT id FROM device WHERE type = 6 LIMIT 1));
```

## Troubleshooting

### Patch Not Applied?

1. Check: `SELECT * FROM device_param WHERE name = 'db_patch'` - see current version
2. Check logs: Look for SQL errors in app logs
3. Verify: Database.qrc includes the patch file
4. Verify: CMakeLists.txt uses GLOB_RECURSE to find patches

### Schema Issues?

```sql
-- Inspect current schema
.schema payment

-- List indexes
SELECT name FROM sqlite_master WHERE type='index' ORDER BY name;

-- Get table structure
PRAGMA table_info(payment);
```

### Rolling Back?

SQLite doesn't support transactions for DDL easily. For critical changes:

1. Backup database before deploying patch
2. Export data before schema changes
3. Have rollback patch prepared

## References

- [SQLite Documentation](https://www.sqlite.org/docs.html)
- [SQLite Pragma Statements](https://www.sqlite.org/pragma.html)
- [Foreign Keys](https://www.sqlite.org/foreignkeys.html)
- Current implementation: [DatabaseUtils.cpp](../DatabaseUtils.cpp)
  \SQLite doesn't support transactions for DDL easily. For critical changes:

1. Backup database before deploying patch
2. Export data before schema changes
3. Have rollback patch prepared

## References

- [SQLite Documentation](https://www.sqlite.org/docs.html)
- [SQLite Pragma Statements](https://www.sqlite.org/pragma.html)
- [Foreign Keys](https://www.sqlite.org/foreignkeys.html)
- Current implementation: [DatabaseUtils.cpp](../DatabaseUtils.cpp)
  \*/

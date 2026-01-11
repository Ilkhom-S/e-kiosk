# Multilanguage Support

EKiosk supports multiple languages for UI and receipts.

## Adding a New Language

- Add translation files (.ts) in the appropriate folder.
- Use Qt Linguist to edit translations.
- Update CMakeLists.txt to include new .ts files.

## Guidelines

- All user-facing strings must use tr() for translation.
- Test UI in all supported languages before release.

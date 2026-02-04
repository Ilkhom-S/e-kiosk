## macOS Tray Icon Theme Adaptation Issues

### Problem Description

The macOS tray icon in WatchServiceController had several theme adaptation issues:

1. **Context menu icons not inverting**: Menu item icons (play, stop, settings, etc.) were not properly adapting to light/dark theme changes
2. **Red status dot losing color**: When the service was disconnected, the red notification dot on the tray icon was appearing white in dark mode instead of staying red
3. **H glyph too small**: The "H" monogram glyph was too small for proper visibility in the macOS status bar

### Root Cause

- Missing `QStyleHints::colorSchemeChanged` signal connection to update icons when macOS theme switches
- Complex icon composition approach that didn't work reliably with macOS template image inversion
- SVG template glyph size was too small (32px tall in 64px canvas) for status bar display

### Solution Implemented

1. **Platform-agnostic badge painting**: Replaced complex icon composition with direct `QPainter` drawing of red badge on template pixmap
2. **Theme change connection**: Added `connect(QGuiApplication::styleHints(), &QStyleHints::colorSchemeChanged, ...)` to update icon on theme switches
3. **Increased glyph size**: Updated SVG templates to use 44px tall H glyph for better status bar visibility
4. **Template icon regeneration**: Regenerated all icon variants with proper "Template" suffix for macOS theme adaptation

### Current Status

✅ **Working**: The red dot now stays red in both light and dark modes, and icons update properly on theme changes

### Need More Analysis

While the current implementation works, further investigation is needed to:

- Verify long-term stability of the badge painting approach
- Test edge cases with different macOS versions and configurations
- Optimize performance of icon updates during theme switches
- Consider alternative approaches if issues arise in production

### Files Changed

- `apps/WatchServiceController/src/WatchServiceController.cpp`: Added badge painting method and theme change connection
- `assets/icons/templates/controller/*.svg`: Updated glyph sizes and naming
- Generated icon files: All variants regenerated with new templates

### Testing

- Build and run successful on macOS with Qt 6.8 LTS
- Theme switching tested manually - icons update correctly
- Red dot color preservation verified in both themes
  oller.cpp`: Added badge painting method and theme change connection
- `assets/icons/templates/controller/*.svg`: Updated glyph sizes and naming
- Generated icon files: All variants regenerated with new templates

### Testing

- Build and run successful on macOS with Qt 6.8 LTS
- Theme switching tested manually - icons update correctly
- Red dot color preservation verified in both themes

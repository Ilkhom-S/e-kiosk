# Build Scripts

This directory contains build automation scripts for the e-kiosk project.

## 📋 Scripts

### `clean-rebuild-macos.sh`

**Full clean rebuild for macOS (Qt6)**

Removes the entire build directory and rebuilds everything from scratch. Use this when:

- Interface changes (IKernel, IApplication, etc.)
- Major architectural changes
- Plugins crash with "pure virtual method called" errors
- After pulling changes that modify core interfaces

```bash
./scripts/clean-rebuild-macos.sh
```

**Time:** ~5-10 minutes (full rebuild)

---

### `clean-rebuild-windows.ps1`

**Full clean rebuild for Windows (MinGW Qt6)**

Windows equivalent of clean-rebuild-macos.sh.

```powershell
.\scripts\clean-rebuild-windows.ps1
```

**Note:** Update `CMAKE_PREFIX_PATH` to match your Qt6 installation path.

---

### `rebuild-plugins-only.sh`

**Fast plugin-only rebuild**

Removes plugin binaries and translations, then rebuilds only plugin targets. Use this when:

- Only plugin code changed (no interface changes)
- Faster iteration during plugin development
- Translation files need regeneration

```bash
./scripts/rebuild-plugins-only.sh
```

**Time:** ~1-2 minutes (plugins only)

---

## 🎯 When to Use Which Script

| Scenario                                     | Script                    | Why                                           |
| -------------------------------------------- | ------------------------- | --------------------------------------------- |
| **Interface changes** (IKernel, ICore, etc.) | `clean-rebuild-*.sh`      | Plugins must recompile against new interfaces |
| **Plugin crashes on startup**                | `clean-rebuild-*.sh`      | ABI mismatch between main app and plugins     |
| **Translation not loading**                  | `rebuild-plugins-only.sh` | Regenerates .qm files                         |
| **Plugin code changes only**                 | `rebuild-plugins-only.sh` | Faster than full rebuild                      |
| **After git pull with core changes**         | `clean-rebuild-*.sh`      | Ensures everything is in sync                 |
| **Normal development**                       | Regular build             | Use IDE or `cmake --build`                    |

---

## ⚠️ Important Notes

### Interface Changes Require Full Rebuild

When you modify:

- `include/SDK/Plugins/IKernel.h`
- `include/SDK/Plugins/IPlugin.h`
- `include/SDK/PaymentProcessor/ICore.h`
- Any other interface header

**Always run `clean-rebuild-*.sh`** to ensure plugins are rebuilt with the updated interface.

### Plugin ABI Compatibility

Plugins are dynamically loaded at runtime. If the main executable and plugins are compiled against different interfaces, you'll get:

- Pure virtual method called errors
- EXC_BAD_ACCESS crashes
- Undefined behavior

### Qt6 Cross-DLL Considerations

- QString temporaries cannot cross DLL boundaries safely
- Always cache concatenated strings before returning from functions called by plugins
- Use `detach()` if needed to force deep copy

---

## 🔧 Customization

### macOS Qt6 Path

Edit `clean-rebuild-macos.sh`:

```bash
-DCMAKE_PREFIX_PATH=/usr/local/opt/qt6 \
```

### Windows Qt6 Path

Edit `clean-rebuild-windows.ps1`:

```powershell
-DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64" `
```

### Build Type

Change `Debug` to `Release` for optimized builds:

```bash
-DCMAKE_BUILD_TYPE=Release \
```

### Parallel Jobs

Adjust `-j8` to match your CPU cores:

```bash
cmake --build "$BUILD_DIR" -j16
```

---

## 📝 Examples

### After Modifying IKernel

```bash
# Full rebuild required
./scripts/clean-rebuild-macos.sh
```

### After Fixing Plugin Bug

```bash
# Quick plugin rebuild
./scripts/rebuild-plugins-only.sh
```

### Daily Development

```bash
# Normal incremental build
cmake --build build/macos-qt6 -j8
```

---

## 🐛 Troubleshooting

### "Plugin not found" Errors

- Check `build/macos-qt6/bin/plugins/` directory exists
- Verify plugin files have `.dylib` extension (macOS) or `.dll` (Windows)
- Run `clean-rebuild-*.sh` to regenerate everything

### Crashes on Plugin Load

- Usually indicates interface mismatch
- Run `clean-rebuild-*.sh` to rebuild with current interfaces
- Check for pure virtual method errors in logs

### Translation Files Missing

- Run `rebuild-plugins-only.sh` to regenerate `.qm` files
- Check source `.ts` files exist in plugin directories

---

## 📚 See Also

---

## 🐛 Troubleshooting

### "Plugin not found" Errors

- Check `build/macos-qt6/bin/plugins/` directory exists
- Verify plugin files have `.dylib` extension (macOS) or `.dll` (Windows)
- Run `clean-rebuild-*.sh` to regenerate everything

### Crashes on Plugin Load

- Usually indicates interface mismatch
- Run `clean-rebuild-*.sh` to rebuild with current interfaces
- Check for pure virtual method errors in logs

### Translation Files Missing

- Run `rebuild-plugins-only.sh` to regenerate `.qm` files
- Check source `.ts` files exist in plugin directories

---

## 📚 See Also

- [docs/build-guide.md](../docs/build-guide.md) - Complete build instructions
- [docs/platform-compatibility.md](../docs/platform-compatibility.md) - Platform-specific notes
- [docs/qt6-iterator-safety.md](../docs/qt6-iterator-safety.md) - Qt6 best practices

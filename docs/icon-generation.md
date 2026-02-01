# Icon Generation for EKiosk Apps

This guide documents how to create, export, and integrate icons for EKiosk applications, supporting multiplatform workflows (Windows, macOS, Linux).

## Source Files

- All vector icon sources are organized by app in `assets/icons/templates/` subdirectories:
  - `tray/` : WatchServiceController (system tray app)
  - `kiosk/` : EKiosk (main kiosk application)
  - `card/` : Card-related applications
  - `updater/` : Updater applications
- Brand primary color: `#FA5300` (used in `tray-monogram.svg`).
- For macOS menu bar, provide a single-color (white glyph) SVG named `*-template.svg`.

## Recommended PNG Exports

- 1x: 16, 22, 24, 32, 48, 64
- 2x (retina): 32, 44, 48, 64, 96, 128
- For macOS template: export monochrome PNGs at 1x and 2x.

## Automated Export: `scripts/generate_icons.py`

Use the provided script to generate all required PNG and ICO files for each app:

```bash
python3 scripts/generate_icons.py --apps WatchServiceController,EKiosk --force
```

- The script automatically uses `assets/icons/templates/` as the source directory.
- Specify `--source <path>` to override the default template directory.
- The script detects and uses the best available converter: `cairosvg` (Python), Inkscape, `rsvg-convert`, or ImageMagick `convert`.
- Output is written to `apps/<app>/src/icons/`.

### Converter Requirements

- **Preferred:** `cairosvg` Python module (`pip install cairosvg Pillow`)
- **Fallbacks:** Inkscape (CLI), `rsvg-convert`, ImageMagick `convert`
- If you see a warning about `cairosvg` not being on PATH, add the user script directory to your shell PATH:
  - macOS: `export PATH="$HOME/Library/Python/3.x/bin:$PATH"`
  - Linux: `export PATH="$HOME/.local/bin:$PATH"`

## Manual Export Examples

Using Inkscape:

```bash
inkscape assets/icons/templates/tray/tray-monogram.svg --export-type=png --export-filename=assets/icons/templates/tray/tray-monogram-16.png --export-width=16 --export-height=16
inkscape assets/icons/templates/tray/tray-monogram-template.svg --export-type=png --export-filename=assets/icons/templates/tray/tray-monogram-template-16.png --export-width=16 --export-height=16
```

Using rsvg-convert:

```bash
rsvg-convert -w 16 -h 16 assets/icons/templates/tray/tray-monogram.svg -o assets/icons/templates/tray/tray-monogram-16.png
rsvg-convert -w 16 -h 16 assets/icons/templates/tray/tray-monogram-template.svg -o assets/icons/templates/tray/tray-monogram-template-16.png
```

## macOS Tips

- For menu bar (template) images, supply a monochrome PNG (alpha mask) and mark it as a template in Xcode/asset catalogs or let the system tint it.
- Keep both colored and template variants and select at runtime per platform.

## Integration

- Place generated icons in `apps/<app>/src/icons/`.
- Reference icons in your Qt `.qrc` resource files for use in the app.

## Template SVG Organization

- Place per-app tray/menu template SVGs in `assets/icons/templates/`.
  - Example: `assets/icons/templates/ekiosk-tray-template.svg`, `assets/icons/templates/watchservice-tray-template.svg`
- This keeps templates separate from general icons and supports future scaling as you add more apps.
- Update your export scripts and resource references to use the new template locations.

## Further Reading

- See `scripts/generate_icons.py` for full automation details.
- For troubleshooting converter issues, see the PATH and dependency notes above.

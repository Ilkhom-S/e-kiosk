#!/usr/bin/env python3
"""
generate_icons.py

Convert SVG templates into platform-specific raster icons, ICO files, and macOS .icns files.

Templates are organized by app in subdirectories under assets/icons/templates/:
- controller/ : WatchServiceController (controller app)
- kiosk/ : EKiosk (main kiosk app)
- card/ : Card-related apps
- updater/ : Updater apps

Icon Types (determined by filename pattern):
- App icons: *-app-icon.svg → PNGs, ICO, ICNS files
- UI icons: menu-*.svg, icon-*.svg, button-*.svg, *ui-*.svg → PNGs only
- Template icons: *template*.svg → macOS template PNGs

Usage examples:
  python3 scripts/generate_icons.py --apps WatchServiceController
  python3 scripts/generate_icons.py --source assets/icons/templates --apps WatchServiceController,EKiosk

For app icons, create controller-app-icon.svg in the app's template directory to generate .icns files for macOS.

Dependencies (preferred):
  pip install cairosvg Pillow

Fallback external tools (if Python packages unavailable):
  inkscape, rsvg-convert, or ImageMagick `convert`
  On macOS: iconutil (for .icns generation)

The script will create/ensure `apps/<app>/src/icons/` and write PNG, ICO, and .icns files.
"""

from __future__ import annotations
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path
import sysconfig

try:
    import cairosvg
except Exception:
    cairosvg = None

try:
    from PIL import Image
except Exception:
    Image = None


def which(cmd: str) -> str | None:
    return shutil.which(cmd)


def user_script_dirs() -> list[Path]:
    """Return common per-user script dirs where pip may install executables."""
    dirs: list[Path] = []
    # typical Linux/Unix user install
    dirs.append(Path.home() / '.local' / 'bin')
    # macOS pip user scripts path for framework python
    pyver = f"{sys.version_info.major}.{sys.version_info.minor}"
    dirs.append(Path.home() / 'Library' / 'Python' / pyver / 'bin')
    # also include scripts folder from sysconfig if available
    try:
        sc = Path(sysconfig.get_path('scripts'))
        dirs.append(sc)
    except Exception:
        pass
    return dirs


def find_inkscape_executable() -> str | None:
    """Try to locate an Inkscape executable across platforms."""
    exe = which('inkscape')
    if exe:
        return exe
    # macOS app bundle
    mac_path = Path('/Applications/Inkscape.app/Contents/MacOS/inkscape')
    if mac_path.exists():
        return str(mac_path)
    # Windows common locations
    prog = os.environ.get('ProgramFiles')
    if prog:
        p = Path(prog) / 'Inkscape' / 'inkscape.exe'
        if p.exists():
            return str(p)
    progx86 = os.environ.get('ProgramFiles(x86)')
    if progx86:
        p2 = Path(progx86) / 'Inkscape' / 'inkscape.exe'
        if p2.exists():
            return str(p2)
    return None


def find_cairosvg_cli() -> str | None:
    """Look for a cairosvg CLI script in PATH or common user script dirs."""
    exe = which('cairosvg')
    if exe:
        return exe
    for d in user_script_dirs():
        p = d / 'cairosvg'
        if p.exists():
            return str(p)
        ppy = p.with_suffix('.exe')
        if ppy.exists():
            return str(ppy)
    return None


def find_converter() -> dict:
    """Return a dict describing the available converter.

    Keys: 'type' in ('cairosvg-module','cairosvg-cli','inkscape','rsvg-convert','convert') and
    'cmd' for CLI path when relevant.
    """
    if cairosvg:
        return {'type': 'cairosvg-module'}
    cs_cli = find_cairosvg_cli()
    if cs_cli:
        return {'type': 'cairosvg-cli', 'cmd': cs_cli}
    # Prioritize rsvg-convert for better SVG transparency handling, then ImageMagick
    if which('rsvg-convert'):
        return {'type': 'rsvg-convert', 'cmd': which('rsvg-convert')}
    if which('convert'):
        return {'type': 'convert', 'cmd': which('convert')}
    inkscape = find_inkscape_executable()
    if inkscape:
        return {'type': 'inkscape', 'cmd': inkscape}
    return {'type': 'none'}


def run_cmd(cmd: list[str]) -> None:
    subprocess.check_call(cmd)


def svg_to_png_cairo(svg_path: Path, out_path: Path, width: int, height: int) -> None:
    with open(svg_path, 'rb') as f:
        svg_bytes = f.read()
    cairosvg.svg2png(bytestring=svg_bytes, write_to=str(out_path), output_width=width, output_height=height)


def svg_to_png_external(svg_path: Path, out_path: Path, width: int, height: int) -> None:
    conv = find_converter()
    t = conv.get('type')
    if t == 'inkscape':
        inkscape_cmd = conv.get('cmd', 'inkscape')
        # Prefer modern inkscape CLI (1.0+) but be tolerant
        try:
            run_cmd([inkscape_cmd, str(svg_path), '--export-type=png', f'--export-filename={out_path}', f'--export-width={width}', f'--export-height={height}'])
            return
        except subprocess.CalledProcessError:
            # fallback to legacy flag
            run_cmd([inkscape_cmd, f'--export-png={out_path}', f'--export-width={width}', f'--export-height={height}', str(svg_path)])
            return
    if t == 'rsvg-convert':
        run_cmd([conv['cmd'], '-w', str(width), '-h', str(height), str(svg_path), '-o', str(out_path)])
        return
    if t == 'convert':
        # Use modern ImageMagick syntax: magick input.svg -background transparent -alpha on -resize WxH output.png
        cmd = [conv['cmd']]
        if conv['cmd'].endswith('/convert') or conv['cmd'] == 'convert':
            # If using legacy convert command, use magick directly
            magick_path = conv['cmd'].replace('convert', 'magick')
            if which(magick_path):
                cmd = [magick_path]
            else:
                cmd = [conv['cmd']]
        run_cmd(cmd + [str(svg_path), '-background', 'transparent', '-alpha', 'on', '-resize', f'{width}x{height}', str(out_path)])
        return
    if t == 'cairosvg-cli':
        # cairosvg input.svg -o output.png -f png -w width -h height
        run_cmd([conv['cmd'], str(svg_path), '-o', str(out_path), '-f', 'png', '-w', str(width), '-h', str(height)])
        return
    raise RuntimeError('No SVG->PNG converter available (cairosvg module, cairosvg CLI, inkscape, rsvg-convert, or convert)')


def render_svg_to_png(svg_path: Path, out_path: Path, width: int, height: int) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    conv = find_converter()
    if conv.get('type') == 'cairosvg-module':
        svg_to_png_cairo(svg_path, out_path, width, height)
        return
    # fallback to external converters
    svg_to_png_external(svg_path, out_path, width, height)


def build_ico_from_pngs(png_paths: list[Path], out_ico: Path) -> None:
    if Image is None:
        # try external convert
        if which('convert'):
            # ImageMagick can produce ico from list of pngs - use modern syntax
            convert_path = which('convert')
            magick_path = convert_path.replace('convert', 'magick') if convert_path else None
            if magick_path and which(magick_path):
                cmd = [magick_path] + [str(p) for p in png_paths] + [str(out_ico)]
            else:
                cmd = ['convert'] + [str(p) for p in png_paths] + [str(out_ico)]
            run_cmd(cmd)
            return
        raise RuntimeError('Pillow not installed and ImageMagick convert not available to build ICO')

    imgs = []
    for p in png_paths:
        imgs.append(Image.open(p).convert('RGBA'))

    # Pillow can save ICO with multiple sizes by passing the sizes parameter
    # Ensure smallest image is first; create sizes tuple
    sizes = [img.size for img in imgs]
    # Use the largest image as base and supply sizes for ICO
    base = imgs[-1]
    base.save(out_ico, format='ICO', sizes=sizes)


def generate_icns(svg_path: Path, out_icns: Path, force: bool = False) -> None:
    """Generate macOS .icns file from SVG using iconutil."""
    if out_icns.exists() and not force:
        print(f'  {out_icns.name} already exists, skipping')
        return

    import platform
    if platform.system() != 'Darwin':
        print(f'  Skipping .icns generation on {platform.system()} (requires macOS)')
        return

    if not which('iconutil'):
        print('  Warning: iconutil not found, cannot generate .icns', file=sys.stderr)
        return

    # Create iconset directory
    iconset_dir = out_icns.parent / f'{out_icns.stem}.iconset'
    iconset_dir.mkdir(parents=True, exist_ok=True)

    # Generate PNGs for iconset
    sizes = [16, 32, 48, 64, 128, 256, 512]
    for s in sizes:
        png_path = iconset_dir / f'icon_{s}x{s}.png'
        render_svg_to_png(svg_path, png_path, s, s)

    # Run iconutil
    run_cmd(['iconutil', '-c', 'icns', str(iconset_dir)])

    # Move the generated .icns to final location
    generated_icns = iconset_dir.parent / f'{iconset_dir.stem}.icns'
    if generated_icns.exists():
        generated_icns.replace(out_icns)

    # Clean up iconset
    shutil.rmtree(iconset_dir, ignore_errors=True)

    print(f'  Created {out_icns.name}')


def generate_for_app(svg_dir: Path, app: str, force: bool = False) -> None:
    app_icons = Path('apps') / app / 'src' / 'icons'
    app_icons.mkdir(parents=True, exist_ok=True)

    # Map app names to their template subdirectories
    app_template_dirs = {
        'WatchServiceController': 'controller',
        'EKiosk': 'kiosk',
        'Updater': 'updater',
        'WatchService': 'watchdog',
        'Card': 'card',  # For payment card related apps
        # Add more mappings as needed
    }

    template_subdir = app_template_dirs.get(app, app.lower())
    app_svg_dir = svg_dir / template_subdir

    # Fallback to main svg_dir if app-specific dir doesn't exist
    if not app_svg_dir.exists():
        app_svg_dir = svg_dir

    # Common patterns: controller.svg (colored), controller_template.svg (single-color for macOS), controller_stopped.svg etc.
    svgs = list(app_svg_dir.glob('*.svg'))
    if not svgs:
        print(f'No SVGs found in {app_svg_dir} (or {svg_dir})', file=sys.stderr)
        return

    print(f'Generating icons for app {app} from {app_svg_dir} into {app_icons}')

    for svg in svgs:
        name = svg.stem  # e.g., controller, controller_template, menu-close

        # Determine icon type based on naming pattern
        if name.endswith('-app-icon'):
            # App icons: generate full set (PNGs, ICO, ICNS)
            icon_type = 'app'
        elif (name.startswith(('menu-', 'icon-', 'button-', 'toolbar-')) or
              'ui-' in name or
              ('monogram' in name and 'Template' in name)):
            # UI icons: generate only PNGs in standard UI sizes
            # Includes: menu-* , icon-* , button-* , toolbar-* , *ui-* , *monogram*Template*
            icon_type = 'ui'
        elif name.endswith('_template') or name.endswith('-template') or 'template' in name:
            # Template icons: generate macOS-style template sizes
            icon_type = 'template'
        else:
            # Default: assume app icon for backward compatibility
            icon_type = 'app'

        if icon_type == 'template':
            # macOS-style template (single-color) — generate status sizes
            base_sizes = [16, 18, 20, 22, 24]
            generated = []
            for s in base_sizes:
                p1 = app_icons / f'{name}-{s}.png'
                p2 = app_icons / f'{name}-{s}@2x.png'
                if force or not p1.exists():
                    render_svg_to_png(svg, p1, s, s)
                    generated.append(p1)
                if force or not p2.exists():
                    render_svg_to_png(svg, p2, s * 2, s * 2)
                    generated.append(p2)
            print(f'  Generated template PNGs: {len(generated)}')
            continue

        elif icon_type == 'ui':
            # UI icons: generate PNGs in standard UI sizes (no ICO/ICNS)
            ui_sizes = [16, 24, 32, 48, 64]  # Common UI icon sizes
            generated = []
            for s in ui_sizes:
                out = app_icons / f'{name}-{s}.png'
                if force or not out.exists():
                    render_svg_to_png(svg, out, s, s)
                    generated.append(out)

            # Create a convenient primary filename for UI icons
            primary = app_icons / f'{name}.png'
            if force or not primary.exists():
                # Use 32px as default for UI icons
                default_size = next((s for s in ui_sizes if s == 32), ui_sizes[0])
                default_png = app_icons / f'{name}-{default_size}.png'
                if default_png.exists():
                    shutil.copy2(default_png, primary)

            print(f'  Generated UI PNGs: {len(generated)}')
            continue

        # App icons: generate full set (PNGs, ICO, ICNS)
        # Baseline sizes for app icons / controller: produce 48, 64, 128, 256, 512
        sizes = [48, 64, 128, 256, 512]
        pngs: list[Path] = []
        for s in sizes:
            out = app_icons / f'{name}-{s}.png'
            if force or not out.exists():
                render_svg_to_png(svg, out, s, s)
            pngs.append(out)

        # Create a convenient primary filenames used by Resources.qrc: e.g., controller.png -> choose 48 as controller.png
        primary = app_icons / f'{name}.png'
        primary_2x = app_icons / f'{name}@2x.png'
        if force or not primary.exists():
            # prefer 48 as default
            shutil.copy2(pngs[0], primary)
        if force or not primary_2x.exists():
            # 2x of 48 is 96; create if not present (we generated 128 as larger; create from 96 if possible)
            # if 96 not generated, create by scaling 128 down
            temp2 = app_icons / f'{name}-96.png'
            try:
                render_svg_to_png(svg, temp2, 96, 96)
            except Exception:
                # fallback: use 128 and resize down
                if Image:
                    Image.open(pngs[2]).resize((96, 96), Image.LANCZOS).save(primary_2x)
            if not primary_2x.exists():
                # if temp2 exists, copy
                if temp2.exists():
                    shutil.copy2(temp2, primary_2x)
        # Build ICO with common Windows sizes: 16, 32, 48, 256
        ico_path = app_icons / f'{name}.ico'
        ico_sizes = [16, 32, 48, 256]
        ico_pngs = []
        for s in ico_sizes:
            p = app_icons / f'{name}-ico-{s}.png'
            if force or not p.exists():
                render_svg_to_png(svg, p, s, s)
            ico_pngs.append(p)

        try:
            build_ico_from_pngs(ico_pngs, ico_path)
            print(f'  Created {ico_path.name}')
        except Exception as e:
            print(f'  Warning: failed to create ICO: {e}', file=sys.stderr)

        # Special case: generate .icns for macOS app icons
        if name in ('controller-app-icon', 'watchdog-app-icon', 'updater-app-icon', 'ekiosk-app-icon'):
            generate_icns(svg, app_icons / f'{name}.icns', force)


def main() -> None:
    parser = argparse.ArgumentParser(description='Generate platform icons from SVG templates')
    parser.add_argument('--source', '--source-dir', dest='source', default='assets/icons/templates', help='Directory with SVG templates')
    parser.add_argument('--apps', dest='apps', default='WatchServiceController', help='Comma-separated list of apps under apps/<name>/src/icons to write')
    parser.add_argument('--force', action='store_true', help='Overwrite existing outputs')

    args = parser.parse_args()

    source_dir = Path(args.source)
    if not source_dir.exists():
        print(f'Source directory {source_dir} does not exist', file=sys.stderr)
        sys.exit(2)

    apps = [a.strip() for a in args.apps.split(',') if a.strip()]
    if not apps:
        print('No apps specified', file=sys.stderr)
        sys.exit(2)

    conv = find_converter()
    ctype = conv.get('type')
    if ctype == 'cairosvg-module':
        print('Using: cairosvg (Python module)', ',', 'Pillow' if Image else 'no-Pillow')
    elif ctype in ('cairosvg-cli', 'inkscape', 'rsvg-convert', 'convert'):
        print(f"Using external converter: {ctype}", conv.get('cmd', ''))
    else:
        print('\nERROR: No SVG converter available. Install cairosvg (pip) or inkscape/rsvg-convert/convert on PATH.', file=sys.stderr)
        print('  pip install cairosvg Pillow')
        # helpful note about user script locations which commonly trigger pip warning
        ux = ', '.join(str(p) for p in user_script_dirs())
        print('\nIf you saw a pip warning like:\n  WARNING: The script cairosvg is installed in \'~/...\' which is not on PATH.', file=sys.stderr)
        print(f'Add one of these to your PATH environment variable: {ux}', file=sys.stderr)
        sys.exit(3)

    for app in apps:
        generate_for_app(source_dir, app, force=args.force)


if __name__ == '__main__':
    main()

# Humo Kiosk Ecosystem: Icon Design Guidelines (2026)

To maintain a premium, unified aesthetic across the **macOS Sequoia**, **Windows 11**, and **Linux** versions of the Humo Kiosk suite, all icons must follow these technical and visual standards.

## 1. Visual Hierarchy

The ecosystem is divided into two tiers to help users and admins distinguish between platform tools and third-party services:

| Tier           | Background                 | Glyph Color     | Usage                                       |
| :------------- | :------------------------- | :-------------- | :------------------------------------------ |
| **Core Suite** | **Brand Orange Gradient**  | White (#FFFFFF) | Kiosk Client, Controller, Watchdog, Updater |
| **Providers**  | **Dark Charcoal Gradient** | White (#FFFFFF) | Banks, Utilities, Mobile Operators          |

---

## 2. Technical Specifications

- **Canvas Size:** 512 x 512 px.
- **Corner Radius:** 112px (Standard Apple Squircle).
- **Namespace (Critical):** All SVGs **MUST** include `xmlns="http://www.w3.org/2000/svg"`. Without this exact URL, the `rsvg-convert` tool will fail with "XML does not have root."
- **Bezel (The "Charm"):** Icons utilize a dual-hairline rim light to simulate glass depth.
  - **Top-Left:** 1.5pt White Stroke (0.8 Opacity).
  - **Bottom-Right:** 1.2pt White Stroke (0.4 Opacity).

---

## 3. Provider Icon Template (SVG)

Third-party providers should use the following SVG structure. Simply replace the `<!-- LOGO HERE -->` section with the provider's vector logo.

```xml
<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512">
    <defs>
        <!-- Neutral Background for Providers -->
        <linearGradient id="bg" x1="0%" y1="0%" x2="100%" y2="100%">
            <stop offset="0%" stop-color="#2D2D2D" />
            <stop offset="100%" stop-color="#1A1A1A" />
        </linearGradient>

        <!-- Standardized Rim Lights -->
        <linearGradient id="topGlow" x1="0%" y1="0%" x2="100%" y2="100%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0.6" />
            <stop offset="40%" stop-color="#FFFFFF" stop-opacity="0" />
        </linearGradient>
        <linearGradient id="bottomGlow" x1="100%" y1="100%" x2="0%" y2="0%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0.3" />
            <stop offset="40%" stop-color="#FFFFFF" stop-opacity="0" />
        </linearGradient>
    </defs>

    <!-- 1. Background Layer -->
    <rect width="512" height="512" rx="112" fill="url(#bg)" />

    <!-- 2. The Bezel (Hairline Highlights) -->
    <rect x="2" y="1" width="508" height="508" rx="111" fill="none" stroke="url(#topGlow)" stroke-width="1.5" />
    <rect x="2" y="2" width="508" height="508" rx="111" fill="none" stroke="url(#bottomGlow)" stroke-width="1.2" />

    <!-- 3. Provider Logo Placement -->
    <g fill="#FFFFFF">
        <!-- LOGO HERE: Ensure logo is centered and stays within 380x380 safe zone -->
        <circle cx="256" cy="256" r="120" />
    </g>
</svg>
```

## 4. Icon Generation and Export

### Source Files

- All vector icon sources are organized by app in `assets/icons/templates/` subdirectories
- Core suite apps use brand orange gradient backgrounds
- Provider apps use dark charcoal gradient backgrounds

### Automated Export: `scripts/generate_icons.py`

Use the provided script to generate all required PNG and ICO files for each app:

```bash
python3 scripts/generate_icons.py --apps Updater
```

### Recommended PNG Exports

- 1x: 16, 22, 24, 32, 48, 64
- 2x (retina): 32, 44, 48, 64, 96, 128
- For macOS template: export monochrome PNGs at 1x and 2x

### Converter Requirements

- **Preferred:** `cairosvg` Python module (`pip install cairosvg Pillow`)
- **Fallbacks:** Inkscape (CLI), `rsvg-convert`, ImageMagick `convert`

## 5. Integration

- Place generated icons in `apps/<app>/src/icons/`
- Reference icons in your Qt `.qrc` resource files for use in the app
- Ensure proper namespace declaration in all SVG sources

## 6. Platform-Specific Notes

- **macOS:** For menu bar (template) images, supply a monochrome PNG (alpha mask)
- **Windows:** ICO files are automatically generated from PNG sources
- **Windows:** ICO files are automatically generated from PNG sources

- **Linux:** PNG files are used directly in desktop environments

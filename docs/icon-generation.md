# 🚀 Humo Kiosk Ecosystem: Icon Design Guidelines (2026 Revision)

This document has been refactored to include the "Liquid Glass" and "Charming" aesthetics finalized in our latest session. These standards apply to macOS Sequoia, Windows 11, and Linux versions of the Humo suite.

## 1. Visual Hierarchy: The "Liquid Glass" Standard

The 2026 aesthetic moves away from flat vectors toward Material Physics. Icons now treat foreground objects as physical, translucent glass layers.

| Tier       | Background                 | Glyph Style   | Finish                                |
| ---------- | -------------------------- | ------------- | ------------------------------------- |
| Core Suite | Vibrant Orange Gradient    | Frosted White | High-Gloss Surface / 85% Translucency |
| Providers  | Midnight Charcoal Gradient | Frosted White | Soft-Matte Surface / 75% Translucency |

## 2. Technical Specifications

- **Canvas Size:** 512 x 512 px
- **Corner Radius:** 112px (Standard Apple Squircle)
- **The "Charming" Bezel:** Dual-hairline rim lights
  - Top-Left: 1.5pt White Stroke (`stop-opacity="0.8"`)
  - Bottom-Right: 1.2pt White Stroke (`stop-opacity="0.4"`)
- **Material Depth:** Use a `linearGradient` for the body color and a separate `linearGradient` for the Specular Shine (The Glint)

## 3. Master App Icon Template (Core Suite)

Use this structure for all Core Suite apps (EKioskApp.svg, ControllerApp.svg, etc.).

```xml
<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512">
    <defs>
        <linearGradient id="bg" x1="0%" y1="0%" x2="100%" y2="100%">
            <stop offset="0%" stop-color="#FF7D40" />
            <stop offset="100%" stop-color="#FA5300" />
        </linearGradient>
        <!-- Frosted Glass Effect -->
        <linearGradient id="frostedGlass" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0.9" />
            <stop offset="100%" stop-color="#FFFFFF" stop-opacity="0.7" />
        </linearGradient>
        <!-- Specular Surface Shine -->
        <linearGradient id="specular" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0.9" />
            <stop offset="45%" stop-color="#FFFFFF" stop-opacity="0" />
        </linearGradient>
        <linearGradient id="topGlow" x1="0%" y1="0%" x2="100%" y2="100%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0.8" />
            <stop offset="40%" stop-color="#FFFFFF" stop-opacity="0" />
        </linearGradient>
        <linearGradient id="bottomGlow" x1="100%" y1="100%" x2="0%" y2="0%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0.4" />
            <stop offset="40%" stop-color="#FFFFFF" stop-opacity="0" />
        </linearGradient>
    </defs>

    <!-- 1. Background & Bezels -->
    <rect width="512" height="512" rx="112" fill="url(#bg)" />
    <rect x="2" y="1" width="508" height="508" rx="111" fill="none" stroke="url(#topGlow)" stroke-width="1.5" />
    <rect x="2" y="2" width="508" height="508" rx="111" fill="none" stroke="url(#bottomGlow)" stroke-width="1.2" />

    <!-- 2. Translucent Foreground (The Glyph) -->
    <g opacity="0.88" fill="url(#frostedGlass)">
        <!-- INSERT GLYPH PATH HERE -->
        <circle cx="256" cy="256" r="120" />
    </g>

    <!-- 3. Top Glint (The Charm) -->
    <path d="M112,0 h288 a112,112 0 0 1 112,112 v100 q-256,50 -512,0 v-100 a112,112 0 0 1 112,-112"
          fill="url(#specular)" opacity="0.5" pointer-events="none" />
</svg>
```

## 4. Provider Icon Template (SVG)

For third-party banks and utilities. Note the Midnight Charcoal background.

```xml
<svg xmlns="http://www.w3.org/2000/svg" width="512" height="512" viewBox="0 0 512 512">
    <defs>
        <linearGradient id="bg" x1="0%" y1="0%" x2="100%" y2="100%">
            <stop offset="0%" stop-color="#2D2D2D" />
            <stop offset="100%" stop-color="#0A0A0B" />
        </linearGradient>
        <linearGradient id="specular" x1="0%" y1="0%" x2="0%" y2="100%">
            <stop offset="0%" stop-color="#FFFFFF" stop-opacity="0.4" />
            <stop offset="50%" stop-color="#FFFFFF" stop-opacity="0" />
        </linearGradient>
    </defs>

    <rect width="512" height="512" rx="112" fill="url(#bg)" />

    <!-- Provider Glyph (75% Translucency) -->
    <g fill="#FFFFFF" opacity="0.75" transform="translate(64,64) scale(0.75)">
        <!-- PROVIDER LOGO HERE -->
    </g>

    <!-- Surface Polish Overlay -->
    <rect width="512" height="256" rx="112" fill="url(#specular)" />
</svg>
```

## 5. Automated Generation: scripts/generate_icons.py

The script now handles Liquid Glass transparency and multi-layer exports.

| Pattern         | Description                                                             |
| --------------- | ----------------------------------------------------------------------- |
| `*App.svg`      | Generates full ICO (Windows) and ICNS (macOS) with transparency support |
| `*Icon.svg`     | Generates UI-ready PNGs (16px - 128px) for buttons                      |
| `*Template.svg` | Generates 100% monochrome black alpha-masks for the macOS menu bar      |

### Execution Command

```bash
# Generate all Humo 2026 suite icons
python3 scripts/generate_icons.py --all
```

## 6. Troubleshooting "White Screen" or "XML Error"

| Issue                    | Solution                                                                                                       |
| ------------------------ | -------------------------------------------------------------------------------------------------------------- |
| White Screen in Qt       | Ensure decimal properties in QML use `property real` instead of `int`                                          |
| "XML does not have root" | Verify the `xmlns="http://www.w3.org/2000/svg"` is present in every SVG                                        |
| Blurry Icons             | For Windows 7, ensure `sourceSize.width` and `sourceSize.height` are explicitly set in the QML Image component |

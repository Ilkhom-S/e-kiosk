# 🚀 Humo Kiosk 2026: Master UI & Admin Directive

This document codifies the **"Liquid Glass"** and **"Charming"** UI/UX standards for the Humo Kiosk 2026 ecosystem. All AI agents and developers must adhere to these specifications to ensure consistency across **Windows 7–11**, **macOS Sequoia**, and **Linux**.

---

## 1. Core Visual Principles (Midnight Rich UI)

All windows, popups, and admin interfaces must utilize the following material standards:

- **Color Palette:**
  - `Midnight Black (#0A0A0B)`: Primary window and popup background.
  - `Deep Charcoal (#1C1C1E)`: Elevated containers, status boxes, and secondary surfaces.
  - `Vibrant Orange (#FA5300)`: Primary brand actions and active focus states.
- **Typography (Outdoor Optimized):**
  - **Standard:** `Segoe UI` (Primary), `Arial` (Fallback).
  - **Weight:** `Black (900)` for headlines/primary alerts; `SemiBold` for body text.
  - **Size:** Monstrous scale for outdoor legibility (e.g., **52px/38pt** for primary messages).
- **The "Charming" Squircle:**
  - **Window/Popup Radius:** `45px`.
  - **Button/Icon Radius:** `25px`.

---

## 2. Window Construction & Real Rounding

To achieve professional rounding (avoiding rectangular "ugly corners") on all OS platforms:

- **C++ Requirement:**
  - Set `setAttribute(Qt::WA_TranslucentBackground)`.
  - Set `setWindowFlags(Qt::FramelessWindowHint)`.
- **Styling Requirement:** Define the background color and `border-radius` within the QSS (`styleSheet`) or QML `Rectangle`, never the OS frame.
- **Shadows:** For macOS Sequoia/Windows 11 depth, apply a `0.5` blur drop shadow with a vertical offset of `10`.

---

## 3. Message Box Tiering (Industrial Standard)

Message level icons must use the **High-Saturate Industrial Palette** to prevent desaturation/wash-out in direct 2000-nit sunlight.

| Level        | Background Gradient   | Icon Glyph | Weight | Logic Mapping         |
| :----------- | :-------------------- | :--------- | :----- | :-------------------- |
| **Info**     | `#007AFF` → `#003366` | "i"        | 72px   | `MessageBox.Info`     |
| **Question** | `#FF6A00` → `#EE3E00` | "?"        | 72pt   | `MessageBox.Question` |
| **Warning**  | `#FFAD00` → `#FF5E00` | "!"        | 72pt   | `MessageBox.Warning`  |
| **Critical** | `#FF3B30` → `#8B0000` | "×"        | 72px   | `MessageBox.Critical` |

---

## 4. Admin Security & Sequence Logic

Access to the **Admin Service Menu** is hidden behind a secret touch sequence on the Splash Screen.

- **Hidden Zones:** 5 oversized hit-boxes (Top-Left, Top-Right, Bottom-Left, Bottom-Right, Center).
- **Master Password:** `1-2-5-4-3` (Numerical sequence of zones).
- **Reset Logic:** A `Timer` must clear the `clickSequence` buffer after **5 seconds** of inactivity.
- **Feedback Toggles:**
  - `showAdminFlash`: Boolean to toggle a subtle orange pulse on touch.
  - `showAdminNumbers`: Boolean to toggle massive (160px) debug numbers for technicians.

---

## 5. Technical Implementation Rules

### Qt Widgets (.ui)

- Use `QGridLayout` with **50px margins** to maintain "Charming" white space.
- Avoid `rgba()` in gradients; use **Hex codes** to ensure compatibility with the Windows 7 GDI+ engine.
- Set `scaledContents: true` for `lbIcon` to ensure SVGs fill the container without gaps.

### Qt Quick (QML)

- Use `Anchors` and `Flow` instead of absolute `x/y` positioning for multilingual text wrapping.
- Set `smooth: true` and `mipmap: true` on all `Image` components for sharp SVG rendering.
- Apply `Text.Outline` with `#40000000` to all primary labels to increase contrast against glare.

---

**Current Project Status:** Branding, Splash Screen, and Message Box UI are 100% Finalized.

**Current Project Status:** Branding, Splash Screen, and Message Box UI are 100% Finalized.
**Next Objective:** Implementation of the **Admin Service Menu Grid** (3x3 pattern) using the **Liquid Glass** button components.

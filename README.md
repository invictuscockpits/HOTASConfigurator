# Important NOTE

Please install the **latest release** of the Invictus SSC Firmware from the [Releases page](https://github.com/invictuscockpits/invitus-ssc-firmware/releases)  before using this software.
If your device was shipped **before 8/1/2025** and the Configurator cannot flash firmware, please contact us at [invictuscockpits.com](https://invictuscockpits.com) with your order number.   

---

# Invictus HOTAS Configurator

**Configuration utility for Invictus Cockpit Systems’ HOTAS products.**

This GUI application is the official tool for managing and updating all Invictus HOTAS devices. It is a **custom fork of [FreeJoy Configurator](https://github.com/FreeJoy-Team/FreeJoyConfiguratorQt)**, rewritten and streamlined to support only Invictus hardware.

---

## Description

The Configurator communicates with Invictus HOTAS devices over **USB HID**, allowing you to:

- Flash new firmware versions to your device.
- Read and write complete device configurations.
- Apply **grip-specific profiles** with correct logical button mapping for supported grips.
- Select simulator presets (DCS, Falcon BMS, MSFS) with automatic remapping.
- Calibrate axes and apply filters, deadzones, and curves.
- Author and lock **force anchors** (developer-only feature).

All persistent device settings are saved in device flash, so they survive restarts, firmware updates, and re-plugs.

---

## Key Features

- Optimized for **Invictus Cockpit Systems hardware**.
- Fully integrated **firmware flasher** (no external tools needed).
- Supports modular **grip profiles** (`/profiles/grips/*.json`).
- Automatic **sim-specific remapping**.
- Developer mode (hidden) for working with protected force anchor pages.
- Multi-language support (English, Russian, Simplified Chinese, German).

---

## Hardware Compatibility

- All Invictus HOTAS controllers (Gen 1–4).  Note: Some devices may requre reflashing bootloader for compatibility.
- Backward-compatible with STM32 “Blue Pill” devices flashed with Invictus firmware.

---

## Installation

1. Download the latest `.msi` or installer package from the [Releases page](https://github.com/invictuscockpits/HOTASConfigurator/releases).
2. Run the installer and launch **Invictus HOTAS Configurator**.
3. Plug in your Invictus HOTAS device — it will be detected automatically.

---

## Usage

- **Read Config** → loads the current settings from your device.  
- **Write Config** → applies changes made in the GUI.    
- **Grip Selection** → choose your grip model; automatically loads the correct JSON profile.  
- **Simulator Selection** → auto-remaps controls for DCS, Falcon BMS, or MSFS.  
- **Firmware Updates** → handled in **Advanced Settings → Flasher** tab.  

---

## Disclaimer

The Invictus HOTAS Configurator is provided as-is and supported only for **official Invictus HOTAS hardware**.  
It is **not intended for use with FreeJoy or generic joystick projects**.

Original FreeJoy Configurator project:  
**https://github.com/FreeJoy-Team/FreeJoyConfiguratorQt**

---

© 2025 Invictus Cockpit Systems. All rights reserved.

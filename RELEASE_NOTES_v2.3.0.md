# Invictus HOTAS Configurator v2.3.0

## Important Note
**Version 2.2.x was never publicly released.** Development continued directly to v2.3.0 due to significant changes required for ADC functionality in Gen 4 boards. All features from v2.2.x are included in this release.

---

## What's New in v2.3.0

### ADC Repair & Diagnostics
- **ADC Repair functionality** - Comprehensive diagnostic and repair tools for analog-to-digital converter issues on Gen 4 boards
- Differential mode ADC support for improved precision

### Enhanced Language Support
- **15 Languages** - Expanded from 4 to 15 languages with modern combo box dropdown:
  - 🇺🇸 English (US)
  - 🇬🇧 English (UK)
  - 🇷🇺 Russian
  - 🇨🇳 Simplified Chinese
  - 🇩🇪 German
  - 🇳🇱 Dutch
  - 🇫🇷 French
  - 🇮🇹 Italian
  - 🇯🇵 Japanese
  - 🇰🇷 Korean
  - 🇵🇱 Polish
  - 🇧🇷 Portuguese (BR)
  - 🇪🇸 Spanish (ES)
  - 🇲🇽 Spanish (LA)
  - 🇹🇷 Turkish

- **Improved UI** - Replaced 4 individual language buttons with single dropdown selector
- **Text auto-sizing** - UI elements automatically adjust for different language text lengths
- **AI-generated translations** - All new languages feature machine-translated strings (please report issues!)

### Features from v2.2.x (Now Included)
- **Gen 4 hardware support** - Full compatibility with latest generation control boards
- **Device identification** - Serial number and model marker implementation
- **Enhanced update checking** - Automatic checks for both GUI and firmware updates

---

## Improvements & Fixes

### Fixed
- ✅ Significant layout and button initialization fixes
- ✅ Logical button background rendering issue
- ✅ Pin mapping corrections for accurate hardware control
- ✅ Unused axes now hidden by default for cleaner interface
- ✅ Improved Read From/Write to Device functions

### Changed
- 🔄 Streamlined codebase - Removed unused features
- 🔄 Enhanced styling and UI improvements
- 🔄 Better code documentation and comments
- 🔄 Updated translation infrastructure

---

## Installation

### New Installation
1. Download `InvictusHOTASConfigurator-v2.3.0-Setup.exe` below
2. Run the installer (no admin rights required)
3. Install latest firmware from [Firmware Releases](https://github.com/invictuscockpits/invictus-ssc-firmware/releases)

### Upgrading from Previous Versions
- The installer will **automatically detect and upgrade** existing installations
- All device configurations and settings will be **preserved**
- No manual uninstall required

---

## Requirements

- **Operating System:** Windows 7 or later (32-bit or 64-bit)
- **Firmware:** Invictus SSC Firmware v2.3.x (included in firmware releases)
- **Hardware:** All Invictus HOTAS devices (Gen 1-4)

⚠️ **Note for early adopters:** If your device was shipped before 8/1/2025 and the Configurator cannot flash firmware, please contact us at [invictuscockpits.com](https://invictuscockpits.com) with your order number.

---

## Known Issues

- Translations are AI-generated and may contain inaccuracies
- Please report translation issues on [GitHub Issues](https://github.com/invictuscockpits/HOTASConfigurator/issues)

---

## Technical Details

- **Version:** 2.3.0
- **Build Type:** 32-bit Release
- **Qt Version:** 5.15.19
- **Compiler:** MSVC 2019
- **Firmware Compatibility:** v2.3.x required

---

## Support

- **Documentation:** [GitHub Repository](https://github.com/invictuscockpits/HOTASConfigurator)
- **Issues:** [Report a Bug](https://github.com/invictuscockpits/HOTASConfigurator/issues)
- **Website:** [invictuscockpits.com](https://invictuscockpits.com)
- **Firmware:** [Firmware Releases](https://github.com/invictuscockpits/invictus-ssc-firmware/releases)

---

© 2025 Invictus Cockpit Systems. All rights reserved.

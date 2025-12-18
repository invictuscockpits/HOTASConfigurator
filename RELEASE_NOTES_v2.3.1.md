# Invictus HOTAS Configurator v2.3.1

## Important Note
**Version 2.2.x was never fully released.** Development continued directly to v2.3.0 due to significant changes required for ADC functionality in Gen 4 boards. All features from v2.2.x are included in v2.3.0, and this v2.3.1 release builds upon that foundation with critical bug fixes and UI improvements.

---

## Critical Bug Fix Release

This patch release fixes two critical bugs discovered in v2.3.0 and introduces quality-of-life improvements for Developer Mode and general UI refinements.

---

## Critical Fixes in v2.3.1

### 🛠️ PGA Corruption Bug [CRITICAL]
- **Fixed:** PGA (Programmable Gain Amplifier) settings were being corrupted when writing configuration to device
- **Cause:** Premature return in factory anchors `pack()` function (developer.cpp:223) caused incomplete buffer
- **Impact:** Corrupted adjacent flash memory where PGA settings are stored, causing axes to pin to maximum values
- **Resolution:** Moved return statement after reserved bytes, ensuring complete buffer transmission

### 🔄 Device Identity UI Refresh
- **Fixed:** Device information fields (serial number, model, manufacture date) were writing successfully but not updating in UI
- **Resolution:** Added signal-slot mechanism to automatically trigger UI refresh after successful write
- **Benefit:** Device info now updates immediately without manual tab switching

---

## New Features

### ⌨️ Developer Mode Keyboard Shortcuts
Streamlined workflow for force calibration when working with forces up to 40 lbf:

- **Enter Key** - Press the currently focused Set button to capture anchor value
- **Period (.) Key** - Navigate to next Set button in sequence
- **Visual Feedback** - Bright green (#00ff00) background highlights focused button
- **Navigation Order:**
  1. Roll Left (100%, 75%, 50%)
  2. Roll Right (100%, 75%, 50%)
  3. Pitch Down (100%, 75%, 50%)
  4. Pitch Up Digital (100%, 75%, 50%)
  5. Pitch Up Analog (100%, 75%, 50%)

**Workflow:** Apply force → Press Enter → Press Period → Repeat
*No need to take your hand off the stick or aim the mouse while pulling with significant force.*

---

## UI Improvements

### Developer Tab
- ✅ "Set Anchor" buttons shortened to "Set" (height: 21px)
- ✅ Device Identity section repositioned with improved top margin
- ✅ "Write Device Identity" shortened to "Write Identity"
- ✅ PGA Reference label removed for cleaner layout
- ✅ ADC PGA Settings repositioned with proper margins

### Main Window
- ✅ Tab names updated:
  - "Button Settings" → "Buttons"
  - "Advanced Settings" → "Settings"
- ✅ Button text: "Write Current Settings to Device" → "Write Device Settings"
- ✅ FLCS Mode and Proportional Force Scaling group boxes resized to prevent radio button clipping
- ✅ Increased spacing between axis widgets (15px)

### Settings Tab
- ✅ All buttons standardized: 180px width × 30px height
- ✅ Label updates:
  - "USB Exchange Period" → "USB Exchange Rate"
  - "Firmware flasher" → "Firmware Flasher"
  - "Serial" → "Serial #:"
  - "Firmware Version" → "Firmware Version:"
- ✅ Spinbox controls enlarged: 70px width × 25px height with 9pt font
- ✅ Device info labels widened to 135px for better text visibility
- ✅ Translation disclaimer repositioned (5px down)

### Axes Settings
- ✅ Overall height increased: 115px → 135px
- ✅ Top margin: 16px, Bottom margin: 11px, Spacing: 15px
- ✅ Fixed filter slider clipping issue in extended settings

### Shift Registers
- ✅ Combo box and spinbox minimum height: 25px

---

## Documentation Updates

All wiki documentation has been updated to reflect UI changes:

- **Developer-Mode.md** - Keyboard shortcut workflow, PGA settings documentation
- **Overview.md** - Updated tab names, expanded Settings section, shift registers info
- **Axes-Settings.md** - FLCS Mode and Proportional Force Scaling documentation
- **Getting-Started.md** - Button name updates
- **Troubleshooting.md** - Updated references
- **Axes-Curves.md** - Button name updates
- **Reflashing-Firmware.md** - Tab reference updates

---

## Also Includes All v2.3.0 Features

If you're upgrading from v2.2.x or earlier, v2.3.1 also includes all features introduced in v2.3.0:

### ADC Support & Diagnostics
- **Differential mode ADC** - Full support for differential mode on Gen 4 ADC (ADS1115)
- **Per-channel PGA selection** - Individual programmable gain amplifier settings for each ADC channel
- **Single-ended/Differential selector** - Choose between single-ended or differential input modes
- **Improved precision** - Enhanced analog signal processing for force-sensing axes

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
- **AI-generated translations** - All new languages feature machine-translated strings

### Hardware & Device Support
- **Gen 4 hardware support** - Full compatibility with latest generation control boards
- **Device identification** - Serial number and model marker implementation
- **Enhanced update checking** - Automatic checks for both GUI and firmware updates

### v2.3.0 Fixes & Improvements
- ✅ Significant layout and button initialization fixes
- ✅ Logical button background rendering issue resolved
- ✅ Pin mapping corrections for accurate hardware control
- ✅ Unused axes now hidden by default for cleaner interface
- ✅ Improved Read From/Write to Device functions
- 🔄 Streamlined codebase - Removed unused features
- 🔄 Enhanced styling and UI improvements throughout
- 🔄 Better code documentation and comments
- 🔄 Updated translation infrastructure

---

## Installation

### New Installation
1. Download `InvictusHOTASConfigurator-v2.3.1-Setup.exe` below
2. Run the installer (no admin rights required)
3. Install latest firmware from [Firmware Releases](https://github.com/invictuscockpits/invictus-ssc-firmware/releases)

### Upgrading from Previous Versions
- **From v2.2.x or v2.3.0:** The installer will automatically detect and upgrade existing installations
- **From versions prior to v2.2:** Manual uninstall of the old version is required before installing v2.3.1
- All device configurations and settings will be **preserved** during upgrade

---

## Requirements

- **Operating System:** Windows 7 or later (32-bit or 64-bit)
- **Firmware:** Invictus SSC Firmware v2.3.x (required for full functionality)
- **Hardware:** All Invictus HOTAS devices (Gen 1-4)

⚠️ **Important Notes:**
- If you experienced PGA corruption issues or axes pinning to maximum in v2.3.0, this update resolves the root cause. You may need to reconfigure PGA settings in Developer Mode after updating.
- **For early adopters:** If your device was shipped before 8/1/2025 and the Configurator cannot flash firmware, please contact us at [invictuscockpits.com](https://invictuscockpits.com) with your order number.

---

## Technical Details

- **Version:** 2.3.1
- **Build Type:** 32-bit Release
- **Qt Version:** 5.15.19
- **Compiler:** MSVC 2019
- **Installer Size:** 14.5 MB
- **Firmware Compatibility:** v2.3.x recommended

### Code Changes
- Signal-slot mechanism for device info UI refresh
- Event filter pattern for keyboard shortcut interception
- Corrected factory anchors buffer packing in developer.cpp

---

## Known Issues

- Translations are AI-generated and may contain inaccuracies
- Please report translation issues on [GitHub Issues](https://github.com/invictuscockpits/HOTASConfigurator/issues)

---

## Support

- **Documentation:** [GitHub Wiki](https://github.com/invictuscockpits/HOTASConfigurator/wiki)
- **Issues:** [Report a Bug](https://github.com/invictuscockpits/HOTASConfigurator/issues)
- **Website:** [invictuscockpits.com](https://invictuscockpits.com)
- **Firmware:** [Firmware Releases](https://github.com/invictuscockpits/invictus-ssc-firmware/releases)

---

© 2025 Invictus Cockpit Systems. All rights reserved.

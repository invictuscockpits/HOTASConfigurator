# Changelog

All notable changes to the Invictus HOTAS Configurator will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.3.3] - 2026-01-13

### Critical Fixes
- **Fixed device identity write clearing PGA settings** - Developer tab write function now preserves PGA and ADC mode settings when writing device identity (model/serial/date)
- **Implemented read-modify-write pattern** - Reads current device settings first, modifies only identity fields, then writes complete structure

### Changed
- **Grip profiles reorganization** - Moved master grip profile JSON files from deploy folder to source tree (`profiles/grips/`) for proper version control
- **Updated firmware version check** - Configurator now expects firmware v2.3.1.2 (0x2312)
- **Updated deployment script** - Modified `deploy_and_package.bat` to copy profiles from new source tree location

### Compatible Firmware
- **Firmware v2.3.1.2** - Fixed shift register timing for Tianhang grips (SHIFTREG_TICK_DELAY increased from 5 to 20)
  - Resolves button bouncing on Tianhang 26-button grips
  - Provides ~278ns delay for reliable CD4021 operation with long cables

---

## [2.3.1] - 2025-12-18

### Critical Fixes
- **Fixed PGA corruption bug** - Resolved premature return in factory anchors `pack()` function that was corrupting PGA settings in adjacent flash memory
- **Fixed device identity UI refresh** - Device information now automatically updates in UI after writing to device

### Added
- **Developer Mode keyboard shortcuts** - Streamlined force calibration workflow:
  - **Enter key** - Press currently focused Set button (capture anchor value)
  - **Period (.) key** - Navigate to next Set button in sequence
  - **Bright green highlighting** - Visual feedback for focused Set button
  - Enables hands-free calibration while applying forces up to 40 lbf
- **Automated deployment script** - `deploy_and_package.bat` for building installers

### UI Improvements
- **Developer tab**:
  - "Set Anchor" buttons shortened to "Set" with optimized sizing
  - Device Identity section repositioned with improved margins
  - "Write Device Identity" shortened to "Write Identity"
  - Removed PGA Reference label for cleaner layout
- **Main window**:
  - Tab names: "Button Settings" → "Buttons", "Advanced Settings" → "Settings"
  - Button text: "Write Current Settings to Device" → "Write Device Settings"
  - FLCS Mode and Proportional Force Scaling group boxes resized to prevent clipping
  - Increased spacing between axis widgets for better readability
- **Settings tab**:
  - All buttons standardized: 180px width × 30px height
  - "USB Exchange Period" → "USB Exchange Rate"
  - Spinbox controls enlarged: 70px width × 25px height with 9pt font
  - Device info labels widened to 135px for better visibility
  - "Firmware flasher" → "Firmware Flasher" (capitalization fix)
  - Translation disclaimer repositioned
- **Axes Settings**:
  - Overall height increased from 115px to 135px
  - Improved margins: top 16px, bottom 11px, spacing 15px between axes
  - Fixed filter slider clipping issue
- **Shift Registers**:
  - Combo box and spinbox minimum height set to 25px

### Documentation
- **Updated wiki** - All documentation files updated to reflect UI changes and new keyboard shortcuts:
  - Developer-Mode.md: Keyboard shortcut workflow, PGA settings documentation
  - Overview.md: Updated tab names and expanded Settings section
  - Axes-Settings.md: FLCS Mode and Force Scaling documentation
  - Getting-Started.md, Troubleshooting.md, Axes-Curves.md, Reflashing-Firmware.md: Button/label updates

### Technical Details
- Signal-slot mechanism for device info UI refresh
- Event filter pattern for keyboard shortcut interception
- Button focus order: Roll Left → Roll Right → Pitch Down → Pitch Up Digital → Pitch Up Analog (100%, 75%, 50% for each)

---

## [2.3.0] - 2025-11-26

### Important Note
**Version 2.2.x was never publicly released.** Development continued directly to v2.3.0 due to significant changes required for ADC functionality in Gen 4 boards. All features from v2.2.x are included in this release.

### Added - v2.3.0 (ADC Repair & Language System)
- **ADC Repair functionality** - Comprehensive diagnostic and repair tools for analog-to-digital converter issues
- **15-Language support** - Expanded from 4 to 15 languages with modern combo box selector:
  - English (US & UK), Russian, Simplified Chinese, German, Dutch, French, Italian, Japanese, Korean, Polish, Portuguese (BR), Spanish (ES & LA), Turkish
- **Text auto-sizing system** - UI elements automatically adjust for different language text lengths
- **New flag images** - 9 additional country flags for language selection (Brazil, France, Italy, Mexico, Netherlands, Portugal, South Korea, Turkey, USA)
- **AI-generated translations** - All new languages feature machine-translated strings with disclaimer

### Added - v2.2.x (Included in this release)
- **Gen 4 hardware support** - Full compatibility with Gen 4 control boards
- **Serial number and model markers** - Device identification system
- **Enhanced firmware update checking** - Automatic checks for both GUI and firmware updates

### Fixed
- Significant layout and button initialization fixes
- Logical button background rendering issue
- Pin mapping corrections for accurate hardware control
- Unused axes now hidden by default for cleaner interface
- Improved Read From/Write to Device functions

### Changed
- **Language selection UI** - Replaced 4 individual language buttons with single dropdown combo box
- Removed unused features and streamlined codebase
- Styling and translation improvements throughout application
- Enhanced code documentation and comments
- Updated .gitignore for cleaner repository

### Technical Details
- Differential mode ADC support for Gen 4 boards
- Firmware version compatibility: v2.3.x required
- Updated translation infrastructure with .qm files for all languages
- Integration of text_fit_helpers across 20+ widget files for dynamic UI sizing

---

## Installation & Upgrade

**New Installation:**
- Download `InvictusHOTASConfigurator-v2.3.3-Setup.exe`
- Run installer (no admin rights required)
- Install latest firmware from [firmware releases](https://github.com/invictuscockpits/invictus-ssc-firmware/releases)

**Upgrading from Previous Versions:**
- The installer will automatically detect and upgrade existing installations
- All device configurations and settings will be preserved
- No manual uninstall required

**Requirements:**
- Windows 7 or later (32-bit or 64-bit)
- Invictus SSC Firmware v2.3.1.2 (recommended for Tianhang 26 grip fix)
- For devices shipped before 8/1/2025: Contact support if firmware flashing fails

---

## Breaking Changes
None. This release is fully backward compatible with previous configurations.

---

## Known Issues
- Translations are AI-generated and may have inaccuracies
- Please report translation issues on [GitHub Issues](https://github.com/invictuscockpits/HOTASConfigurator/issues)

---

## Contributors
- GUI improvements and internationalization system
- ADC repair diagnostic tools
- Gen 4 hardware support
- Translation contributions from the community

---

For older releases and detailed development history, visit:
https://github.com/invictuscockpits/HOTASConfigurator/releases

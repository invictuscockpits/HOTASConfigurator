# Changelog

All notable changes to the Invictus HOTAS Configurator will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
- Download `InvictusHOTASConfigurator-v2.3.0-Setup.exe`
- Run installer (no admin rights required)
- Install latest firmware from [firmware releases](https://github.com/invictuscockpits/invictus-ssc-firmware/releases)

**Upgrading from Previous Versions:**
- The installer will automatically detect and upgrade existing installations
- All device configurations and settings will be preserved
- No manual uninstall required

**Requirements:**
- Windows 7 or later (32-bit or 64-bit)
- Latest Invictus SSC Firmware (v2.3.x)
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

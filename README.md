VeriQC
=========

[![VeriQC Logo](https://github.com/chcnav-gnss/VeriQC/blob/main/src/VeriQC_UI/Resources/Icons/VeriQC-LOGO.ico)](#)

[![Release][release-image]][releases]
[![Downloads][downloads-image]][downloads]
[![License][license-image]][license]

[release-image]: https://img.shields.io/badge/release-0.1.12.11-green.svg?style=flat
[releases]: https://github.com/chcnav-gnss/VeriQC/releases

[downloads-image]: https://img.shields.io/github/downloads/chcnav-gnss/VeriQC/total?label=downloads
[downloads]: https://github.com/chcnav-gnss/VeriQC/releases

[license-image]: https://img.shields.io/badge/license-Apache2.0-green.svg?style=flat
[license]: https://github.com/chcnav-gnss/VeriQC/blob/main/LICENSE

VeriQC is a post‑processing quality inspection and analysis tool designed for GNSS observation data. This first public release provides a complete set of data‑quality evaluation capabilities, enabling users to efficiently assess key GNSS metrics and visualize results through rich analytical charts. VeriQC is intended for surveyors, researchers, system integrators, and anyone who needs reliable GNSS data quality assessment.

## Key Features
- GNSS Data Quality Checks
- Rich Visualization Tools
- Data Processing & Output

## Included in This Release：
- Core VeriQC processing engine
- GNSS data parsing and quality‑inspection modules
- Visualization and plotting components
- Sample datasets and example workflows
- README documentation and basic usage guide
- Initial configuration templates

## From Source
* Prerequisites
VeriQC depends on Qt5(Qt5.12.12). Please ensure you have it installed before proceeding with the compilation.
* for Windows
  - Download and install [Qt 5.12.12](https://download.qt.io/archive/qt/5.12/5.12.12/) using the Qt Online Installer.
  - Ensure you have the latest version of [Visual Studio](https://visualstudio.microsoft.com/) installed.
  - Clone the repository: `git clone https://github.com/chcnav-gnss/VeriQC.git`
  - Open the solution file (`project/windows/VeriQC_UI.sln`) in Visual Studio.
  - Make sure the correct Qt version is selected in your project settings.
  - Build the solution by selecting `Build Solution` from the `Build` menu.
* for Linux
  - Ensure you have the necessary build tools installed
  - Download and install [Qt 5.12.12](https://download.qt.io/archive/qt/5.12/5.12.12/) using the Qt Online Installer.
  ```bash
  sudo apt install build-essential cmake linuxdeployqt 
  git clone https://github.com/chcnav-gnss/VeriQC.git
  cd VeriQC/project/linux
  ./build.sh
  ./deploy.sh
  ```
  

## Feedback & Contribution：
Feedback, bug reports, and contributions are welcome.
Please submit Issues or Pull Requests to help improve VeriQC.

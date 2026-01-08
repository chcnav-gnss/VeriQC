VeriQC
=========

[![VeriQC Logo](https://github.com/chcnav-gnss/VeriQC/blob/main/src/VeriQC_UI/Resources/Icons/VeriQC-LOGO.ico)](#)

[![Release][release-image]][releases]
[![License][license-image]][license]

[release-image]: https://img.shields.io/badge/release-0.1.12.11-green.svg?style=flat
[releases]: https://github.com/chcnav-gnss/VeriQC/releases

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
  
  ## Project structure


    ```mermaid
    flowchart LR
    CHC_VeriQC_Files --> project
    project --> A:[Linux:Linux Startup Engineering]
    project --> B:[windows:windows Startup Engineering]
    CHC_VeriQC_Files --> src
    src --> C[UpgradePlug:Software upgrade app code]
    src --> VeriQC_UI
    VeriQC_UI --> D[Components:GUI component widgets]
    VeriQC_UI --> F[QCustomplot:Open-source C++ plotting library based on Qt]  
    VeriQC_UI --> G[QXlsx:An open-source C++ library built on Qt to read and write Excel files]
    VeriQC_UI --> H[Resources:Resources and docs for app]
    VeriQC_UI --> I[nlohmann:An open-source C++ library called JSON for Modern C++]
    VeriQC_UI --> J[upgrade:check app upgrade]
    src --> VeriQC
    VeriQC --> H:[Common: Provides fundamental operations and methods for all modules,\n including time handling, coordinate transformations, and matrix computations]
    VeriQC --> I:[Interface: Invokes modules such as MsgConvert, QCSource, \n and SPP to perform quality control, RINEX conversion, and RINEX merging]
    VeriQC --> J:[MsgConvert: Implements decoding of RINEX and RTCM files, \n as well as RINEX conversion]
    VeriQC --> K:[PropMsgDecoder: Implements decoding of proprietary messages]
    VeriQC --> L:[QCSource: Implements quality control tasks such as cycle slip detection,\n multipath calculation, and utilization statistics]
    VeriQC --> M:[VeriQCSPP: Implements Single Point Positioning,  providing station coordinates, \n satellite elevation angles, and other information for quality control]
    ```

## Algorithm Framework

*   General process
  
  [![General process](https://github.com/chcnav-gnss/VeriQC/blob/main/whole.png)](#)
*   Multipath
  
  [![Multipath](https://github.com/chcnav-gnss/VeriQC/blob/main/Pseudorange_multipath.png)](#)
*   Cycle slip
  
  [![Cycle slip](https://github.com/chcnav-gnss/VeriQC/blob/main/Cycle_slip_detection.png)](#)

## Feedback & Contribution：
Feedback, bug reports, and contributions are welcome.
Please submit Issues or Pull Requests to help improve VeriQC.

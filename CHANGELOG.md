# CanMV K230 Changelog

## 🚀 CanMV K230 v1.3 Release Notes

We are proud to announce the **v1.3** release of the **CanMV K230** platform! This release brings together all the improvements from `v1.3-rc` and `v1.3-rc2`, featuring enhanced hardware support, critical bug fixes, and new tools to streamline development on the K230 SoC.

---

### 🧠 CanMV Core

* **MicroPython Integration**:

  * Improved hardware abstraction with expanded module support.
  * Fixed HAL transmit data error.
* **Peripheral Fixes**:

  * Fixed `machine.I2C` not raising errors on failed transfers.
  * Corrected `machine.FPIOA` pin function mapping bugs.
* **Display Enhancements**:

  * Improved compatibility and performance with various display interfaces.
* **Stability**:

  * Addressed sensor initialization and memory management issues.

[🔗 View detailed changes](https://github.com/canmv-k230/canmv/compare/canmv-v1.2.2...canmv-v1.3)

---

### 🎥 Media Processing Platform (MPP)

* **Video Encoding**:

  * Added `kd_mapi_venc_request_idr` to allow IDR frame generation.
* **Image Handling**:

  * Enhanced mirror and flip functionality for better image control.

[🔗 View detailed changes](https://github.com/canmv-k230/mpp/compare/canmv-v1.2.2...canmv-v1.3)

---

### ⚙️ RT-Smart OS

* **Driver Improvements**:

  * Fixed I2C transfer timeout.
  * Resolved issues with RTL8189 AP mode.
* **System Improvements**:

  * Enhanced memory management and task scheduling.
* **Peripheral Support**:

  * Extended support for I2C, SPI, and other interfaces.
* **Filesystem**:

  * Improved reliability and performance under edge workloads.

[🔗 View detailed changes](https://github.com/canmv-k230/rtsmart/compare/canmv-v1.2.2...canmv-v1.3)

---

### 🛠️ U-Boot Bootloader

* **New Hardware Support**:

  * Added support for the `k230d_evb` board.
* **Boot Enhancements**:

  * Faster boot process and better boot stability.

[🔗 View detailed changes](https://github.com/canmv-k230/u-boot/compare/canmv-v1.2.2...canmv-v1.3)

---

### 📚 `k230_rtsmart_lib` (New Component)

A new library package for RT-Smart apps:

* Simplifies application development on RT-Smart OS.
* Promotes reusable logic and streamlined integration.

[🔗 Explore the repository](https://github.com/canmv-k230/k230_rtsmart_lib)

---

### 🧪 Tooling

* **Image Generator Update**:

  * Added support for new `kdimg` format for firmware images.
    ([PR #6](https://github.com/kendryte/canmv_k230/pull/6) by [@kendryte747](https://github.com/kendryte747))

---

### 📈 Full Changelog

* [canmv\_k230](https://github.com/kendryte/canmv_k230/compare/canmv-v1.2.2...canmv-v1.3)
* [canmv](https://github.com/canmv-k230/canmv/compare/canmv-v1.2.2...canmv-v1.3)
* [mpp](https://github.com/canmv-k230/mpp/compare/canmv-v1.2.2...canmv-v1.3-rc)
* [rtsmart](https://github.com/canmv-k230/rtsmart/compare/canmv-v1.2.2...canmv-v1.3)
* [u-boot](https://github.com/canmv-k230/u-boot/compare/canmv-v1.2.2...canmv-v1.3-rc)

---

We recommend all users upgrade to **v1.3** to benefit from improved performance, broader hardware compatibility, and a more robust development experience.

For issues, feedback, or contributions, visit our [GitHub organization](https://github.com/kendryte/canmv_k230).

## v1.2.2

Version 1.2.2 introduces minor bug fixes and new features to enhance functionality and performance.

### New Features

#### **RT-Smart**
- Enhanced the `tsensor` driver with mutex support for improved concurrency and stability.

#### **CanMV**
- Added support for a new MIPI panel with a resolution of 368x552.
- Introduced the `machine.chipid` and `machine.temperature` APIs for accessing unique device identifiers and temperature data.
- Implemented `os.urandom` for generating random bytes and `os.statvfs` for retrieving filesystem statistics.

### Bug Fixes

#### **RT-Smart**
- Resolved an issue where the MTP could not monitor the `data` partition effectively.

#### **CanMV**
- Fixed a bug that caused sensor snapshot failures when the sensor channel was bound to display.

## v1.2.1

Version 1.2.1 is a minor bug fix for v1.2

### Bug Fixes

- **CanMV**:
  - Fix sensor and display release vb, now can remove try and catch block
  - Fix machine.I2C

## v1.2

Version 1.2 brings several new features, improvements, and bug fixes to the project. This update focuses on RTOS support, new hardware support, and various enhancements across the CanMV, RT-Smart, MPP, and U-Boot components.

### Project Updates

- **RTOS Only SDK**: Added support for RTOS-only SDK build sample code and AI demo compile support.
- **New Board Support**: Added support for board **ATK-DNK230D**.

### New Features

- **CanMV**:
  - Added **soft I2C support** for software-driven I2C communication.
  - Added **SPI LCD driver** support for SPI-based LCD displays.
  - Integrated **Audio 3A support** for improved audio processing.
  - Expanded **hardware support** with new boards, including **ATK-DNK230D**.
  - Added **MIPI DSI debugger support** for debugging MIPI DSI displays.
  - Introduced new **machine.TOUCH module** for touchscreen functionality.
  - New board type format added to display **board memory size**.

- **RT-Smart**:
  - Added **dynamic memory size detection** support.
  - Integrated support for **4G module (EC200M)**.
  - Added **probe support for touch devices**, including a new driver for **CHCS5XXX**.
  - Introduced **FPIOA driver** for flexible I/O array support.
  - Added **USB host split** support.
  - Improved project structure to allow users to **specify custom app folder**.
  - Added support for **resizing GPT partitions**.

- **MPP**:
  - Added **MIPI DSI debugger support** for debugging MIPI DSI displays.
  - Added support for new **sensor models (bf3238, sc132gs)**.
  - Added support for new **2.4-inch, 480x640 LCD** display.
  - Added **build sample support** for MPP.

- **U-Boot**:
  - Added **dynamic memory size detection**.
  - Integrated support for new **boards** including **ATK-DNK230D**.
  - Enhanced **Kburn OTP support**.

### Bug Fixes

- **CanMV**:
  - Fixed **sensor MCM mode error**.
  - Fixed **LVGL pixel format** handling issue.
  - Resolved **SPI driver** issues.
  - Fixed **UART driver** communication problems.
  - Corrected **machine.PWM duty** cycle error.
  - Fixed **NN image inference error**.

- **RT-Smart**:
  - Fixed issues with **SPI driver**.
  - Corrected **I2C driver** issues.
  - Resolved **UART driver** bugs.
  - Fixed **CherryUSB** functionality.

- **MPP**:
  - Fixed **sensor register configuration** for **GC2093**, **OV5647**, and **IMX335**.
  - Fixed **LCD timing** for 3.5-inch 480x800 ST7701 display.

## v1.1

Version 1.1 is a complete overhaul for the K230 platform, designed to be more user-friendly and development-oriented.

### Project Updates

- **Repo Management**: Subprojects are now managed with `repo`.
- **Dependencies**: Linux dependencies have been removed.
- **Build System**: Introduction of a new compilation system.
- **Board Support**: Added support for new boards, including DonshanPI, LCKFB, and others.

### New Features

- **CanMV**:
  - Added support for WS2812 LEDs via GPIO.
  - Network support: Ethernet and Wi-Fi.
  - New board support: DonshanPI, LCKFB, etc.
  - Added support for a new display panel: ILI9806.
  - Audio module update: Added volume control capabilities.
  
- **RT-Smart**:
  - Automatic partition creation and mounting to `/data`.
  - Project management via Kconfig.
  - Added support for WS2812 LEDs via GPIO.
  - Added support for I2C slave mode.
  - Ethernet-over-USB support with RTL8152.
  - WLAN support: RTL8189 and CYW43xx.
  - Added support for NTP (Network Time Protocol).

- **MPP (Multi-Processor Platform)**:
  - Sensor driver framework updated, now support GC2093, OV5647, and IMX335.
  - Updated screen driver framework.

- **U-Boot**:
  - New `kburn` tool, no longer dependent on DRAM.
  - Added support for new boards: DonshanPI, LCKFB, and more.

### Bug Fixes

- **CanMV**:
  - Fixed IOMUX pins (36, 37).
  - Released UART3 for user access.

- **RT-Smart**:
  - Fixed missing I2C, SPI, and UART device nodes.

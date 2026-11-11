# Musical Business Card (Business-card)

An interactive musical business card project developed in C++ and managed with PlatformIO. This repository contains the embedded source code and configuration required for microcontroller deployment.

---

## Project Overview

The musical business card is an embedded system project designed to run musical functionality on microcontroller hardware. The code structure and logic allow the application to execute on two distinct microcontrollers.

---

## Key Features

- **C++ Development**: Source code written entirely in C++ (100%).
- **PlatformIO Management**: Centralized build configuration and environment management via `platformio.ini`.
- **Multi-Board Compatibility**: Deployment and execution supported across 2 microcontrollers.
- **Modular Architecture**: Clean organization separating main application logic, custom libraries, header files, and unit tests.

---

## Repository Structure

| File / Folder | Description |
| --- | --- |
| `carte_de_visite_musicale/` | Directory containing the main source code for the musical business card |
| `include/` | Header files (`.h` / `.hpp`) |
| `lib/` | Project-specific libraries |
| `test/` | Unit and integration test files |
| `platformio.ini` | PlatformIO configuration file |
| `README.md` | Project documentation |

---

## Build and Flashing

The project utilizes the PlatformIO toolchain.

1. **Build the project**:
   Execute `pio run` to compile the C++ source code.

2. **Upload to microcontroller**:
   Execute `pio run --target upload` to flash the microcontroller.

---

## Author

- **Contributor**: meysoun-gharbi

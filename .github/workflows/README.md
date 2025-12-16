# GitHub Actions CI/CD

This directory contains GitHub Actions workflows for automated building and testing of DB48X/DB50X.

## Workflows

### build.yml

Main build workflow that runs on every push and pull request to main/master/develop branches.

#### Jobs

**Simulator Builds (db48x):**

1. **build-simulator-macos** - Builds the macOS simulator
   - Platform: macOS (latest)
   - Requirements: Qt 6.8.1
   - Target: `make sim`
   - Build ID: Uses GitHub Actions run number
   - Artifacts: `db48x.app`, help files

2. **build-simulator-linux** - Builds the Linux simulator
   - Platform: Ubuntu Linux (latest)
   - Requirements: Qt 6.5.3 LTS, libxcb-cursor0, libgl1-mesa-dev, libxkbcommon-x11-0, libfreetype6-dev, pkg-config
   - Target: `make sim`
   - Build ID: Uses GitHub Actions run number
   - Artifacts: `db48x` binary, help files

3. **build-simulator-windows** - Builds the Windows simulator
   - Platform: Windows (latest)
   - Requirements: Qt 6.7.3 (includes MinGW 9.0), MSYS2 (FreeType, pkg-config)
   - Target: `mingw32-make sim`
   - Build ID: Uses GitHub Actions run number
   - Artifacts: `db48x.exe`, DLLs, help files

**Color DM32 Simulator Builds (db50x):**

4. **build-color-simulator-macos** - Builds the color DM32 simulator for macOS
   - Platform: macOS (latest)
   - Requirements: Qt 6.8.1
   - Target: `make color-dm32-sim`
   - Build ID: Uses GitHub Actions run number
   - Artifacts: `db50x.app`, help files

5. **build-color-simulator-linux** - Builds the color DM32 simulator for Linux
   - Platform: Ubuntu Linux (latest)
   - Requirements: Qt 6.5.3 LTS, libxcb-cursor0, libgl1-mesa-dev, libxkbcommon-x11-0, libfreetype6-dev, pkg-config
   - Target: `make color-dm32-sim`
   - Build ID: Uses GitHub Actions run number
   - Artifacts: `db50x` binary, help files

6. **build-color-simulator-windows** - Builds the color DM32 simulator for Windows
   - Platform: Windows (latest)
   - Requirements: Qt 6.7.3 (includes MinGW 9.0), MSYS2 (FreeType, pkg-config)
   - Target: `mingw32-make color-dm32-sim`
   - Build ID: Uses GitHub Actions run number
   - Artifacts: `db50x.exe`, DLLs, help files

**Other Builds:**

7. **build-wasm** - Builds WebAssembly version
   - Platform: Ubuntu Linux (latest)
   - Requirements: Emscripten SDK (emsdk), libfreetype6-dev, pkg-config
   - Target: `make wasm`
   - Build ID: Uses GitHub Actions run number
   - Artifacts: `db48x.js`, `db48x.wasm`, HTML files, help files

8. **build-dm42-firmware** - Builds DM42 firmware
   - Platform: Ubuntu Linux (latest)
   - Requirements: ARM GCC toolchain (gcc-arm-none-eabi), libfreetype6-dev, pkg-config
   - Target: `make all` (runs twice for CRC verification)
   - Build ID: Uses GitHub Actions run number
   - Artifacts: `db48x.pgm`, `db48x_qspi.bin`, help files

9. **build-dm32-firmware** - Builds DM32 firmware
   - Platform: Ubuntu Linux (latest)
   - Requirements: ARM GCC toolchain (gcc-arm-none-eabi), libfreetype6-dev, pkg-config
   - Target: `make dm32-all` (runs twice for CRC verification)
   - Build ID: Uses GitHub Actions run number
   - Artifacts: `db50x.pg5`, `db50x_qspi.bin`, help files

10. **build-release-package** - Creates release packages
    - Runs only on pushes to main/master
    - Depends on all other build jobs
    - Creates `.tar.gz` archives of firmware builds

## Artifacts

Build artifacts are automatically uploaded and can be downloaded from the Actions tab in GitHub:

- **macOS simulators**: `.app` bundles ready to run
- **Linux simulators**: Native binaries
- **Windows simulators**: `.exe` executables with required DLLs
- **WASM builds**: `.js`, `.wasm`, and HTML files for web deployment
- **Firmware builds**: `.pgm`/`.pg5` and `_qspi.bin` files
- **Help files**: `.md` and `.idx` included with all builds

## Manual Triggering

All workflows can be manually triggered using the "workflow_dispatch" event from the Actions tab in GitHub.

## Local Testing

To test builds locally before pushing:

```bash
# Simulator (requires Qt 6.8.1)
make sim

# Color DM32 simulator (requires Qt 6.8.1)
make color-dm32-sim

# WASM build (requires Emscripten SDK)
make wasm

# DM42 firmware (requires ARM toolchain)
make all

# DM32 firmware (requires ARM toolchain)
make dm32-all
```

## Requirements

### For macOS simulator builds:
- Qt 6.8.1 with qtmultimedia module
- Xcode command line tools

### For Linux simulator builds:
- Qt 6.5.3 LTS with qtmultimedia module
- libxcb-cursor0
- libgl1-mesa-dev
- libxkbcommon-x11-0
- libfreetype6-dev
- pkg-config

### For Windows simulator builds:
- Qt 6.7.3 with qtmultimedia module (MinGW build, includes MinGW 9.0 toolchain)

### For WASM builds:
- Emscripten SDK (emsdk)
- libfreetype6-dev (FreeType font library)
- pkg-config

### For firmware builds:
- gcc-arm-none-eabi
- binutils-arm-none-eabi
- libfreetype6-dev
- pkg-config

## Troubleshooting

### Common Build Issues

**Qt version compatibility:**
- Linux builds use Qt 6.5.3 LTS (6.8.1 not available for `gcc_64`)
- Windows builds use Qt 6.7.3 with bundled MinGW 9.0 (switched from MSVC due to preprocessor directive incompatibilities)
- macOS builds use Qt 6.8.1
- Note: Qt's `win64_mingw` package includes its own MinGW toolchain; no separate MinGW installation needed

**FreeType dependency:**
- All builds that use fonts (WASM, simulators, firmware) require FreeType and pkg-config
- **Linux/Ubuntu**: `sudo apt-get install -y libfreetype6-dev pkg-config`
- **Fedora**: `sudo dnf install -y freetype-devel pkgconfig`
- **Windows**: Install via MSYS2: `pacman -S mingw-w64-x86_64-freetype mingw-w64-x86_64-pkg-config`
- **macOS**: FreeType is usually available via system or Homebrew

**WASM shell issues:**
- The `source` command requires bash, not dash (Ubuntu's default `/bin/sh`)
- Fix: Use `shell: bash` in workflow and `SHELL=/bin/bash` for make

**Firmware CRC re-builds:**
- First build may fail CRC check, requiring a second build
- This is expected behavior
- Fix: Use `make all || make all` to automatically retry

**Build tool compilation:**
- Build tools (forcecrc32, crc32check, decimize) must specify `TARGET=opt`
- Without this, they inherit the firmware target (e.g., `db50x`) and fail to build
- Fixed in Makefile lines 592-596

**Windows MSVC issues:**
- MSVC doesn't support `#warning` preprocessor directive (GCC/Clang extension)
- MSVC doesn't support POSIX signals used in recorder library
- Solution: Use MinGW (GCC for Windows) which provides POSIX compatibility layer

**MinGW missing POSIX features:**
- MinGW lacks `SIGSTKSZ` constant and `strsignal()` function
- Solution: Patched recorder submodule to provide fallbacks when `HAVE_SIGSTKSZ` and `HAVE_STRSIGNAL` are not defined
- The recorder's Makefile CONFIG checks for these features and generates appropriate `HAVE_*` defines
- Fallback: `SIGSTKSZ` defaults to 16384, `strsignal()` returns "Signal N" string

**Build ID:**
- All builds use `.build_id` file in root directory for version tracking
- GitHub Actions creates this automatically using `${{ github.run_number }}` for all build jobs
- The build ID is read by `tools/build_id` script and embedded in firmware builds
- For local builds, create manually: `echo "1" > .build_id` (or it defaults to 0)

## Customization

To modify build triggers, edit the `on:` section in `build.yml`. Current triggers:
- Push to stable or dev branches
- Pull requests targeting stable or dev
- Manual workflow dispatch

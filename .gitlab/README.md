# GitLab CI/CD

This directory contains GitLab CI/CD configuration for automated building and testing of DB48X/DB50X.

## Configuration

The CI/CD pipeline is defined in `.gitlab-ci.yml` at the repository root.

### Pipeline Structure

The pipeline has two stages:
1. **build** - All build jobs run in parallel
2. **package** - Creates release archives (only for stable/dev branches)

### Jobs

**Simulator Builds (db48x):**

1. **build-simulator-macos** - Builds the macOS simulator
   - Platform: macOS (requires macOS runner)
   - Requirements: Qt 6.x installed via Homebrew
   - Target: `make sim`
   - Build ID: Uses GitLab pipeline IID (`$CI_PIPELINE_IID`)
   - Artifacts: `db48x.app`, help files

2. **build-simulator-linux** - Builds the Linux simulator
   - Platform: Fedora (latest) (Docker)
   - Requirements: Qt 6.5.3 LTS (installed via aqtinstall)
   - Target: `make sim`
   - Build ID: Uses GitLab pipeline IID
   - Artifacts: `db48x` binary, help files

3. **build-simulator-windows** - Builds the Windows simulator
   - Platform: Windows (requires Windows runner)
   - Requirements: Qt 6.7.3 with MinGW (installed via aqtinstall), MSYS2 (FreeType, pkg-config)
   - Target: `mingw32-make sim`
   - Build ID: Uses GitLab pipeline IID
   - Artifacts: `db48x.exe`, DLLs, help files

**Color DM32 Simulator Builds (db50x):**

4. **build-color-simulator-macos** - Builds the color DM32 simulator for macOS
   - Platform: macOS (requires macOS runner)
   - Requirements: Qt 6.x installed via Homebrew
   - Target: `make color-dm32-sim`
   - Build ID: Uses GitLab pipeline IID
   - Artifacts: `db50x.app`, help files

5. **build-color-simulator-linux** - Builds the color DM32 simulator for Linux
   - Platform: Fedora (latest) (Docker)
   - Requirements: Qt 6.5.3 LTS (installed via aqtinstall)
   - Target: `make color-dm32-sim`
   - Build ID: Uses GitLab pipeline IID
   - Artifacts: `db50x` binary, help files

6. **build-color-simulator-windows** - Builds the color DM32 simulator for Windows
   - Platform: Windows (requires Windows runner)
   - Requirements: Qt 6.7.3 with MinGW (installed via aqtinstall), MSYS2 (FreeType, pkg-config)
   - Target: `mingw32-make color-dm32-sim`
   - Build ID: Uses GitLab pipeline IID
   - Artifacts: `db50x.exe`, DLLs, help files

**Other Builds:**

7. **build-wasm** - Builds WebAssembly version
   - Platform: Fedora (latest) (Docker)
   - Requirements: Emscripten SDK (emsdk), freetype-devel, pkgconfig
   - Target: `make wasm`
   - Build ID: Uses GitLab pipeline IID
   - Artifacts: `db48x.js`, `db48x.wasm`, HTML files, help files

8. **build-dm42-firmware** - Builds DM42 firmware
   - Platform: Fedora (latest) (Docker)
   - Requirements: ARM GCC toolchain (arm-none-eabi-gcc-cs), freetype-devel, pkgconfig
   - Target: `make all` (runs twice for CRC verification)
   - Build ID: Uses GitLab pipeline IID
   - Artifacts: `db48x.pgm`, `db48x_qspi.bin`, help files

9. **build-dm32-firmware** - Builds DM32 firmware
   - Platform: Fedora (latest) (Docker)
   - Requirements: ARM GCC toolchain (arm-none-eabi-gcc-cs), freetype-devel, pkgconfig
   - Target: `make dm32-all` (runs twice for CRC verification)
   - Build ID: Uses GitLab pipeline IID
   - Artifacts: `db50x.pg5`, `db50x_qspi.bin`, help files

10. **build-release-package** - Creates release packages
    - Platform: Fedora (latest) (Docker)
    - Runs only on pushes to stable/dev branches
    - Depends on firmware and WASM build jobs
    - Creates `.tar.gz` archives of builds

## Artifacts

Build artifacts are automatically uploaded and can be downloaded from the CI/CD pipelines page:

- **macOS simulators**: `.app` bundles ready to run
- **Linux simulators**: Native binaries
- **Windows simulators**: `.exe` executables with required DLLs
- **WASM builds**: `.js`, `.wasm`, and HTML files for web deployment
- **Firmware builds**: `.pgm`/`.pg5` and `_qspi.bin` files
- **Help files**: `.md` and `.idx` included with all builds

Artifacts are kept for:
- **30 days** for regular builds
- **90 days** for release packages

## Runner Requirements

### Docker Runners
Most jobs use Docker and can run on any runner with Docker support. The following jobs require Docker:
- All Linux simulator builds
- WASM build
- All firmware builds
- Release packaging

### macOS Runners
The following jobs require a macOS runner with the `macos` tag:
- `build-simulator-macos`
- `build-color-simulator-macos`

**Setup for macOS runners:**
```bash
# Install Qt via Homebrew
brew install qt@6

# Ensure Qt is in PATH
export PATH="/usr/local/opt/qt@6/bin:$PATH"
```

### Windows Runners
The following jobs require a Windows runner with the `windows` tag:
- `build-simulator-windows`
- `build-color-simulator-windows`

**Setup for Windows runners:**
- Chocolatey package manager installed
- Python 3.x available
- PowerShell execution enabled

## Manual Triggering

Pipelines can be manually triggered from the CI/CD > Pipelines page in GitLab by clicking "Run Pipeline".

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

## Variables

The pipeline uses the following GitLab CI variables:

- `CI_PIPELINE_IID` - Internal pipeline ID, used as build ID
- `CI_COMMIT_SHORT_SHA` - Short commit SHA, used in artifact names
- `GIT_SUBMODULE_STRATEGY: recursive` - Ensures submodules are checked out

## Troubleshooting

### Common Build Issues

**Qt version compatibility:**
- Linux builds use Qt 6.5.3 LTS (6.8.1 not available for `gcc_64`)
- Windows builds use Qt 6.7.3 with MinGW (switched from MSVC due to preprocessor directive incompatibilities)
- macOS builds use Qt 6.x from Homebrew

**FreeType dependency:**
- All builds that use fonts (WASM, simulators, firmware) require FreeType and pkg-config
- **Fedora/Linux**: `sudo dnf install -y freetype-devel pkgconfig`
- **Ubuntu**: `sudo apt-get install -y libfreetype6-dev pkg-config`
- **Windows**: Install via MSYS2: `pacman -S mingw-w64-x86_64-freetype mingw-w64-x86_64-pkg-config`
- **macOS**: FreeType is usually available via system or Homebrew

**WASM shell issues:**
- The `source` command requires bash, not dash (Ubuntu's default `/bin/sh`)
- Fix: Use `SHELL=/bin/bash` for make

**Firmware CRC re-builds:**
- First build may fail CRC check, requiring a second build
- This is expected behavior
- Fix: Use `make all || make all` to automatically retry

**Build tool compilation:**
- Build tools (forcecrc32, crc32check, decimize) must specify `TARGET=opt`
- Without this, they inherit the firmware target (e.g., `db50x`) and fail to build
- Fixed in Makefile

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
- GitLab CI creates this automatically using `$CI_PIPELINE_IID` for all build jobs
- The build ID is read by `tools/build_id` script and embedded in firmware builds
- For local builds, create manually: `echo "1" > .build_id` (or it defaults to 0)

**version.h missing:**
- The `version.h` file is generated by the Makefile from git describe
- Always use `make sim` or `make color-dm32-sim` rather than calling qmake directly
- This ensures all dependencies (including version.h) are generated correctly

### Runner-Specific Issues

**macOS Runner:**
- Ensure Qt is installed via Homebrew: `brew install qt@6`
- Ensure Qt binaries are in PATH: `export PATH="/usr/local/opt/qt@6/bin:$PATH"`

**Windows Runner:**
- Ensure Chocolatey is installed
- Ensure Python is available in PATH
- MinGW tools are installed automatically via aqtinstall

**Docker Runner:**
- Ensure Docker daemon is running and accessible
- Ensure runner has sufficient disk space for images and artifacts
- Qt libraries for Linux require X11 libraries even for headless builds

## Customization

To modify build triggers, edit the `only:` sections in `.gitlab-ci.yml`. Current triggers:
- All pushes and merge requests (build stage)
- Pushes to stable or dev branches only (package stage)

To add or modify variables, edit the `variables:` section at the top of `.gitlab-ci.yml`.

## Comparison with GitHub Actions

GitLab CI/CD provides equivalent functionality to the GitHub Actions workflow:

| Feature | GitHub Actions | GitLab CI |
|---------|---------------|-----------|
| Build ID | `${{ github.run_number }}` | `$CI_PIPELINE_IID` |
| Commit SHA | `${{ github.sha }}` | `$CI_COMMIT_SHA` |
| Short SHA | `${GITHUB_SHA:0:8}` | `$CI_COMMIT_SHORT_SHA` |
| Branch filter | `if: github.ref == 'refs/heads/stable'` | `only: - stable` |
| Artifacts | `actions/upload-artifact@v4` | `artifacts:` section |
| Dependencies | `needs:` | `dependencies:` |

Both pipelines build the same targets and produce identical artifacts.


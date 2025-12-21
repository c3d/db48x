# GitHub Actions Build Badges

Add these badges to your main README.md to show build status:

## Build Status Badge

```markdown
[![Build Status](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml)
```

Replace `YOUR_USERNAME` with your actual GitHub username or organization name.

## Individual Job Badges

You can also create badges for specific jobs:

### Simulator Build
```markdown
[![Simulator](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml/badge.svg?job=build-simulator)](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml)
```

### Color Simulator Build
```markdown
[![Color Simulator](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml/badge.svg?job=build-color-simulator)](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml)
```

### WASM Build
```markdown
[![WASM](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml/badge.svg?job=build-wasm)](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml)
```

### DM42 Firmware Build
```markdown
[![DM42 Firmware](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml/badge.svg?job=build-dm42-firmware)](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml)
```

### DM32 Firmware Build
```markdown
[![DM32 Firmware](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml/badge.svg?job=build-dm32-firmware)](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml)
```

## Example Section for README.md

```markdown
## Build Status

[![Build Status](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml/badge.svg)](https://github.com/YOUR_USERNAME/db48x/actions/workflows/build.yml)

Automated builds are performed for:
- Simulators for macOS, Linux, and Windows (db48x)
- Color DM32 Simulators for macOS, Linux, and Windows (db50x)
- WebAssembly (WASM)
- DM42 Firmware
- DM32 Firmware

Download the latest build artifacts from the [Actions tab](https://github.com/YOUR_USERNAME/db48x/actions).
```


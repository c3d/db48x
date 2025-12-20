# GitLab CI/CD Pipeline Badges

This document provides badge templates for displaying build status in your README or documentation.

## Pipeline Status Badge

Overall pipeline status:

```markdown
[![Pipeline Status](https://gitlab.com/YOUR_USERNAME/db48x/badges/dev/pipeline.svg)](https://gitlab.com/YOUR_USERNAME/db48x/-/pipelines)
```

For stable branch:
```markdown
[![Pipeline Status](https://gitlab.com/YOUR_USERNAME/db48x/badges/stable/pipeline.svg)](https://gitlab.com/YOUR_USERNAME/db48x/-/pipelines)
```

## Coverage Badge (if tests are added)

```markdown
[![Coverage](https://gitlab.com/YOUR_USERNAME/db48x/badges/dev/coverage.svg)](https://gitlab.com/YOUR_USERNAME/db48x/-/pipelines)
```

## Individual Job Badges

GitLab doesn't provide individual job badges by default, but you can use shields.io:

### Simulator Builds

**macOS Simulator:**
```markdown
![macOS Simulator](https://img.shields.io/badge/build-macOS%20simulator-blue)
```

**Linux Simulator:**
```markdown
![Linux Simulator](https://img.shields.io/badge/build-Linux%20simulator-blue)
```

**Windows Simulator:**
```markdown
![Windows Simulator](https://img.shields.io/badge/build-Windows%20simulator-blue)
```

### Color DM32 Simulator Builds

**macOS Color Simulator:**
```markdown
![macOS Color Simulator](https://img.shields.io/badge/build-macOS%20color%20simulator-green)
```

**Linux Color Simulator:**
```markdown
![Linux Color Simulator](https://img.shields.io/badge/build-Linux%20color%20simulator-green)
```

**Windows Color Simulator:**
```markdown
![Windows Color Simulator](https://img.shields.io/badge/build-Windows%20color%20simulator-green)
```

### Other Builds

**WASM:**
```markdown
![WASM Build](https://img.shields.io/badge/build-WebAssembly-orange)
```

**DM42 Firmware:**
```markdown
![DM42 Firmware](https://img.shields.io/badge/build-DM42%20firmware-red)
```

**DM32 Firmware:**
```markdown
![DM32 Firmware](https://img.shields.io/badge/build-DM32%20firmware-red)
```

## Latest Release Badge

```markdown
[![Latest Release](https://gitlab.com/YOUR_USERNAME/db48x/-/badges/release.svg)](https://gitlab.com/YOUR_USERNAME/db48x/-/releases)
```

## Example README Section

Here's how you might include these badges in your README:

```markdown
# DB48X Calculator

[![Pipeline Status](https://gitlab.com/YOUR_USERNAME/db48x/badges/dev/pipeline.svg)](https://gitlab.com/YOUR_USERNAME/db48x/-/pipelines)
[![Latest Release](https://gitlab.com/YOUR_USERNAME/db48x/-/badges/release.svg)](https://gitlab.com/YOUR_USERNAME/db48x/-/releases)

## Build Status

| Platform | Status |
|----------|--------|
| macOS Simulator | ![macOS Simulator](https://img.shields.io/badge/build-passing-brightgreen) |
| Linux Simulator | ![Linux Simulator](https://img.shields.io/badge/build-passing-brightgreen) |
| Windows Simulator | ![Windows Simulator](https://img.shields.io/badge/build-passing-brightgreen) |
| WASM | ![WASM Build](https://img.shields.io/badge/build-passing-brightgreen) |
| DM42 Firmware | ![DM42 Firmware](https://img.shields.io/badge/build-passing-brightgreen) |
| DM32 Firmware | ![DM32 Firmware](https://img.shields.io/badge/build-passing-brightgreen) |
```

## Custom Badges with Shields.io

You can create custom dynamic badges using the GitLab API and shields.io:

### Pipeline Status
```markdown
![Pipeline](https://img.shields.io/gitlab/pipeline/YOUR_USERNAME/db48x/dev)
```

### Last Commit
```markdown
![Last Commit](https://img.shields.io/gitlab/last-commit/YOUR_USERNAME/db48x)
```

### Tag/Release
```markdown
![Tag](https://img.shields.io/gitlab/v/tag/YOUR_USERNAME/db48x)
```

### Stars
```markdown
![Stars](https://img.shields.io/gitlab/stars/YOUR_USERNAME/db48x)
```

### Forks
```markdown
![Forks](https://img.shields.io/gitlab/forks/YOUR_USERNAME/db48x)
```

## Notes

- Replace `YOUR_USERNAME` with your actual GitLab username or group name
- Replace `db48x` with your repository name if different
- Badges update automatically based on pipeline results
- For private repositories, you may need to configure badge access tokens
- GitLab's built-in badges are available at: Settings > CI/CD > General pipelines > Pipeline status


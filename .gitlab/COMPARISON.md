# GitHub Actions vs GitLab CI/CD Comparison

This document compares the CI/CD implementations for DB48X on both platforms.

## Quick Reference

| Feature | GitHub Actions | GitLab CI/CD |
|---------|---------------|--------------|
| **Configuration File** | `.github/workflows/build.yml` | `.gitlab-ci.yml` |
| **Pipeline Structure** | Jobs (parallel by default) | Stages → Jobs |
| **Build ID Variable** | `${{ github.run_number }}` | `$CI_PIPELINE_IID` |
| **Commit SHA** | `${{ github.sha }}` | `$CI_COMMIT_SHA` |
| **Short SHA** | `${GITHUB_SHA:0:8}` | `$CI_COMMIT_SHORT_SHA` |
| **Branch Name** | `${{ github.ref }}` | `$CI_COMMIT_REF_NAME` |
| **Artifact Upload** | `actions/upload-artifact@v4` | `artifacts:` keyword |
| **Job Dependencies** | `needs:` | `dependencies:` + stages |
| **Conditional Execution** | `if:` | `only:` / `except:` / `rules:` |
| **Runner Selection** | `runs-on:` | `tags:` |
| **Docker Support** | Via container image | Native with `image:` |

## Syntax Comparison

### Job Definition

**GitHub Actions:**
```yaml
build-simulator-macos:
  name: Build Simulator (macOS)
  runs-on: macos-latest
  steps:
    - name: Checkout code
      uses: actions/checkout@v4
      with:
        submodules: recursive
    - name: Build
      run: |
        echo "${{ github.run_number }}" > .build_id
        make sim
```

**GitLab CI:**
```yaml
build-simulator-macos:
  stage: build
  tags:
    - macos
  before_script:
    - brew install qt@6
  script:
    - echo "$CI_PIPELINE_IID" > .build_id
    - make sim
```

### Artifacts

**GitHub Actions:**
```yaml
- name: Upload simulator artifact
  uses: actions/upload-artifact@v4
  with:
    name: db48x-simulator-macos
    path: |
      sim/db48x.app
      help/
```

**GitLab CI:**
```yaml
artifacts:
  name: "db48x-simulator-macos-$CI_COMMIT_SHORT_SHA"
  paths:
    - sim/db48x.app
    - help/
  expire_in: 30 days
```

### Conditional Execution

**GitHub Actions:**
```yaml
if: github.event_name == 'push' && (github.ref == 'refs/heads/stable' || github.ref == 'refs/heads/dev')
```

**GitLab CI:**
```yaml
only:
  - stable
  - dev
```

### Dependencies Between Jobs

**GitHub Actions:**
```yaml
build-release-package:
  needs: [
    build-simulator-macos,
    build-simulator-linux,
    build-wasm
  ]
```

**GitLab CI:**
```yaml
build-release-package:
  stage: package  # Runs after 'build' stage
  dependencies:
    - build-simulator-macos
    - build-simulator-linux
    - build-wasm
```

## Platform-Specific Differences

### macOS Builds

**GitHub Actions:**
- Uses `macos-latest` (GitHub-hosted runner with pre-installed tools)
- Qt installed automatically via `jurplel/install-qt-action@v4`
- No manual setup required

**GitLab CI:**
- Requires self-hosted macOS runner with `macos` tag
- Requires manual Qt installation (via Homebrew in `before_script`)
- More setup required but more control

### Linux Builds

**GitHub Actions:**
- Uses `ubuntu-latest` (GitHub-hosted)
- Dependencies installed in separate step
- Qt installed via `jurplel/install-qt-action@v4`

**GitLab CI:**
- Uses Docker image (`ubuntu:22.04`)
- All dependencies installed in `before_script`
- Qt installed via `aqtinstall` Python package
- Can run on any Docker-capable runner

### Windows Builds

**GitHub Actions:**
- Uses `windows-latest` (GitHub-hosted)
- Qt installed via `jurplel/install-qt-action@v4`
- PowerShell/bash available

**GitLab CI:**
- Requires self-hosted Windows runner with `windows` tag
- Qt installed via `aqtinstall` in PowerShell
- More manual setup required

## Runner Requirements

### GitHub Actions
- **Free tier**: 2,000 minutes/month for private repos, unlimited for public repos
- **Hosted runners**: macOS, Linux (Ubuntu), Windows pre-configured
- **No setup required**: Just enable Actions in repository settings

### GitLab CI
- **Free tier**: 400 minutes/month for private repos (SaaS), unlimited for self-hosted
- **Shared runners**: Linux (Docker) on gitlab.com
- **Self-hosted required**: For macOS and Windows builds
- **Setup required**: Install and register GitLab Runner, configure tags

## Advantages

### GitHub Actions Advantages
1. **Zero setup** for standard platforms (macOS, Linux, Windows)
2. **Larger free tier** for public repositories (unlimited minutes)
3. **Integrated marketplace** for actions (e.g., Qt installer)
4. **Better Windows support** out of the box
5. **More pre-installed tools** on hosted runners

### GitLab CI Advantages
1. **Better Docker integration** (native, not via containers)
2. **More flexible caching** and artifacts system
3. **Better visualization** of pipeline stages
4. **Self-hosted runners** don't count against minutes quota
5. **More granular control** with stages, rules, and dependencies
6. **Built-in container registry** for Docker images
7. **Better for private repositories** if self-hosting

## Migration Notes

If migrating from one platform to the other:

1. **Variable substitution**: Update all `${{ }}` (GitHub) to `$VARIABLE` (GitLab)
2. **Action replacements**: Replace GitHub Actions with equivalent commands/scripts
3. **Runner tags**: Map `runs-on:` to appropriate `tags:` in GitLab
4. **Artifacts**: Convert `actions/upload-artifact` to `artifacts:` sections
5. **Checkout**: GitLab checks out code automatically (set `GIT_SUBMODULE_STRATEGY: recursive`)
6. **Conditionals**: Convert `if:` expressions to `only:`/`except:`/`rules:`

## Recommendations

### For Public Open Source Projects
- **GitHub Actions** may be preferred due to:
  - Unlimited free minutes
  - Zero setup required
  - Better community visibility

### For Private/Enterprise Projects
- **GitLab CI** may be preferred due to:
  - Better for self-hosted infrastructure
  - More control over runners and resources
  - Integrated with GitLab's full DevOps platform

### For This Project (DB48X)
- **Both configurations are maintained** to support users on either platform
- Builds are functionally identical on both platforms
- Choose based on where your repository is hosted
- Self-hosted GitLab runners recommended for better performance and unlimited minutes

## Testing Locally

Both configurations support local testing, but with different tools:

### GitHub Actions
```bash
# Use act to test GitHub Actions locally
brew install act
act -j build-simulator-macos
```

### GitLab CI
```bash
# Use gitlab-runner to test GitLab CI locally
brew install gitlab-runner
gitlab-runner exec shell build-simulator-macos
```

## Further Reading

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [GitLab CI/CD Documentation](https://docs.gitlab.com/ee/ci/)
- [Migrating from GitHub Actions to GitLab CI](https://docs.gitlab.com/ee/ci/migration/github_actions.html)


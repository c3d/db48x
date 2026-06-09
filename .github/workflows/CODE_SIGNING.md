# Code Signing Setup

The CI/CD workflows support optional code signing for macOS and Windows builds. If you don't configure code signing, the builds will still work with fallback options:
- **macOS**: Uses ad-hoc signing (works for personal use, not for distribution)
- **Windows**: Unsigned executables (works but may trigger security warnings)

## macOS Code Signing (GitHub Actions)

For proper distribution of macOS apps, you'll need an Apple Developer account and certificates.

### Prerequisites
1. Apple Developer Program membership ($99/year)
2. Developer ID Application certificate
3. App-specific password for notarization

### Required GitHub Secrets

Add these secrets to your repository (Settings → Secrets and variables → Actions):

| Secret Name | Description | How to Get It |
|-------------|-------------|---------------|
| `APPLE_CERT_DATA` | Base64 encoded .p12 certificate | Export your Developer ID certificate from Keychain as .p12, then: `base64 -i certificate.p12 | pbcopy` |
| `APPLE_CERT_PASSWORD` | Password for the .p12 certificate | The password you set when exporting the certificate |
| `APPLE_TEAM_ID` | Your Apple Team ID | Found in Apple Developer account (e.g., FY8GAUX282) |
| `APPLE_NOTARY_USER` | Apple ID email | Your Apple Developer account email |
| `APPLE_NOTARY_PASSWORD` | App-specific password | Generate at appleid.apple.com → Sign-In and Security → App-Specific Passwords |

### How It Works

The workflow uses [lando/code-sign-action](https://github.com/lando/code-sign-action) which:
1. Installs your certificate in the keychain
2. Code signs the .app bundle with hardened runtime
3. Notarizes the app with Apple
4. Staples the notarization ticket

If secrets aren't configured, it falls back to ad-hoc signing (`codesign --sign -`).

## Windows Code Signing (GitHub Actions)

For Windows, you'll need a code signing certificate from a trusted Certificate Authority.

### Required GitHub Secrets

| Secret Name | Description | How to Get It |
|-------------|-------------|---------------|
| `WINDOWS_CERT_DATA` | Base64 encoded .pfx or .p12 certificate | `base64 -i certificate.pfx | pbcopy` (or similar on Windows) |
| `WINDOWS_CERT_PASSWORD` | Password for the certificate | The password protecting your certificate |

### Supported Certificate Providers

The action supports:
- Traditional code signing certificates (DigiCert, Sectigo, etc.)
- DigiCert KeyLocker (cloud-based signing)

If secrets aren't configured, Windows executables remain unsigned.

## Android Code Signing (GitHub Actions)

For publishing to Google Play Store, you'll need a signed Android App Bundle.

### Prerequisites
1. Android keystore file (`.keystore` or `.jks`)
2. Keystore password

### Creating a Keystore

If you don't have a keystore yet:

```bash
keytool -genkey -v -keystore android_release.keystore -alias db48x \
  -keyalg RSA -keysize 2048 -validity 10000
```

**Important**: Keep your keystore and password secure! If you lose them, you cannot update your app on Google Play.

### Required GitHub Secrets

| Secret Name | Description | How to Get It |
|-------------|-------------|---------------|
| `ANDROID_KEYSTORE_DATA` | Base64 encoded keystore file | `base64 -i android_release.keystore \| pbcopy` |
| `ANDROID_KEYSTORE_PASS` | Keystore password | The password you set when creating the keystore |

### How It Works

The workflow:
1. Decodes the base64 keystore and saves it to `~/.local/android_release.keystore`
2. Runs `make android` or `make color-android` with the keystore
3. Produces a signed AAB ready for Google Play Store upload

If secrets aren't configured, it builds an unsigned AAB that can be manually signed later.

### Uploading to Google Play

The resulting `.aab` file can be uploaded directly to Google Play Console:
1. Go to Google Play Console
2. Select your app (or create a new one)
3. Release → Production/Testing → Create new release
4. Upload the `.aab` file from the artifacts

See [../ANDROID_BUILD.md](../ANDROID_BUILD.md) for more details about Android builds and Google Play requirements.

## GitLab CI Code Signing

**Note**: The `lando/code-sign-action` is GitHub Actions-specific and won't work in GitLab CI.

### Current Behavior
- **macOS**: Always uses ad-hoc signing (same as GitHub Actions fallback)
- **Windows**: Unsigned
- **Android**: Supports signing if `ANDROID_KEYSTORE_DATA` variable is set (same as GitHub Actions)

### For Proper Signing in GitLab
You would need to:
1. Create a custom signing script
2. Add secrets as GitLab CI/CD variables (Settings → CI/CD → Variables)
3. Implement similar logic to what `lando/code-sign-action` does

Example structure for GitLab macOS signing:
```yaml
script:
  - make sim
  - |
    if [ -n "$APPLE_CERT_DATA" ]; then
      # Decode certificate
      echo "$APPLE_CERT_DATA" | base64 -d > cert.p12
      # Import to keychain
      security create-keychain -p actions temp.keychain
      security import cert.p12 -k temp.keychain -P "$APPLE_CERT_PASSWORD" -T /usr/bin/codesign
      # Sign
      codesign --keychain temp.keychain --sign "$APPLE_TEAM_ID" --options runtime db48x.app
      # Notarize (requires xcrun altool or notarytool)
      # ... notarization steps ...
    else
      codesign --force --deep --sign - db48x.app
    fi
```

## Testing Without Signing

The workflows are designed to work without any secrets configured:
- Fork the repository
- Enable GitHub Actions
- Builds will complete with:
  - **macOS**: Ad-hoc signing (functional for personal use)
  - **Windows**: Unsigned executables (functional with security warnings)
  - **Android**: Unsigned AAB (can be signed manually or used for testing)
- Artifacts are still functional for testing

## Troubleshooting

### macOS: "App is damaged and can't be opened"
- **With ad-hoc signing**: Right-click → Open (first time only), or System Preferences → Security & Privacy → "Open Anyway"
- **With proper signing**: Should open without warnings after notarization

### Windows: "Windows protected your PC"
- **Without signing**: Click "More info" → "Run anyway"
- **With signing**: Should run without warnings if certificate is trusted

### Android: Cannot install or upload to Play Store
- **Without signing**: AAB must be manually signed before uploading to Google Play
- **With signing**: Should upload directly to Google Play Console

### Signing Verification

**macOS**:
```bash
# Check if properly signed
codesign -dv --verbose=4 db48x.app

# Check notarization
spctl -a -vv db48x.app
```

**Windows**:
```powershell
# Check signature
Get-AuthenticodeSignature sim\release\db48x.exe
```

**Android**:
```bash
# Check if AAB is signed
jarsigner -verify -verbose android/db48x.aab

# List signature details
jarsigner -verify -verbose -certs android/db48x.aab
```

## References

- [lando/code-sign-action](https://github.com/lando/code-sign-action)
- [Apple Code Signing Guide](https://developer.apple.com/support/code-signing/)
- [Apple Notarization Guide](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution)
- [Microsoft Authenticode](https://docs.microsoft.com/en-us/windows-hardware/drivers/install/authenticode)
- [Android App Signing](https://developer.android.com/studio/publish/app-signing)
- [Google Play Console](https://play.google.com/console)
- [../ANDROID_BUILD.md](../ANDROID_BUILD.md) - DB48X Android build documentation

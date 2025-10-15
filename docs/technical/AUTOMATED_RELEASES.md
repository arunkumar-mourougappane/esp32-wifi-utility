# Automated Release System

This document explains the automated release system for the ESP32 WiFi Utility Suite, which automatically creates GitHub releases when version changes are detected in `platformio.ini`.

## 🎯 Overview

The automated release system monitors the `platformio.ini` file for version changes and automatically creates GitHub releases with compiled firmware binaries for both supported boards.

### Key Features

- ✅ **Automatic Detection**: Triggers on version changes in `platformio.ini`
- ✅ **Dual-Board Support**: Builds firmware for ESP32dev and Feather ESP32-S3 TFT
- ✅ **Comprehensive Release Notes**: Auto-generated with changelog and features
- ✅ **Semantic Versioning**: Validates version format (e.g., 2.0.0, 2.1.0-beta)
- ✅ **Duplicate Prevention**: Prevents creating releases for existing versions
- ✅ **Professional Presentation**: Enterprise-grade release documentation

## 🔄 How It Works

### 1. Version Detection

The system monitors changes to `platformio.ini` on the `main` branch:

```yaml
on:
  push:
    branches: [ main ]
    paths: 
      - 'platformio.ini'
```

### 2. Version Validation

When a change is detected, the system:
- Extracts the current version from `-DVERSION="x.x.x"`
- Compares with the previous commit's version
- Validates semantic versioning format
- Proceeds only if version actually changed

### 3. Automated Build & Release

If version changed:
- Builds firmware for both ESP32dev and Feather ESP32-S3 TFT
- Generates comprehensive release notes
- Creates GitHub release with firmware attachments
- Tags the release with `v{version}`

## 🛠 Usage

### Method 1: Using the Version Manager Script (Recommended)

```bash
# Show current version
./scripts/version-manager.sh show

# Bump to specific version
./scripts/version-manager.sh bump 2.1.0

# Auto-increment versions
./scripts/version-manager.sh patch    # 2.0.0 -> 2.0.1
./scripts/version-manager.sh minor    # 2.0.0 -> 2.1.0  
./scripts/version-manager.sh major    # 2.0.0 -> 3.0.0

# Validate current version
./scripts/version-manager.sh check
```

### Method 2: Manual Version Update

1. **Edit `platformio.ini`**:
   ```ini
   build_flags = 
       -DVERSION="2.1.0"  # Update this version
   ```

2. **Commit and Push**:
   ```bash
   git add platformio.ini
   git commit -m "chore: bump version to 2.1.0"
   git push origin main
   ```

3. **Automatic Release**: The GitHub Action will detect the version change and create a release

## 📋 Version Format Requirements

### Valid Formats
- `2.0.0` - Standard semantic version
- `2.1.0-beta` - Pre-release version
- `3.0.0-rc1` - Release candidate

### Invalid Formats
- `2.0` - Missing patch version
- `v2.0.0` - Prefix not allowed in platformio.ini
- `2.0.0.1` - Four-part versions not supported

## 🎯 Workflow Triggers

### Automatic Triggers
- ✅ **Push to main** with `platformio.ini` changes
- ✅ **Merged PR** that modifies `platformio.ini`

### What Gets Built
- **ESP32dev**: Standard ESP32 development board firmware
- **Feather ESP32-S3 TFT**: Enhanced firmware with NeoPixel support

### Release Assets
Each release includes:
- `esp32-wifi-utility-esp32dev-v{version}.bin` - Production firmware
- `esp32-wifi-utility-esp32dev-debug-v{version}.elf` - Debug symbols
- `esp32-wifi-utility-feather-s3-tft-v{version}.bin` - Feather firmware
- `esp32-wifi-utility-feather-s3-tft-debug-v{version}.elf` - Feather debug

## 📝 Release Notes Structure

Auto-generated release notes include:

### Header Section
- Version information and upgrade path
- Build metadata (commit, date, type)

### Features Section  
- Core functionality overview
- Channel analysis capabilities
- Technical specifications for both boards

### Installation Instructions
- Download links for appropriate firmware
- Flashing instructions
- Quick start commands

### Documentation Links
- User guides and technical documentation
- Complete command reference

## 🔍 Monitoring & Debugging

### Check Workflow Status
1. Go to **Actions** tab in GitHub repository
2. Look for "Automated Release on Version Bump" workflow
3. Check individual job logs for details

### Common Issues

#### Version Not Detected
- Ensure version is in correct format: `-DVERSION="x.x.x"`
- Check that change is actually committed to main branch
- Verify version differs from previous commit

#### Build Failures
- Check PlatformIO environment configuration
- Verify all dependencies are available
- Review build logs in GitHub Actions

#### Release Already Exists
- System prevents duplicate releases for same version
- Use different version number or delete existing release

## 🚀 Best Practices

### Version Bumping Strategy
- **Patch** (x.x.X): Bug fixes, documentation updates
- **Minor** (x.X.x): New features, enhancements
- **Major** (X.x.x): Breaking changes, architecture updates

### Development Workflow
1. **Development**: Work on feature branches
2. **Testing**: Ensure all tests pass
3. **Version Bump**: Update version before merging to main
4. **Release**: Automatic release on merge

### Quality Assurance
- Always test firmware before version bump
- Run Unity tests to ensure stability
- Verify both board variants work correctly
- Review generated release notes for accuracy

## 🔧 Configuration

### Workflow File Location
`.github/workflows/automated-release.yml`

### Customization Options
- Modify release note template
- Add additional build environments
- Change trigger conditions
- Customize asset naming

## 📊 Benefits

### For Developers
- ✅ **Zero Manual Work**: No manual release creation
- ✅ **Consistent Process**: Same steps every time
- ✅ **Quality Control**: Automated validation and building
- ✅ **Time Savings**: Focus on development, not releases

### For Users
- ✅ **Faster Releases**: Immediate availability of new versions
- ✅ **Comprehensive Assets**: Both firmware variants included
- ✅ **Professional Presentation**: Consistent, detailed release notes
- ✅ **Easy Discovery**: Clear version tracking and changelogs

---

The automated release system transforms version management from a manual, error-prone process into a seamless, professional workflow that ensures users always have access to the latest stable firmware builds.
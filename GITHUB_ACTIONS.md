# GitHub Actions CI/CD Pipeline

This project uses GitHub Actions for continuous integration and deployment. The pipeline automatically builds, tests, and releases the ESP32 WiFi Utility.

## Workflows

### 🔨 Build and Test (`build.yml`)

**Triggers:**
- Push to `main` or `develop` branches
- Pull requests to `main` branch

**Jobs:**

#### 1. Build Matrix
- **Environments**: `esp32dev`, `test`
- **Actions**: 
  - Compile firmware for ESP32
  - Run static code analysis
  - Generate build reports
  - Upload firmware artifacts

#### 2. Documentation Check
- Lint markdown files with markdownlint
- Verify documentation completeness
- Check for broken links and formatting

#### 3. Size Analysis (PR only)
- Compare memory usage between base and PR branches
- Comment on PR with size analysis
- Alert on significant memory usage changes

#### 4. Security Scan
- Run Trivy vulnerability scanner
- Upload results to GitHub Security tab
- Identify potential security issues

#### 5. Release (main branch only)
- Build release firmware
- Create GitHub release with changelog
- Upload firmware binary as release asset

### 📊 Project Status (`status.yml`)

**Triggers:**
- Push to `main` branch
- Weekly schedule (Sunday midnight UTC)

**Features:**
- Generate project statistics
- Update build status badges
- Check dependency status
- Create project health reports

## Build Artifacts

The pipeline generates the following artifacts:

### 📦 Firmware Files
- `firmware.bin` - Ready-to-flash ESP32 binary
- `firmware.elf` - Debug symbols for development

### 📋 Reports
- Build size analysis
- Static code analysis results
- Security scan reports
- Documentation quality checks

## Status Badges

The following badges are available for the repository:

```markdown
![Build Status](https://github.com/arunkumar-mourougappane/esp32-wifi-utility/actions/workflows/build.yml/badge.svg)
![Platform](https://img.shields.io/badge/platform-ESP32-blue.svg)
![Framework](https://img.shields.io/badge/framework-Arduino-green.svg)
![PlatformIO](https://img.shields.io/badge/build-PlatformIO-orange.svg)
```

## Release Process

### Automatic Releases

1. **Trigger**: Push to `main` branch
2. **Version**: Extracted from `platformio.ini` (`VERSION` build flag)
3. **Tag Format**: `v{version}-{build_number}`
4. **Assets**: Compiled firmware binary

### Manual Releases

1. Update version in `platformio.ini`
2. Update changelog in commit message
3. Push to `main` branch
4. GitHub Action will create release automatically

## Build Environment

### Requirements
- **OS**: Ubuntu Latest
- **Python**: 3.9
- **PlatformIO**: Latest stable
- **Node.js**: 18 (for documentation tools)

### Caching
- PlatformIO packages and libraries
- Python pip cache
- Node.js dependencies

### Build Matrix
- **esp32dev**: Production firmware build
- **test**: Unit test compilation and execution

## Security

### Code Scanning
- **Trivy**: Vulnerability scanning for dependencies
- **CodeQL**: Static analysis for security issues
- **SARIF**: Security findings uploaded to GitHub Security tab

### Secrets Management
- `GITHUB_TOKEN`: Automatically provided for releases
- No additional secrets required for basic functionality

## Monitoring

### Build Notifications
- ✅ Successful builds: No notification
- ❌ Failed builds: Email notification to maintainers
- 📊 PR comments: Size analysis and build status

### Health Checks
- Weekly dependency updates check
- Project statistics updates
- Documentation freshness verification

## Configuration Files

### `.markdownlint.json`
Configuration for markdown linting with relaxed rules for technical documentation.

### `platformio.ini`
Project configuration with version information for automated releases.

## Usage for Contributors

### Before Submitting PRs
1. Ensure local build passes: `pio run`
2. Run tests if available: `pio test`
3. Check documentation updates
4. Verify no new security issues

### After PR Merge
- Automatic build triggers on `main`
- Release created if version changed
- Documentation updated automatically
- Security scan runs and reports findings

## Troubleshooting

### Common Issues

**Build Failures:**
- Check PlatformIO version compatibility
- Verify ESP32 platform dependencies
- Review build logs in GitHub Actions

**Test Failures:**
- Ensure hardware-in-loop tests have proper configuration
- Check test environment setup
- Review test output in Actions logs

**Release Issues:**
- Verify version format in `platformio.ini`
- Check GitHub token permissions
- Review release workflow logs

### Support

For CI/CD pipeline issues:
1. Check GitHub Actions logs
2. Verify workflow file syntax
3. Review platform requirements
4. Open issue with full error details
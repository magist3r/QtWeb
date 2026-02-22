# Qt5 Static Build POC Spec

## Goal
Build a reproducible static Qt5 toolchain for QtWeb migration using:
- Qt `5.5.1`
- QtWebKit `5.5.1`
- Linux `x86_64`

This POC validates the toolchain/build pipeline only. It does not migrate or build the QtWeb application yet.

## Scope
- In scope:
  - Containerized build environment with explicit image tag and dependency set for repeatability.
  - Source download with mandatory checksum validation.
  - Static build/install of Qt and QtWebKit.
  - Verification gates and generated build artifacts.
- Out of scope:
  - App source migration changes.
  - Windows/macOS build support.
  - Changes to legacy Qt4 build flow (`build.sh`).

## Exit Criteria
POC is considered successful when all are true:
1. Static Qt build and install complete.
2. QtWebKit modules are built and installed statically.
3. Verification checks pass:
   - `qmake -query QT_VERSION` is `5.5.1`.
   - `qmake -query QT_CONFIG` includes `static`.
   - Required static libraries exist:
     - `libQt5Core.a`
     - `libQt5Gui.a`
     - `libQt5Widgets.a`
     - `libQt5Network.a`
     - `libQt5Xml.a`
     - `libQt5PrintSupport.a`
     - `libQt5WebKit.a`
     - `libQt5WebKitWidgets.a`

## Toolchain Strategy
- Build runs in a containerized toolchain (`toolchains/qt5-static/Dockerfile`).
- Host wrapper script: `build-qt5-static.sh`.
- Runtime selection: `podman`, `docker`, or auto-detect.
- All generated outputs are repository-local (default: `artifacts/qt5-static-5.5.1`).
- Reproducibility note: base image is currently tag-pinned (`ubuntu:18.04`), not digest-pinned.

## Sources and Checksum Policy
- Source definitions are stored in `toolchains/qt5-static/sources.lock`.
- Every source must define at least one checksum.
- Verification priority:
  1. `sha256` if provided.
  2. Fallback to `md5` only when `sha256` is unavailable for legacy archive metadata.
- Build aborts immediately on checksum mismatch.
- URLs and checksums can be overridden using env vars:
  - `QT5_SRC_URL`
  - `QT5_SRC_SHA256`
  - `QT5_WEBKIT_SRC_URL`
  - `QT5_WEBKIT_SHA256`

## Artifact Layout
Default output root: `artifacts/qt5-static-5.5.1`

- `src-cache/`
  - downloaded source archives
- `build/`
  - extracted/build tree
- `install/`
  - static Qt install prefix
- `logs/`
  - `configure.log`
  - `build.log`
  - `install.log`
  - `verify.log`
- `build-manifest.txt`
  - runtime, versions, resolved URLs, checksums, and verification results

## Risks and Fallback
- Risk: Qt `5.5.1` + legacy QtWebKit may not compile cleanly with modern host toolchains.
  - Mitigation: build inside a versioned container image with explicit dependencies.
- Risk: archive mirrors can move or become unavailable.
  - Mitigation: lock file format supports URL overrides.
- Risk: static QtWebKit may be disabled by configure constraints.
  - Mitigation: optional patch hook in `toolchains/qt5-static/patches/`; build fails if WebKit static gate is not met.

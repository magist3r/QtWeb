# Qt5 Static Build POC

## Objective
Build a reproducible static Qt5 toolchain for QtWeb migration on Linux `x86_64`.

Target versions:
- Qt `5.5.1`
- QtWebKit `5.5.1`

This POC validates the toolchain pipeline only (not the QtWeb app build).
Primary portability target: static linking plus minimal runtime dependencies.

## Non-Goals
- Migrating QtWeb application sources.
- Windows/macOS support.
- Modifying legacy Qt4 flow in `build.sh`.

## Fixed Baseline
- Qt version is locked to `5.5.1`.
- Containerized build is required (`podman` or `docker`).
- ICU support is required for Qt5 + QtWebKit in this path.
- Outputs stay inside the repository (default: `artifacts/qt5-static-5.5.1`).

## Inputs
- Wrapper script: `build-qt5-static.sh`
- In-container script: `toolchains/qt5-static/build-inside-container.sh`
- Container definition: `toolchains/qt5-static/Dockerfile`
- Source lock and checksums: `toolchains/qt5-static/sources.lock`
- Optional patch hook: `toolchains/qt5-static/patches/*.patch`

Supported source override env vars:
- `QT5_SRC_URL`
- `QT5_SRC_SHA256`
- `QT5_WEBKIT_SRC_URL`
- `QT5_WEBKIT_SHA256`
- `ICU_SRC_URL`
- `ICU_SRC_SHA256`

Checksum policy:
1. Prefer `sha256`.
2. Allow `md5` fallback only when `sha256` is unavailable in legacy metadata.
3. Abort immediately on mismatch.

## Output Layout
Default root: `artifacts/qt5-static-5.5.1`

- `src-cache/`: downloaded archives
- `build/`: extracted/build tree
- `install/`: static Qt install prefix
- `icu-static/`: static ICU install prefix
- `logs/`: `icu-configure.log`, `icu-build.log`, `icu-install.log`, `configure.log`, `build.log`, `install.log`, `verify.log`
- `build-manifest.txt`: runtime, source URLs/checksums, configure flags, verification summary

## Verification Gates
A successful run must satisfy:
1. `qmake -query QT_VERSION` is `5.5.1`.
2. Install is static (`QT_CONFIG` contains `static` or static libs prove it).
3. Required static libs exist in `install/lib`:
   - `libQt5Core.a`
   - `libQt5Gui.a`
   - `libQt5Widgets.a`
   - `libQt5Network.a`
   - `libQt5Xml.a`
   - `libQt5PrintSupport.a`
   - `libQt5WebKit.a`
   - `libQt5WebKitWidgets.a`
4. Verification log ends with `verification passed`.
5. Required static ICU libs exist in `icu-static/lib`:
   - `libicuuc.a`
   - `libicui18n.a`
   - `libicudata.a`

## Current Status vs Planned Gates
Implemented now:
- Containerized build pipeline.
- Source lock + checksum verification.
- In-container static ICU build from locked source archive.
- Static Qt + QtWebKit library verification.
- Static ICU library verification.
- Manifest/log generation.

Planned (not yet enforced by scripts):
- QtWebKit smoke test binary compile/link gate against produced toolchain.
- Digest-pinned container base image (currently tag-pinned `ubuntu:16.04`).

## Run Examples
Default run:
```bash
./build-qt5-static.sh
```

Clean rebuild with explicit runtime and jobs:
```bash
./build-qt5-static.sh --clean --runtime podman --jobs 8
```

Custom output directory inside repo:
```bash
./build-qt5-static.sh --output-dir artifacts/qt5-static-poc-run1
```

## Risks
- Legacy Qt/QtWebKit code may fail under newer host toolchains.
- Static WebKit can still be rejected by configure constraints.
- Archive URLs may become unavailable over time.
- Over-aggressive dependency reduction can break TLS/cert/platform behavior.

## Next POC Tasks
1. Add and enforce the QtWebKit smoke-test build gate.
2. Add SSL/TLS support in the Qt5 static build and validate it with an HTTPS smoke test.
3. Move Docker base image from tag pinning to digest pinning.
4. Define explicit dependency-audit output for produced artifacts (for example `ldd` policy and exceptions).

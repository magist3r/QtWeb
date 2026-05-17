# Qt5 Static Build POC

## Objective
Build a reproducible static Qt5 toolchain for QtWeb migration on Linux `x86_64`.

Target versions:
- Qt `5.15.17`
- QtWebKit `5.212.0-alpha4`

This POC validates the toolchain pipeline and the QtWebKit smoke-build gate.
After rebuilding the browser, use `./run-browser.sh --check about:blank` from
the host to runtime-check the Docker-built browser.
Primary portability target: static linking plus minimal runtime dependencies.

## Non-Goals
- Running the QtWeb browser UI or other runtime smoke tests on the agent.
- Windows/macOS support.
- Modifying legacy Qt4 flow in `build.sh`.
- Migrating the browser engine to Qt WebEngine.

## Fixed Baseline
- Qt version is locked to `5.15.17`.
- QtWebKit version is locked to `5.212.0-alpha4`.
- Containerized build is required (`podman` or `docker`).
- ICU support is required for Qt5 + QtWebKit in this path.
- Outputs stay inside the repository (default: `artifacts/qt5-static-5.15.17`).

## Inputs
- Wrapper script: `build-qt5-static.sh`
- In-container script: `toolchains/qt5-static/qt5-static-build-entrypoint.sh`
- Container definition: `toolchains/qt5-static/Dockerfile`
- Source lock and checksums: `toolchains/qt5-static/sources.lock`
- Optional patch hook: `toolchains/qt5-static/patches/*.patch`

Supported source override env vars:
- `QT5_SRC_URL`
- `QT5_SRC_SHA256`
- `QT5_WEBKIT_SRC_URL`
- `QT5_WEBKIT_SHA256`

Checksum policy:
1. Prefer `sha256`.
2. Allow `md5` fallback only when `sha256` is unavailable in locked metadata.
3. Abort immediately on mismatch.

## Output Layout
Default build roots: `artifacts/qt5-static-5.15.17-release` and `artifacts/qt5-static-5.15.17-debug`

- `artifacts/src-cache/`: shared downloaded archives for all build flavors
- `build/`: extracted/build tree
- `install/`: static Qt install prefix
- `logs/`: `configure.log`, `build.log`, `install.log`, `qtwebkit-configure.log`, `qtwebkit-build.log`, `qtwebkit-install.log`, `smoke-build.log`, `verify.log`
- `build-manifest.txt`: runtime, source URLs/checksums, configure flags, verification summary

## Verification Gates
A successful run must satisfy:
1. `qmake -query QT_VERSION` is `5.15.17`.
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
4. Required static ICU libs exist in the container image:
   - `libicuuc.a`
   - `libicui18n.a`
   - `libicudata.a`
5. The QtWebKit smoke app compiles and links against the produced toolchain.
6. Verification log ends with `verification passed`.

## Current Status
Implemented target behavior:
- Containerized build pipeline.
- Source lock + checksum verification.
- Static ICU from the container image.
- Static Qt build.
- Standalone QtWebKit build against installed Qt.
- Static Qt + QtWebKit library verification.
- Static ICU library verification.
- QtWebKit smoke test compile/link gate.
- Manifest/log generation.

## Run Examples
Default run:
```bash
./build-qt5-static.sh
```

Clean rebuild:
```bash
./build-qt5-static.sh --clean
```

Custom output directory inside repo:
```bash
./build-qt5-static.sh --output-dir artifacts/qt5-static-poc-run1
```

## Risks
- Legacy Qt/QtWebKit code may fail under newer host and container toolchains.
- Static QtWebKit can still require follow-up patches for newer compilers or linkers.
- Archive URLs may become unavailable over time.
- Over-aggressive dependency reduction can break TLS, certificate handling, or module detection.
- Runtime browser validation must use the host-side `run-browser.sh --check`
  helper for the Docker-built browser.

## Next Tasks
1. Build the main browser with `build-browser-docker.sh` against the produced toolchain.
2. Fix Qt `5.15.17` or QtWebKit `5.212` source incompatibilities exposed by that build.
3. Move the Docker base image from tag pinning to digest pinning when refreshed online.

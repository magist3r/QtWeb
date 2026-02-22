# Qt5 + QtWebKit Migration Plan

## Goal
Port QtWeb from Qt4.8.x to Qt5 (with QtWebKit), keeping behavior as close as possible, minimizing feature loss, and maximizing portability by minimizing runtime dependencies.

## POC Spec
- Static Qt5 toolchain POC is tracked in `docs/qt5-static-poc.md`.
- This keeps the migration roadmap high-level here while the POC execution spec remains implementation-focused.

## Scope
- In scope:
  - Build system and source compatibility for Qt5 + QtWebKit.
  - Preserve the static-build deployment model (or an equivalent option), since the current custom Qt build exists to maximize portability and keep distribution size small.
  - Minimize non-essential runtime dependencies (shared libraries/plugins) so the produced build is as portable as practical.
  - Keep current architecture (`BrowserApplication`, `BrowserMainWindow`, `WebView`, `WebPage`, managers).
  - Keep qmake build.
- Out of scope:
  - Qt6/WebEngine rewrite.
  - UI redesign or major feature changes.
  - Full refactor to modern C++/Qt patterns.

## Target Baseline
- Qt `5.5.1` target baseline for migration (latest Qt5 release line that still ships QtWebKit from Qt's own release artifacts).
- QtWebKit module available (`webkit`, `webkitwidgets`) from Qt's own `qtwebkit-opensource-src-5.5.1` release package.
- ICU is a hard dependency for Qt5 QtWebKit builds; static Qt5 + QtWebKit path requires ICU-enabled Qt configuration.
- qmake-based build producing equivalent desktop app behavior.

### Why this version
- Qt `5.5.1` submodule releases include `qtwebkit-opensource-src-5.5.1`.
- Qt `5.6.0` submodule release packages no longer include `qtwebkit`.
- Qt Wiki (Qt 5.6 notes) documents QtWebKit as removed from release packages in 5.6.
- Therefore, if we require Qt-provided (non-fork) QtWebKit in release artifacts, `Qt 5.5.1` is the latest stable target.

## Current Blockers (from code scan)
- Qt4 module includes are pervasive (`<QtGui/...>`, `<QtWebKit/...>`): ~259 include lines.
- Removed/changed APIs in use:
  - `QFtp` (`src/webview.h`, `src/webview.cpp`).
  - `Qt::escape` (`src/networkaccessmanager.cpp`).
  - `QDesktopServices::storageLocation` (`src/browserapplication.cpp`).
  - `qVariantValue<T>` (`src/browserapplication.cpp`, `src/settings.cpp`).
  - `QUrl::queryItems()` (`src/autocomplete.cpp`).
- Qt4 platform macros used (`Q_WS_WIN`, `Q_WS_MAC`) in multiple files.
- Build scripts are Qt4-source-centric (`build.sh`, `get-src.sh`, `qt-patches/`).

## Execution Phases

## Phase 0: Freeze Baseline
- Actions:
  - Record current Qt4 behavior and create a manual smoke checklist.
  - Tag current state in git.
- Files:
  - `README.md` (add baseline run notes).
  - New `docs/manual-smoke-test.md`.
- Exit criteria:
  - Repeatable baseline test list exists before migration edits.

## Phase 1: Build System Split (Qt4 legacy path + Qt5 path)
- Actions:
  - Keep existing Qt4 `build.sh` untouched for legacy path.
  - Add Qt5 build entry path (new script) that uses system Qt5, not patched Qt4 source tree.
  - Define how static linking is handled in Qt5 builds so portability/size goals remain explicit.
  - Make ICU an explicit build gate for Qt5 QtWebKit (no `-no-icu` path for WebKit-enabled builds).
  - Define a dependency-minimization policy for Qt5 artifacts (modules, plugins, and optional features to avoid unless required).
  - Add a basic dependency audit step (for example `ldd` on Linux) so dependency growth is visible during migration.
  - Update qmake project modules conditionally by Qt major version.
- Files:
  - `src/QtWeb.pro`
  - New `build-qt5.sh`
  - `README.md`
- Required qmake changes:
  - Qt4: `QT += network xml webkit`
  - Qt5: `QT += core gui widgets network xml webkit webkitwidgets printsupport`
- Exit criteria:
  - qmake configure step works under Qt5 (`qmake` generates Makefile).
  - Portability gates are defined: no Qt shared-library runtime dependency and dependency audit procedure is documented.

## Phase 2: Platform Macro and Header Compatibility Sweep
- Actions:
  - Replace `Q_WS_WIN` -> `Q_OS_WIN`, `Q_WS_MAC` -> `Q_OS_MAC`.
  - Convert module-specific Qt4 includes to Qt5-compatible includes.
  - Start with minimal-impact mechanical edits.
- Priority files:
  - `src/browserapplication.cpp`
  - `src/browsermainwindow.cpp`
  - `src/settings.cpp`
  - `src/bookmarksimport.cpp`
  - `src/networkaccessmanager.cpp`
  - `src/googlesuggest.cpp`
- Exit criteria:
  - No compile errors from platform macro usage.
  - Main UI compilation passes include/module resolution stage.

## Phase 3: API Migration (Compile Breakers)
- Actions:
  - Replace `Qt::escape(x)` with `x.toHtmlEscaped()`.
  - Replace `qVariantValue<T>(...)` with `...value<T>()`.
  - Replace `QDesktopServices::storageLocation(...)` with `QStandardPaths::writableLocation(...)`.
  - Replace `QUrl::queryItems()` with `QUrlQuery`.
- Priority files:
  - `src/networkaccessmanager.cpp`
  - `src/browserapplication.cpp`
  - `src/settings.cpp`
  - `src/autocomplete.cpp`
- Exit criteria:
  - Project compiles through these units under Qt5.

## Phase 4: QtWebKit and Printing Boundary Fixes
- Actions:
  - Ensure `QWebView/QWebPage/QWebFrame` includes use Qt5 QtWebKit headers.
  - Ensure print classes resolve under `QtPrintSupport`.
  - Fix signal/slot overload mismatches if exposed by stricter Qt5 checks.
- Priority files:
  - `src/webview.*`
  - `src/webpage.*`
  - `src/tabwidget.*`
  - `src/browsermainwindow.*`
  - `src/savepdf.*`
- Exit criteria:
  - Browser window opens, tabs load pages, print dialog opens.

## Phase 5: FTP Decision (Hard Block)
- Problem:
  - `QFtp` is removed in Qt5.
- Options:
  - Option A (recommended first): feature-flag and disable FTP browsing path for Qt5.
  - Option B: reimplement FTP using another library/backend.
- Files:
  - `src/webview.h`
  - `src/webview.cpp`
  - UI/help text assets if behavior changes.
- Exit criteria:
  - No Qt5 compile dependency on `QFtp`.
  - Defined runtime behavior for `ftp://` URLs.

## Phase 6: Runtime Behavior Parity
- Actions:
  - Run manual smoke tests and fix regressions.
  - Validate settings persistence keys remain compatible.
  - Run dependency audit on produced artifacts and reduce avoidable runtime dependencies.
- Test focus areas:
  - Single-instance behavior and URL handoff.
  - Tab creation rules (current tab/new tab/new window).
  - Downloads and save dialogs.
  - SSL warning prompt behavior.
  - Proxy and adblock settings.
  - Session save/restore.
  - Bookmark/history/password dialogs.
- Exit criteria:
  - Core workflows function on Qt5 build with no critical regressions.
  - Runtime dependency set is minimized and documented (including justified unavoidable system dependencies).

## Phase 7: Cleanup and Hardening
- Actions:
  - Remove stale files (`src/webpage.cpp.bak`).
  - Document known differences between Qt4 and Qt5 build.
  - Add CI job for Qt5 build (if CI is introduced).
- Exit criteria:
  - Clean repository and clear migration notes in docs.

## Suggested Delivery Slices
- Slice 1:
  - Phase 1 + Phase 2 mechanical changes.
- Slice 2:
  - Phase 3 API replacements.
- Slice 3:
  - Phase 4 WebKit/print fixes.
- Slice 4:
  - Phase 5 FTP strategy + Phase 6 smoke stabilization.

## Risk Notes
- Highest risk area is `webview/webpage/tabwidget` behavior changes.
- `QSettings` key naming must remain stable to preserve user data compatibility.
- `build.sh` is legacy-Qt4 specific; avoid mixing Qt5 flow into that script.
- Over-aggressive dependency stripping can break runtime features (for example TLS/certificates, platform integration, and rendering paths); changes need smoke coverage.
- Building Qt5 statically with QtWebKit while keeping runtime dependencies minimal requires explicit ICU strategy (prefer static ICU linkage in the Qt5 toolchain path).

## Immediate Next Tasks (start now)
- Update `src/QtWeb.pro` with Qt version conditionals for module lists.
- Add `build-qt5.sh` with qmake/make path.
- Define portability/dependency gates for Qt5 outputs (what is allowed vs disallowed at runtime).
- Apply mechanical macro replacements (`Q_WS_*` -> `Q_OS_*`) in core files.
- Migrate `Qt::escape`, `qVariantValue`, `storageLocation`, `queryItems` in first pass.

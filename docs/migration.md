# Qt5 + QtWebKit Migration Plan

## Goal
Port QtWeb from Qt4.8.x to Qt5 + QtWebKit with minimal behavior change, minimal feature loss, and minimal runtime dependencies.

## Scope
- In scope:
  - Qt5 build compatibility for current application architecture.
  - qmake-based build flow.
  - Static-build-friendly output and dependency minimization.
- Out of scope:
  - Qt6/WebEngine rewrite.
  - UI redesign.
  - Broad refactors unrelated to migration blockers.

## Baseline and Constraints
- Target baseline: Qt `5.5.1` (last Qt5 release line with Qt-provided QtWebKit artifacts).
- Required modules under Qt5: `core gui widgets network xml webkit webkitwidgets printsupport`.
- ICU is required for Qt5 + QtWebKit builds.
- Keep legacy Qt4 `build.sh` path intact; introduce a separate Qt5 build path.

## Main Blockers
- Qt4 platform macros: `Q_WS_WIN`, `Q_WS_MAC`.
- Qt4-only APIs:
  - `Qt::escape`.
  - `qVariantValue<T>`.
  - `QDesktopServices::storageLocation`.
  - `QUrl::queryItems()`.
  - `QFtp` (removed in Qt5).
- Qt4-style module/header usage throughout browser UI and networking layers.

## Migration Phases
1. Baseline freeze
- Create and keep a manual smoke checklist before code changes.

2. Build split
- Keep `build.sh` for Qt4.
- Add a Qt5 build entry script and Qt-version conditionals in `src/QtWeb.pro`.
- Document dependency audit expectations (for example `ldd` on Linux artifacts).

3. Mechanical compatibility
- Replace `Q_WS_*` macros with `Q_OS_*`.
- Update includes/module usage for Qt5.

4. API and WebKit boundary fixes
- Replace:
  - `Qt::escape(x)` -> `x.toHtmlEscaped()`.
  - `qVariantValue<T>(v)` -> `v.value<T>()`.
  - `QDesktopServices::storageLocation(...)` -> `QStandardPaths::writableLocation(...)`.
  - `QUrl::queryItems()` -> `QUrlQuery`.
- Fix Qt5 QtWebKit/PrintSupport compile boundaries (`webview`, `webpage`, `tabwidget`, print flow).

5. FTP and parity stabilization
- Decide Qt5 `ftp://` behavior (disable path or reimplement backend).
- Run smoke validation and resolve behavior regressions.

## High-Risk Areas
- Browser behavior in `webview/webpage/tabwidget` (tab/window/navigation semantics).
- `QSettings` key stability (silent compatibility regressions if keys drift).
- Runtime dependency creep during static-toolchain work.

## Known Issues to Track During Migration
- SSL approval logic bug in `src/networkaccessmanager.cpp` (`QMessageBox::YesToAll` constant misuse in condition).
- Locking misuse in `src/browserapplication.cpp` (`QReadLocker` used while mutating `s_hostIcons`).
- Proxy exceptions key mismatch in `src/settings.cpp` (`exceptions` vs `Exceptions`).
- Build script jobs/default mismatch in `build.sh` and `MAKE_COMMAND` initialization timing.
- Stale artifact: `src/webpage.cpp.bak`.

## Validation Checklist
- qmake generation works under Qt5.
- Core UI opens and pages load.
- New tab/new window behavior matches baseline.
- Downloads and save dialogs function.
- SSL warning flow behaves correctly.
- Proxy/adblock settings work.
- Session restore works.
- No unintended settings-key renames.
- Runtime dependency set is documented and minimized.

## Immediate Next Tasks
- Add Qt-version conditionals in `src/QtWeb.pro`.
- Add `build-qt5.sh` (separate from legacy `build.sh`).
- Apply `Q_WS_*` -> `Q_OS_*` replacements in core files.
- Migrate first-pass API breakers (`escape`, `qVariantValue`, `storageLocation`, `queryItems`).
- Define and document Qt5 dependency gates.

## Repository Reference (Condensed)
- Entry point: `src/main.cpp` -> `BrowserApplication`.
- Core shell: `browserapplication.*`, `browsermainwindow.*`.
- Tab/navigation: `tabwidget.*`, `webview.*`, `webpage.*`.
- Networking/data: `networkaccessmanager.*`, `cookiejar.*`, `downloadmanager.*`, bookmarks/history/password/search managers.
- Build system is qmake-based and intentionally legacy/static-oriented.
- Preserve existing Qt4 signal/slot style in touched areas unless a full migration edit requires otherwise.

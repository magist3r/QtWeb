# AGENTS.md

## Purpose
This repository is a legacy Qt4 browser project (`QtWeb`) with an embedded custom Qt/WebKit build pipeline.  
This guide documents QtWeb-specific architecture, constraints, and risk areas.

## Repository Structure
- `QtWeb.pro`: top-level qmake `subdirs` entry point.
- `src/QtWeb.pro`: main app target (`TEMPLATE = app`) with static Qt configuration and all sources/forms/resources.
- `src/`: browser application sources.
- `src/torrent/`: bundled torrent client subsystem and UI.
- `src/data`, `src/htmls`, `src/Resources`: runtime assets and translations.
- `build.sh`: end-to-end build script for patched Qt + app.
- `get-src.sh`: fetches legacy source tarballs used by `build.sh`.
- `qt-patches/`: patches applied to Qt/QtWebKit/mkspecs before build.

## High-Level Architecture
- Entry: `src/main.cpp` initializes resources and starts `BrowserApplication`.
- App lifecycle/service locator: `src/browserapplication.*`.
- Main UI shell: `src/browsermainwindow.*`.
- Tabs/navigation:
  - `src/tabwidget.*` manages tabs and per-tab URL bars.
  - `src/webview.*` is the page widget.
  - `src/webpage.*` custom navigation behavior and window/tab opening.
- Networking/security/content handling:
  - `src/networkaccessmanager.*` proxy/adblock/auth/SSL handling.
  - `src/cookiejar.*`, `src/downloadmanager.*`.
- Data models and persistence:
  - `src/bookmarks.*`, `src/history.*`, `src/passwords.*`, `src/searches.*`.
  - Extensive `QSettings` group-based persistence.
- Optional torrent UI/engine: `src/torrent/*`.

## Build and Tooling Reality
- Primary build system is qmake (Qt4-era), not CMake.
- The custom Qt build phase exists intentionally to produce static binaries for maximum portability and a small distributable footprint.
- For Qt5 migration work, portability means both static linking and minimizing non-essential runtime dependencies.
- Typical flow:
  - Ensure Qt sources are extracted into `src/qt` (`src/qt/readme.txt`).
  - Optionally run `./get-src.sh` (legacy URLs may be unavailable today).
  - Run `./build.sh`.
- `build.sh` applies/reverts patches in `qt-patches/` and creates `./build`.
- There is no modern CI/test harness in this repo.

## Code Patterns and Style
- Predominantly Qt4 idioms:
  - Macro-based signal/slot connections (`SIGNAL(...)`, `SLOT(...)`).
  - `foreach`, `QRegExp`, `qVariantValue`, `QDesktopServices::storageLocation`, QtWebKit classes.
- Memory management is mixed:
  - Heavy QObject parent ownership.
  - Also explicit `new`/`delete` for singleton-like services and containers.
- Naming conventions:
  - Methods: camelCase.
  - Members: `m_` prefix.
  - Static service pointers in `BrowserApplication` use `s_` prefix.
- UI:
  - `.ui` forms compiled by qmake.
  - Actions/menus centralized in `src/commands.*`.
- Broad modernization can easily destabilize behavior.

## Practical Editing Guidance
- Preserve existing Qt4 signal/slot style in nearby code unless you are doing a deliberate full migration.
- Verify `QSettings` key names carefully; key drift causes silent behavior regressions.
- Respect singleton/service initialization ordering in `BrowserApplication`.
- Avoid introducing new runtime dependencies unless required; if unavoidable, document and justify them in migration docs.
- For browser behavior changes, verify at least:
  - New tab/new window behavior.
  - Session restore.
  - Download flow.
  - Proxy/adblock toggles.
  - SSL warning flow.

## Known Issues and Risk Hotspots
- SSL approval logic bug:
  - `src/networkaccessmanager.cpp:321`
  - `if (ret == QMessageBox::Yes || QMessageBox::YesToAll)` is always true because the second operand is a constant.
- Locking misuse:
  - `src/browserapplication.cpp:257`
  - `QReadLocker` is used while mutating `s_hostIcons`; should be write lock semantics.
- Proxy exceptions settings inconsistency in settings UI load/save:
  - Load reads lowercase key: `src/settings.cpp:410` (`"exceptions"`).
  - Save writes uppercase key: `src/settings.cpp:606` (`"Exceptions"`).
- Build script argument mismatch/fragility:
  - Default jobs variable is `8`: `build.sh:11`.
  - Help text says default is `4`: `build.sh:75`.
  - `MAKE_COMMAND` is initialized before `--jobs` parsing (`build.sh:12`, `build.sh:63-66`), so changing jobs may not affect make command.
- Legacy/deprecated surface area:
  - `Qt::escape` usage: `src/networkaccessmanager.cpp:252`, `src/networkaccessmanager.cpp:276`.
  - `QFtp`-based FTP path: `src/webview.h:116`.
  - Extensive QtWebKit dependence across web UI/network flow.
- Stale artifact in tree:
  - `src/webpage.cpp.bak` exists and can cause confusion during searches/reviews.

## What To Check Before Submitting Changes
- Build still starts with existing qmake flow (at minimum, no obvious qmake/project breakage).
- No accidental settings-key renames.
- No new ownership leaks in long-lived managers.
- No regressions in tab/session/network behavior.
- For Qt5/static toolchain work, verify produced artifacts do not gain avoidable runtime dependencies (for example via `ldd`) and document any unavoidable ones.
- Keep platform-specific code paths (`Q_WS_WIN`, old mac/linux branches) compiling when touched.

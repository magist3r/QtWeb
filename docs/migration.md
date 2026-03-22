# Qt5 Browser Port Implementation Plan

## Summary
Port QtWeb to the existing static Qt `5.5.1` toolchain on Linux while keeping the legacy Qt4 flow intact. The implementation base stays on `qt5-migration`, and the existing `qt5` branch is used as the donor for already-solved Qt5 application changes. The first source mutation is the automated Qt4-to-Qt5 header rewrite, followed by targeted manual integration until the full browser builds and runs with torrent and FTP support preserved. All Qt5 browser validation builds must run inside Docker against the repository's containerized toolchain environment rather than on the host, and they must be invoked through the repository helper scripts rather than direct `docker` or ad hoc build commands. Host-local `qmake`/`make`/compiler validation runs are explicitly out of policy for this repository.

## Decisions
- Delivery branch: `qt5-migration`
- Donor branch for application porting work: `qt5`
- Target runtime/build baseline: Qt `5.5.1` static Linux `x86_64`
- Required build environment for Qt5 validation: Docker container using the repository image/toolchain
- Required invocation path for Qt5 validation: repository helper scripts such as `build-browser-docker.sh` and `run-broswer.sh`, not direct `docker` commands
- Keep the existing Qt4 `build.sh` path unchanged
- Preserve torrent support in the first Qt5 browser milestone
- Preserve FTP browsing and FTP downloads in the first Qt5 browser milestone

## Implementation Order
1. Update this document with the concrete execution plan.
2. Run `/home/magist3r/code/qtbase/bin/fixqt4headers.pl` against `src/` and review the generated include rewrites.
3. Bring the qmake project files up to Qt5.5.1:
- `src/QtWeb.pro` must use `widgets`, `webkitwidgets`, and `printsupport`
- `src/torrent/torrent.pro` must stay compatible with Qt `5.5.1` qmake and must not depend on `requires(qtConfig(filedialog))`
4. Replay the relevant Qt5 source-port work from `qt5`:
- Qt5 include/module split across browser UI, WebKit, and print code
- `QStandardPaths` for storage paths
- `QUrlQuery` for query parsing
- Torrent networking migration from `QHttp` to `QNetworkAccessManager`
5. Adapt donor-branch code back down to Qt `5.5.1` where it assumes newer Qt:
- replace `QDateTime::currentSecsSinceEpoch()`
- replace `QRandomGenerator`
- replace `QOverload` connect syntax
6. Restore and keep both legacy feature areas:
- torrent remains built into the application
- FTP remains supported in `webview` for directory listing and file download
7. Validate the main browser inside Docker on top of the existing smoke-test/toolchain work, using the repository helper scripts instead of direct container or compiler invocations.

## Public and Internal Interfaces
- No new user-facing CLI or configuration layer is added.
- Internal build interfaces change to Qt5-aware qmake module usage in:
- `src/QtWeb.pro`
- `src/torrent/torrent.pro`
- Torrent internal types move from `QHttp`-based APIs to `QNetworkAccessManager` / `QNetworkReply`.
- Storage path handling moves to `QStandardPaths`.
- Query parsing moves from deprecated Qt4 APIs to `QUrlQuery`.

## Required Behavior
- The browser must launch and render pages under Qt5.
- Tabs, windows, and navigation behavior must remain functional.
- Downloads must continue to work.
- Torrent support must build and reach tracker communication under Qt5.
- `ftp://` directory browsing must still work.
- FTP file downloads must still work.
- Portable and non-portable settings/data paths must continue to resolve correctly.

## Validation
- Documentation reflects the chosen implementation path and constraints.
- qmake generation succeeds inside Docker when invoked through `build-browser-docker.sh`.
- A clean out-of-tree browser build succeeds inside Docker through the helper scripts, with outputs kept in repository-local `build-docker-*` directories.
- Validate changed Bash / POSIX shell scripts with `shellcheck` when script changes are part of the work.
- Browser smoke validation covers page loading, tabs, windows, downloads, and print-related actions.
- Torrent validation covers successful build and tracker communication after the networking port.
- FTP validation covers directory listing and file download.
- Regression checks cover settings/data paths and autocomplete/password/query parsing after API replacements.

## Known Risks
- The `qt5` donor branch was written against newer Qt and contains incompatible APIs for `5.5.1`.
- FTP was partially disabled in the donor branch and must be reintroduced without regressing navigation behavior.
- Settings-path or key drift can create silent compatibility regressions.
- Static-build constraints may surface additional link/runtime issues after compilation succeeds.
- Host-native builds can hide or invent problems relative to the intended toolchain, so the Docker helper-script path is the source of truth for Qt5 validation.

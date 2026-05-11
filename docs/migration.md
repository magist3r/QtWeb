# Save Migration Plan In `docs` First, Then Execute Qt 5.15.17 Static Migration

## Summary
- The first step is documentation: save the migration plan in `docs` before any implementation work starts, so the repo has a single agreed source of truth for the Qt `5.15.17` + QtWebKit `5.212` migration.
- After that, migrate the current `qt5-migration` branch from Qt `5.5.1` to Qt `5.15.17` while keeping QtWebKit `5.212`.
- Do not use the `qt5` branch as a donor or reference baseline. All porting work is done in place from the current `qt5-migration` tree.
- Replace the current Qt `5.5.1` static toolchain flow in place rather than adding a parallel Qt `5.15` path.

## Key Changes
- Save this plan first in `docs/migration.md`, replacing the current Qt `5.5.1` / donor-branch-oriented plan before any code or script changes begin.
- Retarget the existing toolchain flow in `build-qt5-static.sh`, `toolchains/qt5-static/sources.lock`, and `toolchains/qt5-static/qt5-static-build-entrypoint.sh`:
- update locked sources from Qt `5.5.1` to Qt `5.15.17`
- replace the current "unpack `qtwebkit` into the Qt source tree" flow with a standalone QtWebKit `5.212` build against the installed Qt `5.15.17` prefix
- keep static ICU and OpenSSL handling, and continue verifying `libQt5WebKit.a` and `libQt5WebKitWidgets.a`
- Retarget `build-browser-docker.sh` and the QtWebKit smoke build to the new install prefix, image tag, artifact naming, and verification gates.
- Keep `src/QtWeb.pro` on `webkitwidgets`; do not introduce `webenginewidgets` or any engine-switch abstraction.
- Port the application code directly from current `qt5-migration` sources:
- fix compile and link issues caused by Qt `5.15.17` API tightening
- fix any QtWebKit `5.212` API or behavior differences
- preserve existing browser behavior, torrent support, FTP support, downloads, printing, and settings behavior unless a concrete incompatibility forces a narrow adjustment

## Execution Order
1. Replace `docs/migration.md` with the Qt `5.15.17` + QtWebKit `5.212` migration plan and treat that document as the implementation baseline.
2. Replace the current Qt `5.5.1` toolchain lock, artifact names, and verification assumptions with Qt `5.15.17`.
3. Implement the standalone QtWebKit `5.212` build/install step and prove the static libraries install into the Qt prefix.
4. Update the QtWebKit smoke build and require it to compile/link successfully inside Docker before proceeding.
5. Retarget the main browser Docker build helper to the new prefix and compile the current app as-is to expose the real Qt `5.15.17` breakage list.
6. Fix the browser code in place on `qt5-migration`, starting with WebKit-facing code and then any remaining Qt `5.15.17` compatibility issues.
7. Validate build success through the sanctioned helper scripts and update the remaining Qt5 toolchain docs to match the implemented flow.

## Validation
- Documentation gate: `docs/migration.md` is updated first and accurately describes the chosen migration strategy.
- Toolchain gate: `build-qt5-static.sh` produces a static Qt `5.15.17` install and static QtWebKit `5.212` libraries.
- Smoke gate: `smoke-tests/qtwebkit-smoke` builds and links against the produced prefix in Docker.
- Browser gate: `build-browser-docker.sh` completes for release; debug is secondary after release passes.
- Runtime regression testing is explicitly user-owned and out of scope for agent execution.

## Assumptions And Defaults
- Work starts from current `qt5-migration` head only; the `qt5` branch is explicitly out of scope.
- Qt baseline is pinned to `5.15.17`.
- Linux `x86_64` only.
- Existing Qt4 flow stays untouched.
- QtWebKit `5.212` is accepted despite its age and security risk; this migration plan does not include an engine replacement track.
- If the static QtWebKit smoke gate fails and cannot be resolved cleanly, the migration stops with a documented blocker rather than pivoting to Qt WebEngine.

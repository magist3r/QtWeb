# AGENTS.md

## Code Style
- Do not introduce one-off variables for simple values or small argument groups. Keep values inline at the use site unless a variable removes real duplication, clarifies non-obvious logic, or matches nearby style.
- When individual shell command arguments need comments, prefer a single argument array with comments beside the relevant entries instead of separate helper variables for small flag groups.
- In shell scripts, do not build command strings or use `eval`; use direct command calls or argument arrays so quoting and word splitting stay explicit.
- Do not pass long inline shell programs to Docker with `bash -c`/`bash -lc`; put the container-side logic in a checked-in script and execute that script.
- Do not add helper functions that are only called once unless they isolate a meaningful phase, cleanup/error boundary, or repeated validation pattern.
- Treat `build.sh` as a legacy script: do not apply cleanup/style rewrites there unless the user explicitly asks to modernize that script.

## Build And Validation Policy
- Do not run host-local or ad hoc builds in this repository.
- Do not invoke `qmake`, `make`, `cmake`, `ninja`, or compiler commands directly on the host for validation or debugging.
- Use the repository's sanctioned helper-script flows instead, such as `build-browser-docker.sh`, `build-qt5-static.sh`, and `run-smoke.sh`, when a build or runtime validation is explicitly required.
- For any `build-qt5-static.sh` request, including short requests like "clean rebuild", use the `.agents/build-static-qt5.toml` subagent instead of running the long build directly in the main agent.
- If that sanctioned path is unavailable, blocked, or out of scope for the task, state that validation could not be run. Do not fall back to a local build.
- Do not launch the built `QtWeb` binary or other produced executables as an agent.
- Do not run `run-smoke.sh` or start GUI/runtime validation on the user's behalf unless the user explicitly asks for that exact execution.
- Use `./run-smoke.sh --check` and `./run-browser.sh --check` to check existing Docker-built binaries for remaining shared dependencies.
- When a build succeeds, report the output path or the command the user can run; leave execution to the user unless explicitly requested.

## QtWeb Static Build Notes
- Shared Qt build defaults live in `toolchains/qt5-static/common.sh`: Qt `5.15.17`, image tag `qtweb-qt5-static:5.15.17`.
- Static-linking fixes must be applied in the Qt or QtWebKit build itself, such as configure inputs, source patches, or checked-in container-side build scripts. This keeps future Qt/QtWebKit upgrades honest: patches should fail early and be rebased deliberately instead of relying on post-build metadata rewrites or other broken hacks.
- For local validation, check only debug builds to save time; release builds are validated by CI.
- Use the `.agents/build-static-qt5.toml` subagent when running full or scoped `./build-qt5-static.sh` flows so long builds are monitored without unrelated edits or commands. The default clean rebuild command is `./build-qt5-static.sh --runtime docker --debug --clean`.
- Use `./build-qt5-static.sh --runtime docker --qt-only` for the static Qt stage and `./build-qt5-static.sh --runtime docker --qtwebkit-only` for the QtWebKit stage.
- Use `./build-browser-docker.sh` for the QtWeb browser stage; add `--analyze` only when clang-tidy analysis is explicitly requested.
- Use `./smoke-tests/qtwebkit-smoke/smoke-build-docker.sh` for the QtWebKit smoke-test build.
- Runtime checks are host-side wrappers and must only be run on explicit request: `./run-smoke.sh --runtime-check about:blank` or `./run-browser.sh --runtime-check about:blank`.
- The CI workflow is `.github/workflows/qt5-static.yml`; its main order is `qt5` -> `qtwebkit` -> `qtweb`, followed by publish jobs.
- For build failures, classify the first real error as configure-time, compile-time, link-time, runtime/smoke-test, CI resource exhaustion, or missing static dependency before editing scripts.

## Documentation Policy
- Keep `docs/migration-status.md` limited to behavior that is implemented in the repository.
- Keep `docs/plan.md` limited to planned or pending work.
- When implementing a planned change, update both docs in the same change: move completed points from `docs/plan.md` to `docs/migration-status.md` and remove stale planned items.

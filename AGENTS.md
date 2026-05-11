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
- If that sanctioned path is unavailable, blocked, or out of scope for the task, state that validation could not be run. Do not fall back to a local build.
- Do not launch the built `QtWeb` binary or other produced executables as an agent.
- Do not run `run-smoke.sh` or start GUI/runtime validation on the user's behalf unless the user explicitly asks for that exact execution.
- Use `ldd ./smoke-tests/qtwebkit-smoke/build-docker-$(build-type)/qtwebkit-smoke` to check for remaining shared dependencies
- When a build succeeds, report the output path or the command the user can run; leave execution to the user unless explicitly requested.

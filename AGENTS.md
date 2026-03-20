# AGENTS.md

## Build And Validation Policy
- Do not run host-local or ad hoc builds in this repository.
- Do not invoke `qmake`, `make`, `cmake`, `ninja`, or compiler commands directly on the host for validation or debugging.
- Use the repository's sanctioned helper-script flows instead, such as `build-browser-docker.sh`, `build-qt5-static.sh`, and `smoke-run.sh`, when a build or runtime validation is explicitly required.
- If that sanctioned path is unavailable, blocked, or out of scope for the task, state that validation could not be run. Do not fall back to a local build.
- Do not launch the built `QtWeb` binary or other produced executables as an agent.
- Do not run `smoke-run.sh` or start GUI/runtime validation on the user's behalf unless the user explicitly asks for that exact execution.
- When a build succeeds, report the output path or the command the user can run; leave execution to the user unless explicitly requested.

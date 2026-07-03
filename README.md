QtWeb
=====

QtWeb Internet Browser

Build policy: do not use host-local or ad hoc builds for this repository. Use the repository helper scripts and containerized flows for build or validation work; if that path is unavailable, report validation as not run instead of falling back to local `qmake`/`make` commands. `./run-smoke.sh --check` and `./run-browser.sh --check` validate shared dependencies for existing Docker-built binaries. `./run-smoke.sh --runtime-check about:blank` and `./run-browser.sh --runtime-check about:blank` are the host-side runtime checks. Agents may only use `run-smoke.sh` and `run-browser.sh` for runtime checks; directly launching the built `QtWeb` binary or other produced executables is prohibited.

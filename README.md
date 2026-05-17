QtWeb
=====

QtWeb Internet Browser

Build policy: do not use host-local or ad hoc builds for this repository. Use the repository helper scripts and containerized flows for build or validation work; if that path is unavailable, report validation as not run instead of falling back to local `qmake`/`make` commands. `./run-smoke.sh --check about:blank` is a valid host-side runtime check for an existing Docker-built smoke binary. After rebuilding the browser, `./run-browser.sh --check about:blank` is the valid host-side runtime check for the Docker-built browser. Agents may only use `run-smoke.sh` and `run-browser.sh` for runtime checks; directly launching the built `QtWeb` binary or other produced executables is prohibited.

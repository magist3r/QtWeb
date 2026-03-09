QtWeb
=====

QtWeb Internet Browser

Build policy: do not use host-local or ad hoc builds for this repository. Use the repository helper scripts and containerized flows for build or validation work; if that path is unavailable, report validation as not run instead of falling back to local `qmake`/`make` commands. Agents should not launch the built `QtWeb` binary or other produced executables unless explicitly asked; they should report the path or command for the user to run instead.

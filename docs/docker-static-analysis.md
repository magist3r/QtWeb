# Docker Static Analysis Plan

## Summary
Integrate `clang-tidy` and `clazy` with the existing Docker build path for QtWeb instead of adding a separate ad hoc workflow. Docker remains the canonical environment for generating the browser build and `compile_commands.json`, and `build-browser-docker.sh` remains the main entrypoint for both browser builds and static-analysis runs.

The first milestone is reproducible Docker-backed analysis plus a conservative cleanup wave for the hand-written QtWeb application code. The goal is to establish a stable baseline and reduce high-value findings without broad refactors or behavior changes.

## Decisions
- Docker is the canonical build environment and the source of truth for the compilation database.
- Docker also runs `clang-tidy` so it stays on a toolchain version compatible with the Qt `5.5.1` headers in this repository.
- Host-installed `clazy-standalone` consumes the Docker-produced `compile_commands.json`.
- `build-browser-docker.sh` is the main entrypoint for Dockerized analysis.
- The initial scope is application code only:
  - `src/*.cpp`
  - `src/torrent/*.cpp`
- The first cleanup wave is conservative and focuses on low-risk fixes only.
- Smoke tests, extracted Qt sources, patch files, and generated build outputs are out of scope for the initial analyzer integration.

## Required Tooling
The Docker image used for browser builds must provide:
- `bear`
- `clang`
- `clang-tidy`

The host environment running `build-browser-docker.sh --analyze` must provide:
- `clazy-standalone`

This keeps the build path aligned with Docker while avoiding two compatibility problems:
- Ubuntu `16.04` does not provide `clazy` from the default package repositories.
- the host `clang-tidy` available in this environment is too new for the Qt `5.5.1` headers used by this codebase.

## Implementation Changes
Extend [`build-browser-docker.sh`](/home/magist3r/code/QtWeb/build-browser-docker.sh) with explicit analysis modes while preserving its current build behavior.

Expected script behavior:
- `--debug` continues selecting the debug Docker build path.
- `--analyze` runs qmake, captures a compile database, and runs both analyzers.
- `--analyze-only` runs the analysis flow without keeping a separate build-only mode as the primary action.
- Optional convenience flags such as `--clang-tidy-only` or `--clazy-only` can be added if they remain simple and do not complicate the interface.

Expected analysis flow:
1. Remove and recreate `build-docker-release` or `build-docker-debug` inside the Docker build run.
2. Run `qmake` exactly as the normal Docker build does today.
3. Run `bear -- make -j"${JOBS}"` in Docker to produce `compile_commands.json`.
4. During the container build run, execute `clang-tidy` against the scoped application sources using the generated compilation database.
5. After the container exits, run host `clazy-standalone` against the same source set using that Docker-produced compilation database.
6. Write reports into the Docker build directory so they remain visible in the repository.

## Scope Boundaries
Included:
- hand-written browser sources in `src/`
- hand-written torrent sources in `src/torrent/`

Excluded:
- `src/qt`
- `qt-patches/`
- smoke-test projects
- generated `moc/`, `rcc/`, `uic/`, and `qtweb_plugin_import.cpp`
- `artifacts/`
- existing `build-*` outputs except as analyzer working directories

## Analyzer Policy
The initial analyzer configuration should be intentionally narrow and low-noise.

Planned `clang-tidy` direction:
- conservative `readability-*`
- conservative `bugprone-*`
- simple `modernize-*` checks that do not force broad API churn

Planned `clazy` direction:
- Qt-specific checks for clear API misuse
- temporary and container inefficiencies
- connect-related issues with obvious fixes

The first cleanup wave should fix only findings that are clearly semantics-preserving, such as:
- dead includes
- unused locals
- redundant conditionals
- obvious Qt API misuse
- trivial temporary or container inefficiencies

The first cleanup wave should not include:
- ownership refactors
- threading changes
- signal/slot behavior changes
- persistence or settings behavior changes
- broad stylistic rewrites

## Outputs
Expected outputs in the selected Docker build directory:
- `build-docker-release/compile_commands.json`
- `build-docker-release/clang-tidy.txt`
- `build-docker-release/clazy.txt`
- corresponding debug variants when `--debug` is used

Optional exported fixes files may be added if they remain clearly scoped and reviewable.

## Validation
A successful first implementation should verify:
1. The Docker image contains `bear`, `clang`, and `clang-tidy`.
2. The host environment contains `clazy-standalone`.
3. `./build-browser-docker.sh --analyze` succeeds end to end.
4. `compile_commands.json` is generated in the selected Docker build directory.
5. The compile database contains representative entries for browser and torrent sources.
6. Both analyzer reports are written to the repository-local build directory.
7. After the first cleanup pass, the normal Docker build still succeeds.

Representative validation files:
- `src/browsermainwindow.cpp`
- `src/networkaccessmanager.cpp`
- `src/torrent/torrentclient.cpp`

## Risks
- Ubuntu `16.04` does not provide `clazy` from the default package repositories, so full in-image analyzer installation is not viable in the current base image.
- A host `clang-tidy` that is much newer than Qt `5.5.1` can fail inside Qt headers before project diagnostics are produced.
- Legacy Qt `5.5.1` code and its headers may produce noisy diagnostics if the enabled check set is too broad.
- Generated Qt sources can easily pollute analyzer output if the source list is not explicitly scoped.
- A zero-warning target is likely unrealistic for the first pass and would create unnecessary churn.

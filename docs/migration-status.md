# QtWeb Migration Status

- Qt `5.15.17` is the active Qt5 migration baseline.
- QtWebKit is pinned to the Movable Ink `2022-09-07` source archive.
- Static Qt5 release and debug toolchain builds are supported.
- Standalone QtWebKit builds against the static Qt5 toolchain are supported.
- Static Qt, QtWebKit, ICU, and OpenSSL library verification is implemented.
- QtWebKit smoke build validation is implemented.
- Docker browser builds against the static Qt5 toolchain are implemented.
- Host-side browser runtime checking through the repository helper is implemented.
- Docker-backed `clang-tidy` analysis is implemented.
- Analyzer fix export is implemented for `clang-tidy`.
- Generated build manifests and build logs are implemented.
- The browser remains on QtWebKit Widgets; no Qt WebEngine migration has been implemented.
- Existing Qt4 build flow remains untouched.

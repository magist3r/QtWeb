# QtWebKit Smoke Test

Minimal Qt 5.15.17 QtWebKitWidgets app used to verify that the custom Qt build
can compile and link a `QWebView` application.

## Files

- `qtwebkit-smoke.pro`: qmake project
- `main.cpp`: minimal `QWebView` app
- `smoke-build-docker.sh`: build in the same Docker environment as `build-qt5-static.sh`

## Build

Recommended:

```bash
cd smoke-tests/qtwebkit-smoke
./smoke-build-docker.sh
```

Debug build:

```bash
cd smoke-tests/qtwebkit-smoke
./smoke-build-docker.sh --debug
```

Output binary:

`smoke-tests/qtwebkit-smoke/build-docker-release/qtwebkit-smoke`

Debug output binary:

`smoke-tests/qtwebkit-smoke/build-docker-debug/qtwebkit-smoke`

## Host Check

After the Docker build has produced the smoke binary, run the headless check
from the repository root on the host:

```bash
./run-smoke.sh --check about:blank
```

This uses `xvfb-run` on the host. It does not build inside the host
environment.

## Notes

- Docker build is required because the smoke app must link against the SSL
  libraries bundled with the container toolchain.
- The current static plugin import includes `xcb` platform support. Running
  headless with `-platform offscreen` is not available with this build.

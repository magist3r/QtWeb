# QtWebKit Smoke Test

Minimal Qt 5.5.1 QtWebKitWidgets app used to verify that the custom Qt build
can compile and link a `QWebView` application.

## Files

- `qtwebkit-smoke.pro`: qmake project
- `main.cpp`: minimal `QWebView` app
- `smoke-build.sh`: local host build (uses a qmake `qt.conf` shim)
- `smoke-build-docker.sh`: recommended build in the same Docker environment as `build-qt5-static.sh`

## Build

Recommended:

```bash
cd smoke-tests/qtwebkit-smoke
./smoke-build-docker.sh
```

Output binary:

`smoke-tests/qtwebkit-smoke/build-docker/qtwebkit-smoke`

## Notes

- This Qt artifact is configured around `/workspace/...` paths from container
  builds, so host-only linking may fail unless the host has matching library
  layout/dependencies.
- The current static plugin import includes `xcb` platform support. Running
  headless with `-platform offscreen` is not available with this build.

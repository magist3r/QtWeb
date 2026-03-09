Qt4 Docker Builder
==================

These files add a Docker-based build environment for this repo.

Quick start
-----------

```bash
./docker-build.sh
```

That will:

1. build the `qtweb-qt4-builder` image from `Dockerfile.qt4-build`
2. mount this repository at `/workspace`
3. run `./get-src.sh`
4. run `./build.sh`
5. copy the built binary to `artifacts/QtWeb`

Arguments passed to `docker-build.sh` are forwarded to `build.sh`:

```bash
./docker-build.sh --jobs 4
./docker-build.sh --use-qtwebkit-23 --jobs 2
```

Artifacts remain in the repository working tree, including `temp-src/`, `src/qt/`, `build/`, and `artifacts/QtWeb`.

Manual usage
------------

```bash
docker build -t qtweb-qt4-builder -f Dockerfile.qt4-build .
docker run --rm -it \
  --user "$(id -u):$(id -g)" \
  -e HOME=/tmp/qtweb-home \
  -v "$PWD:/workspace" \
  -w /workspace \
  qtweb-qt4-builder \
  bash -lc './get-src.sh && ./build.sh'
```

#!/usr/bin/env python3
import sys

sys.dont_write_bytecode = True

import os
import re
import subprocess


ALLOWED_LIBRARIES = {
    "ld-linux-x86-64.so.2",
    "libatomic.so.1",
    "libbsd.so.0",
    "libc.so.6",
    "libdl.so.2",
    "libEGL.so.1",
    "libbrotlicommon.so.1",
    "libbrotlidec.so.1",
    "libbz2.so.1",
    "libfreetype.so.6",
    "libgcc_s.so.1",
    "libglib-2.0.so.0",
    "libGL.so.1",
    "libGLdispatch.so.0",
    "libGLX.so.0",
    "libgraphite2.so.3",
    "libharfbuzz.so.0",
    "libgthread-2.0.so.0",
    "libICE.so.6",
    "libm.so.6",
    "libmd.so.0",
    "libpcre.so.3",
    "libpcre2-8.so.0",
    "libpthread.so.0",
    "libpng16.so.16",
    "librt.so.1",
    "libSM.so.6",
    "libstdc++.so.6",
    "libX11.so.6",
    "libX11-xcb.so.1",
    "libXau.so.6",
    "libxcb.so.1",
    "libxcb-icccm.so.4",
    "libxcb-image.so.0",
    "libxcb-keysyms.so.1",
    "libxcb-randr.so.0",
    "libxcb-render.so.0",
    "libxcb-render-util.so.0",
    "libxcb-shape.so.0",
    "libxcb-shm.so.0",
    "libxcb-sync.so.1",
    "libxcb-util.so.1",
    "libxcb-xfixes.so.0",
    "libxcb-xinerama.so.0",
    "libxcb-xkb.so.1",
    "libXcomposite.so.1",
    "libXcursor.so.1",
    "libXdamage.so.1",
    "libXdmcp.so.6",
    "libXext.so.6",
    "libXfixes.so.3",
    "libXi.so.6",
    "libxkbcommon.so.0",
    "libxkbcommon-x11.so.0",
    "libXrandr.so.2",
    "libXrender.so.1",
    "libz.so.1",
    "linux-vdso.so.1",
}


LDD_ARROW_LINE_RE = re.compile(r"^\s*(\S+)\s+=>")
LDD_LOADER_LINE_RE = re.compile(r"^\s*/\S*/([^/\s]+)\s+\(")
LDD_DIRECT_LINE_RE = re.compile(r"^\s*(\S+)\s+\(")


def fail(message: str) -> None:
    print(f"error: {message}", file=sys.stderr)
    sys.exit(1)


def parse_library(line: str) -> str:
    if "not found" in line:
        return line.split(None, 1)[0]

    for pattern in (LDD_ARROW_LINE_RE, LDD_LOADER_LINE_RE, LDD_DIRECT_LINE_RE):
        match = pattern.match(line)
        if match:
            return match.group(1)

    fail(f"could not parse ldd line: {line}")


def main() -> int:
    if len(sys.argv) != 2:
        fail(f"usage: {sys.argv[0]} <binary>")

    binary = sys.argv[1]
    if not os.path.isfile(binary) or not os.access(binary, os.X_OK):
        fail(f"binary not found or not executable: {binary}")

    result = subprocess.run(
        ["ldd", binary],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    print(f"==> ldd output: {binary}")
    print(result.stdout, end="")
    print()
    print(f"==> dependency check: {binary}")

    missing_allowed_libraries = []
    missing_unexpected_libraries = []
    unexpected_libraries = []
    matched_allowed_libraries = set()
    for line in result.stdout.splitlines():
        library = parse_library(line)
        matching_allowed_libraries = {
            allowed_library
            for allowed_library in ALLOWED_LIBRARIES
            if library.startswith(allowed_library)
        }
        matched_allowed_libraries.update(matching_allowed_libraries)
        if "not found" in line:
            if matching_allowed_libraries:
                missing_allowed_libraries.append(library)
            else:
                missing_unexpected_libraries.append(library)
        elif not matching_allowed_libraries:
            unexpected_libraries.append(library)

    failed = False
    if missing_unexpected_libraries:
        print(f"error: missing shared dependencies for {binary}:")
        for library in sorted(set(missing_unexpected_libraries)):
            print(f"  {library}")
        failed = True

    if unexpected_libraries:
        print(f"error: unexpected shared dependencies for {binary}:")
        for library in sorted(set(unexpected_libraries)):
            print(f"  {library}")
        failed = True

    if failed:
        return 1

    if missing_allowed_libraries:
        print(f"warning: allowed shared dependencies not installed on check host for {binary}:")
        for library in sorted(set(missing_allowed_libraries)):
            print(f"  {library}")

    unused_allowed_libraries = sorted(ALLOWED_LIBRARIES - matched_allowed_libraries)
    if unused_allowed_libraries:
        print(f"warning: allowed libraries not linked by {binary}:")
        for library in unused_allowed_libraries:
            print(f"  {library}")

    print(f"shared dependencies allowed: {binary}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

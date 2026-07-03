# Optional Patch Hook

Store source patches (`*.patch`) under the tree-specific subdirectories:

- `qt/` for Qt source patches
- `qtwebkit/` for QtWebKit source patches

Behavior in `qt5-static-build-entrypoint.sh`:
1. Apply patches from `patches/qt/` only to the Qt tree.
2. Apply patches from `patches/qtwebkit/` only to the QtWebKit tree.
3. Process patches in lexicographic order within that tree.
4. Fail the build if a patch for that tree is neither applicable nor already applied.

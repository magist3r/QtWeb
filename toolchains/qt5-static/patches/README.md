# Optional Patch Hook

Drop Qt source patches (`*.patch`) in this directory to support legacy build fixes.

Behavior in `build-inside-container.sh`:
1. Run `configure` once.
2. If `Qt WebKit` is not enabled, apply all patches in lexicographic order.
3. Run `configure` again and continue only if WebKit is enabled.

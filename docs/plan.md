# QtWeb Implementation Plan

- Docker-backed `clazy` analysis is planned.
- `clazy` report generation is planned.
- `clazy` validation alongside the existing `clang-tidy` analysis is planned.
- Analyzer cleanup for clear Qt API misuse is planned.
- Analyzer cleanup for unused locals, dead includes, redundant conditionals, and trivial temporary/container inefficiencies is planned.
- Removal of deprecated Qt API usage is planned, including `QRegExp`, `QString::SkipEmptyParts`, `QDesktopWidget`/`QApplication::desktop()`, `qrand()`/`qsrand()`, `Q_ENUMS`, `Q_FOREACH`/`foreach`, and legacy string-based `SIGNAL`/`SLOT` connections.
- Review of `QTextCodec`-based page/source encoding paths is planned before Qt6 migration work.
- Removal or replacement of the remaining `QFtp` download path is planned.
- Qt6 migration planning is constrained to a QtWebKit-preserving path; no QtWebKit code removal or Qt WebEngine replacement is planned.
- QtWebKit compatibility review is planned for the browser surface that currently depends on `QWebView`, `QWebPage`, `QWebFrame`, `QWebSettings`, `QWebHistoryInterface`, `QWebHitTestResult`, and `QWebHistory`.
- Docker base image digest pinning is planned.
- Additional dependency reduction checks for TLS, certificate handling, and module detection are planned before removing more runtime dependencies.
- Normal Docker browser build validation after analyzer cleanup is planned.
- Documentation updates must move completed points from this file to `docs/migration-status.md`.

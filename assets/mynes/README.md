# MyNES Product Binaries

This directory retains only the latest verified MyNES x64/x86 executable pair
and the adjacent editable `mynes.ini` startup configuration. Generated
manifests, build logs and probes stay under ignored `build/`; game ROMs remain
external owner-provided inputs and are never committed.

Delete superseded EXEs when delivering a verified replacement pair, following
[Execution](../../docs/rules/EXECUTION.md). Earlier binaries remain in Git history;
preserve the editable INI and owner-provided snapshot.

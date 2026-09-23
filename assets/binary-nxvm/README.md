# Generated Product Binaries

Each NXVM profile deploys its x64 and x86 EXE beside its `NXVM.ini`, under
`<profile>/`. The INI is the checked-in per-profile product template consumed
by builds and integration tests; executables remain ignored local results.

`build/` is not an executable deployment location; any old task artifacts
there are historical evidence only. A future product uses its own peer root,
for example `assets/binary-mynes/`; it does not write under this NXVM root.

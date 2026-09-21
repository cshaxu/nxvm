# Generated Product Binaries

Each current fixed-profile product deploys its x64 and x86 EXE plus generated
`NXVM.ini` only to `<profile>/`. Those directories are ignored: they
are local build results, not source templates or protected assets.

`assets/sessions/` retains the checked-in INI templates consumed by builds and
integration tests. `build/` is not an executable deployment location; any old
task artifacts there are historical evidence only.

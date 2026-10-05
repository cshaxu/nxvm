# Test ownership

| Directory | Owner and purpose |
| --- | --- |
| [lib](lib/README.md) | Independently reusable Lib tests and fixtures. |
| [common](common/README.md) | Independently reusable neutral Common tests and fake machine. |
| [x86](x86/README.md) | Independently reusable x86 debugger and assembler tests. |
| app | Product configuration, command parsing and key bindings. |
| core | Concrete machine, compatibility host, devices, media and ABI. |
| integration | Product command/worker/frame/snapshot/shutdown chains and package boot. |
| checks | Product source, build and package boundary checks. |

Root CMake registers product tests. Existing CTest names and labels are retained
for compatibility: the historical `unit` label is not a guarantee of isolated
execution. Self-contained tests create disposable media in their build working
directory. Only `runtime_restart_boot_smoke` reads the fixed installed image;
`package_smoke` exercises the matching packaged EXE and owner configuration.
They use non-mutating media modes; tests must not overwrite supplied media/INI.

`integration/machine_fixture.c/h` assembles one SoftPC driver and Common machine
for three worker/boot tests. It owns only these two test objects; callers own
the original machine. Tests call Common directly after assembly. It is not the
neutral fake machine from test/common and is not part of any reusable corpus.
`integration/snapshot_cross_process.cmake` orchestrates disposable save/load
processes using the existing snapshot transaction test.

Configure with `cmake --preset mingw-gcc-x64-release` (or x86), then build with
`cmake --build --preset tests-x64` (or x86). `ctest --preset test-x64` and
`test-x86` exclude the five desktop cases per width and are safe for background
work. `test-desktop-x64` / `test-desktop-x86` require reserved desktop access;
never use an unfiltered invocation while the owner is using the desktop.
List exact cases with `ctest --preset test-x64 -N` or the desktop preset.

The old unregistered direct-slice and Setup diagnostics are retired, not
supported runners. Their source and historical evidence remain in Git; no
registered test was removed. Full ownership and build policy belongs to
[Source Layout](../docs/design/CODING.md).

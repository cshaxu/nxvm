# M5 T539 S63 — TSS I/O-map receiver map

| I/O-map context | Sole receiver | Observed outcome |
| --- | --- | --- |
| 80386 bitmap allows byte IN/OUT | `machine_tss_iomap_port_authorization_smoke.c` | one actual port read and write reach the installed provider |
| 80386 bitmap denies IN or OUT | same | no provider access; the guest #GP delivery handler records the fault |
| 80386 bitmap truncates a word access | same | no provider access; the actual #GP route is observed |
| 80386 and 80286 IOPL bypass cases | same | actual provider accesses remain allowed despite the bitmap denial bit |

The receiver uses real protected entry, `LTR`, TSS bitmap bytes, CPL/IOPL
state and a public Core port provider. Its minimal TSS setup exists only to
exercise the I/O-map authorization boundary; it does not construct a task
transfer or duplicate any S56-S62 task-state receiver.

Final serial repository-only unit suites pass 426/426 on x64 and x86. T344
fixture-shape and registration, Core CPU/PIC authority, VM lifecycle,
documentation governance and `git diff --check` pass. This is
test/CMake/documentation-only work; no EXE rebuild is required.

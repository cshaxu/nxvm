# Architecture Direction

The canonical architecture is [Module Layout](module-layout.md). This
compatibility entry remains so older links resolve.

The controlling direction is a shared NXVM-derived core with two products:
`nxvm.exe` for bootable whole-machine VM use and `ntvdm64.exe` for non-bootable
DOS application running. Microsoft NTVDM component feasibility is an M11 T2
research topic, not a backend. Do not treat this file as a second architecture
specification.

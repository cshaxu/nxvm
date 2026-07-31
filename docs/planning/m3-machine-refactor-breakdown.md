# M3 Machine Refactor Breakdown

## Entry And Common Gate

M3 starts from M2 Architecture Requirements V1, the M1 snapshot branch
`snapshot/m1-baseline-7d54223`, and the M1 FDD/HDD verification record. One
subtask is active at a time. Each subtask creates a record with concrete files,
commands, budget values, expected checkpoints and rollback point before code
changes. No task implements DOS services, a product CLI, or a concrete host
filesystem.

Every completed task builds with GCC/CMake/Ninja, preserves the M1 legacy-boot
regression, records its focused tests, and updates provenance for moved or
substantially derived NXVM units. The M1 raw snapshot remains immutable.

| Task / subtask | Approved decision and implementation result | Focused acceptance | Stop condition |
| --- | --- | --- | --- |
| T1 S1: define Machine V1 | Create project-owned opaque headers, status/stop enums, instance config and profile selector. No source behavior moves yet. | Compile-only ABI consumer; forbidden-dependency include scan. | Any interface needs a DOS or Win32 type. |
| T1 S2: create Machine instance shell | Move or wrap CPU, RAM and port state under one instance without raw state exposure; retain legacy adapter behind it. | CPU/RAM/port deterministic tests; M1 FDD/HDD checkpoints. | Pointer-width, aliasing or ownership defect that changes M1 trace before diagnosis. |
| T2 S1: make lifecycle explicit | Implement initialize/reset/run/request-stop/destroy state machine and finite run budgets on the Machine thread. | Legal/illegal transition tests, reset-vector checkpoint, budget/request/fault stop reason tests. | A transition requires platform thread access. |
| T2 S2: add neutral trace boundary | Implement disabled-by-default V1 sink at run and device boundaries with bounded buffering. | Null-sink equivalence; deterministic event schema test; no external reference adapter. | Trace alters M1 checkpoint or becomes required at runtime. |
| T3 S1: isolate legacy boot profile | Move BIOS, CMOS, DMA, FDC/FDD, HDC/HDD and VADP activation behind `legacy_boot`; retain exact fixture path only there. | Recorded fixture hash, insertion/connection marker, reset and DOS `INT 21h` checkpoints under M1 budgets. | Missing fixture or new disk/BIOS behavior requirement. |
| T3 S2: create DOS-minimal device profile | Compose CPU/RAM/ports with declared PIC/PIT, keyboard and text-display devices, without BIOS or disk devices. | Profile inventory test and deterministic reset/IRQ/input/text-snapshot tests. | A DOS requirement is unspecified by M4. |
| T4 S1: isolate platform presentation | Replace direct platform access to device globals with adapter-owned input queues and display snapshots; keep Console/window developer behavior only. | Mock platform input/snapshot test; source scan proves no platform include of Machine internals. | Product display/CLI behavior is needed to decide the adapter. |
| T4 S2: move developer debugging boundary | Make debugger inspection/commands operate through synchronized Machine debug APIs; preserve no current product-debug claim. | paused-step/continue/read-register test; no direct legacy global access. | Required debugger command semantics belong to M6. |
| T5 S1: compose and close M3 | Runtime owns Machine/profile/adapters; remove obsolete startup coupling and snapshot verified result. | Full GCC build, all focused tests, M1 FDD/HDD checkpoints, dependency scan, immutable M3 snapshot. | Any M1 regression lacks a focused explanation or M4 contract is needed. |

## Exit Evidence

M3 closes only after the M2 V1 ABI is implemented and documented, `legacy_boot`
reproduces M1 checkpoints, `dos_minimal` has deterministic no-media tests, and
no Machine source depends on DOS or Windows. The verified main commit receives
an immutable M3 snapshot before M4 begins. A runnable developer artifact follows
the local artifact policy only when a completed M3 subtask produces one.

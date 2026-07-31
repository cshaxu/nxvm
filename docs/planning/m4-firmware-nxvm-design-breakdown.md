# M4 Firmware And nxvm Product Design Breakdown

M4 is a design milestone. It does not move firmware or add `nxvm.exe`; it
produces the bounded M5 implementation breakdown.

| Task | Design result | Evidence | Stop condition |
| --- | --- | --- | --- |
| T1: assign retained full-PC ownership | Map BIOS/POST/ROM, CMOS, DMA, storage, VADP, and legacy adapter seams to `firmware`, `core`, `platform`, or `products/nxvm`. | Versioned ownership map and no-cross-boundary review. | Any DOS backend or product CLI behavior is required. |
| T2: define firmware service contracts | Specify reset/POST, ROM loading, BIOS INT service registry, failure model, and host capability use. | C contract sketch and fixture regression mapping. | A service needs a concrete platform API in firmware. |
| T3: define nxvm product surface | Specify boot-media policy, Console/debug entry points, display/input expectations, and artifact identity. | CLI/Console state tables and compatibility matrix. | ntvdm64 product behavior or M8 CLI policy is required. |
| T4: define M5 verification and breakdown | Freeze fixture, watchdog, smoke, and artifact rules; decompose M5 into bounded subtasks. | Approved M5 breakdown and Go/No-go review. | An M1 regression lacks a focused owner. |

M4 starts from `docs/planning/m4-design-inputs.md` and the immutable M3
snapshot. Its implementation work begins only after M4 closes.

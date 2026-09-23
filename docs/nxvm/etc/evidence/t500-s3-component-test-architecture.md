# T500 S3 Component Test Architecture

## One Test Tree And Two Routes

T500's completed layout is:

```text
test/
  core/{utils,machine/{cpu,memory,controllers,lifecycle},platform,product}/
  vm/{machine,composition/{session,profiles},platform,product,profile}/
  vdm/{machine,composition,platform,product}/
  support/
  integration/{firmware,dos,hdd}/
```

`test/support/` has only fixture/setup helpers. It has no test registration,
product path, mutable machine state or adapter authority. A test with a real
cross-owner contract lives at the nearest composition boundary, not in a
second generic `machine` bucket.

CTest remains the only membership and execution owner. The final labels are
`unit` for every repository-only check and `integration` for every
owner-managed external-input scenario; a check has exactly one route. `smoke`
and `current-gate` are retired route names, not compatibility labels. Focused
selection remains packet-local and never becomes a label or list.

## Inventory Consumption

| S2 class | Future module and route | S4 action |
| --- | --- | --- |
| 285 registered repository-only checks | `test/core/...`, `test/vm/...` or `test/vdm/...`; `unit` | Move by linked production owner and register once. |
| 13 registered external scenarios | `test/integration/{firmware,dos,hdd}/`; `integration` | Move and preserve explicit owner-provided input arguments. |
| 2 shared-source EGA DOS scenarios | `test/integration/dos/ega-planar-dos.c` and `ega-rom-int10-dos.c`, or one proved replacement | Split because their compile-time programs and observables differ; do not retain two targets built from one source. |
| 2 generated-result checks | `test/core/machine/cpu/...`; `unit` | Write derived result only in the build tree, compare it to tracked oracle/evidence, and never rewrite tracked JSON. |
| 34 unregistered repository-only targets | Matching `test/<owner>/...`; decision required per target | Register only after its no-input contract passes, or delete it with an owner/coverage proof. |
| 6 unregistered BYOB/Windows/fdisk probes | `test/integration/...`; `integration` | Keep opt-in and explicitly asset-gated; no unit fallback. |
| 3 platform-conditional targets | `test/vm/platform/<platform>/...`; `unit` where supported | Register only on its supported platform with an explicit unavailable/skip condition elsewhere. |

## Owner Modules

- `test/core/machine/cpu/` owns five CPU-profile instruction function, flags,
  exception, retirement and timing-ledger checks.
- `test/core/machine/memory/` owns checked memory, ROM, mapping, transaction,
  bus and arbitration checks; `controllers/` owns PIC, DMA, PIT, RTC, KBC,
  FDC, HDC and VADP behavior/timing checks.
- `test/core/machine/lifecycle/` owns reset, scheduler, deadline, HLT and
  machine reconfiguration checks. `test/core/{platform,product,utils}/` maps
  only the corresponding production module.
- `test/vm/composition/` owns Core--VM construction and profile composition;
  `test/vm/machine/`, `platform/`, `product/` and `profile/` map the matching
  VM boundary. This is the large repository-only composition layer, not
  external integration.
- `test/integration/firmware/` owns BIOS/option-ROM scenarios;
  `dos/` owns supplied DOS boot/media checkpoints; `hdd/` owns the Windows 3.1
  installer and HDD checkpoints. These tests consume paths at runtime and
  retain neither media nor machine-local paths in evidence.

## Migration Order And Resource Policy

S4 first adds the single `test/` CMake registration surface and moves helper
code, then moves route-complete Core, VM/VDM and integration batches. Each old
`tests/` path is deleted in the same batch that adds its replacement; no
forwarding CMake list or dual-tree period is allowed. The existing 7 host-thread
tests retain a narrow serial/resource declaration until S5 proves isolation.
The three console tests retain owned build-tree working directories. All other
serialization and working-directory assumptions are re-proven rather than
copied.

S5 runs clean and incremental parallel unit routes, then the owner-managed
integration route. It may relax an isolation only with a reproducer; it never
serializes the whole suite to mask a collision.

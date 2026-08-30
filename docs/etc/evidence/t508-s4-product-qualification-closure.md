# T508 S4 Product Qualification Closure

`test/vm/machine/vm_timing_qualification_smoke.c` is one table-driven,
owner-local proof over IBM 5160 XT, IBM 5170 Model 339, DeskPro Model 40 and
default PC/AT. Each row materializes its existing frozen profile configuration,
creates Core through the existing plan-copy route, enables DMA2, and observes
a pending request. Every row has a nonzero copied DMA ratio and yields
`CORE_MACHINE_TIME_PROGRESS_DEADLINE` with a valid deadline; none yields the
bounded L1 compatibility disposition. The test does not expose a controller
setter or feed a guest tick from VM.

The first test revision used direct `core_machine_create`; it correctly
observed L1 because that bypasses the production plan-copy seam. The retained
test uses `core_machine_plan_create` and `core_machine_create_from_plan`, so it
guards the actual one-way profile-to-Core construction boundary instead of
creating a parallel test route.

S4 verification completed on 2026-08-30:

| Check | Result |
| --- | --- |
| Focused timing matrix | `unit.vm-timing-qualification-smoke`: pass. |
| Complete repository-only unit | 313/313 pass with `ctest -L unit -j 8` in 13.60 seconds. |
| External-asset integration | 20/20 pass with `ctest -L integration -j 4` in 17.43 seconds. |
| Release | `nxvm_0_5_0508.exe`, 1,242,996 bytes, SHA-256 `98B8BAE5DD009E1A2954A7698545C37E044365999C023440B52E79A1CE65E9D8`; the `-O3`, `-DNDEBUG`, `--strip-debug` route has no `.debug` section. |

An initial integration replay ran stale statically-linked integration
executables after their Core/VM libraries had been rebuilt for the new unit.
The six immediately failing executables were relinked without source changes;
their focused replays and the complete parallel 20/20 replay passed. This is
a build freshness correction, not a timing or product regression.

The two retained unqualified boundaries are deliberate: generic default-PC/AT
ATA service has no portable sourced duration, and Model-40 elapsed ticks are
not proven CPU cycles. Both remain owner-local immediate or unqualified
boundaries; neither is an active built-in L1 fallback.

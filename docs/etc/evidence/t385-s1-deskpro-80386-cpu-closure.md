# T385 S1: DeskPro Model-40 80386DX-16 CPU Closure

`M5:T385:S1:DESKPRO-CPU-LEDGER:OK`

## Scope And Model Boundary

This is the CPU/state portion of the selected original 1986 Compaq DeskPro
386 Model 40 / DeskPro 386/16.  T384's Compaq primary-source ledger selects an
80386DX at 16 MHz; it does not create a second CPU architecture, decoder,
page walker, exception route or retirement publisher.  The 16 MHz source and
its board relationship are profile facts, while physical clocking, waits and
board service placement remain the later DeskPro board-timing receiver.

The owner-selected external Rev-E ROM is not executed or mapped here.  The
Compaq EGA personality, keyboard, FDC/fixed-disk controller, memory/ROM
composition, PIC/DMA/PIT/RTC/NMI/reset binding and every board route remain
the later selected-device or board-timing work.  No runnable DeskPro or L3
claim follows from this CPU ledger.

## Shared 80386DX Reconciliation

The current source still has one `CORE_MACHINE_CPU_PROFILE_80386` CPU profile,
one decode/execute construction, one translation/preflight/commit owner and
one machine successful-retirement publisher.  The generic `default-pc-at`
descriptor can select that CPU profile, but no DeskPro descriptor or CPU
execution fork exists.  This is the required shared-core result: the selected
Model-40 consumes only the CPU semantics whose Intel-defined form/state
boundary was already closed by T340--T342.

| Model-40 CPU row | Current owner and current evidence | T385 disposition |
| --- | --- | --- |
| 80386-only width, prefix, FS/GS, `LOCK`, ordinary and non-privileged `0F` forms | One profile gate, prefix/width/decode route and existing T340 form matrix. | Complete shared CPU semantics; no DeskPro-local form variant exists. |
| Protection, descriptor/table, CR/DR/TR and privilege state | Existing special-register/table/cache owners and T341 S2 evidence. | Complete shared CPU semantics; board/device state is not inferred. |
| Paging, faults and successful publication | One page-walk preflight/commit route and current `core_machine_instruction_cost` / `core_machine_publish_elapsed_ticks` retirement path. | Complete at the CPU semantic boundary. Persistent cache/test-register model remains explicitly outside 80386. |
| VM86, task/TSS and exception/interrupt delivery | Existing serializers, task plan/commit, delivery owners and T341 S3/S4 evidence. | Complete shared CPU semantics. VME/PVI remains outside 80386. |
| Debug, `#DB`, vector 1 and rejected forms | Existing debug matcher/delivery owners and T341 S4 evidence. | Complete shared CPU semantics; later debug extensions are not claimed. |
| x87, physical 16 MHz behavior, waits/arbitration, device service, firmware execution and Windows | T384 transfer, queue and TODO boundaries. | Explicitly transferred: x87 stays unselected; physical timing goes to DeskPro board timing; all machine/device work goes to the DeskPro functional/timing candidates. |

The T340/T341/T342 source-owner sweeps and their Intel-constrained matrices
remain valid on the current graph: no parallel 80386 executor, page walk,
descriptor writer, task materializer, vector-1 route or retirement publisher
was found.  No CPU repair is needed or admitted in this task.

## Current-Source Verification

Using MSYS2 GCC 16.2.0, a clean `build/t385-s1-msys` configuration built the
complete set of 251 targets registered under the `current-gate` CTest label.
`ctest --output-on-failure -L current-gate --parallel 4` passed **251/251**.
The focused 80386 profile, paging, debug, VM86 delivery/IRET, task-switch and
instruction-timing targets are included in that result.

An ordinary `all` build separately stops at the pre-existing, non-current
`vm-request-bridge-smoke` fixture because it names the removed
`VM_PLATFORM_REQUEST_KEY_PRESS` / `key_press` API.  This is the already
recorded [request-bridge TODO](../../states/TODO.md#architecture-and-portability-debt),
not a T385 CPU regression and not an exclusion from current-gate.  T385 does
not repair, hide or reclassify that deferred platform-test issue.

## Transfer

T385 closes only the selected Model-40 80386DX-16 CPU/state ledger.  The next
DeskPro selected-device functional candidate owns the first runnable
composition, including memory/ROM, input, storage, platform topology and the
small Compaq EGA personality.  The following board candidate owns the D4 bus,
physical clock, availability/wait, device-service and monitor timing.  The
final DeskPro audit alone decides Model-40 L3; IBM EGA remains a
current-product closure receiver.

# T349 S2: PC/AT PIC Command, Priority, and Cascade

## Reconciled mechanism

`src/core/machine/pic.c` now has one controller-owned selection path:

1. `core_machine_pic_select_controller` scans a controller's unmasked IRR in
   current priority order and admits only a request that is higher priority
   than the current ISR boundary.
2. `core_machine_pic_select` applies that same rule to master candidates and
   only selects master IR2 when the slave can actually raise its interrupt
   output. A blocked slave request therefore does not suppress an eligible
   lower-priority master request.
3. `scan`, `peek`, and `get` all use this selection. `get` alone moves the
   selected IRR request into ISR, applies AEOI, and refreshes the virtual
   master cascade request after a slave acknowledgement.

This replaces the prior divergent construction in which scan tested a single
master top request, peek/get independently reselected it, and a pending but
ISR-blocked slave request could make scan false without permitting a lower
master request. It preserves real master/slave layout and the existing CPU
delivery boundary.

## Reproduced and repaired defects

| Reproducer | Prior behavior | Repair and sweep |
| --- | --- | --- |
| Write ICW1 after an acknowledged, masked request. | Stale IRR, IMR, ISR, ICW/OCW and rotation state survived reinitialization. | `core_machine_pic_begin_initialization` resets controller command/priority state before ICW2. The smoke proves cleared state and an empty acknowledgement. |
| Deliver slave IRQ14, EOI only master IR2, then request blocked slave IRQ15 and master IRQ3. | The old master-top cascade check selected blocked IR2, then reported no interrupt instead of considering IRQ3. | Shared selection skips a cascade candidate whose slave cannot interrupt and selects IRQ3; after specific slave EOI, IRQ15 is delivered. Scan/peek/get share the same decision. |
| Acknowledge with no eligible request. | The old get path could manufacture a vector and mutate ISR from the sentinel priority id. | Shared selection returns zero with no ISR mutation. |

## Focused proof

`tests/machine/core_machine_pic_command_priority_smoke.c` is the owner-local
controller proof and emits `M5:T349:S2:PIC-COMMAND-PRIORITY:OK`. It covers:

- ICW restart and port-visible IRR/ISR reads;
- IMR, fixed priority, specific and non-specific EOI;
- set-priority and rotate-on-non-specific-EOI behavior;
- AEOI ISR clearing without a software EOI;
- slave IRQ14/15 cascade, masked slave withdrawal, blocked slave plus lower
  master arbitration, and exact scan/peek/get vectors.

The similar-issue sweep covered every `irr`, `imr`, `isr`, `irx`, `icw*`,
`ocw2`, scan/peek/get, and cascade refresh use in `pic.c`, plus the sole CPU
consumer in `cpu_instructions.c`. The same controller structure serves master
and slave; no parallel selection path remains. S3 retains OCW3 poll, special
mask, and SFNM. S4 retains spurious IRQ and device-source lifecycle/reset
composition.

## Build and regression result

The target is registered once in `PROJECT_CURRENT_SMOKE_TARGETS`. The T345
owner-test strict inventory therefore rises from 122 to 123 exact pure test
targets; its fixed expected count was updated, while its three mixed targets
remain unchanged. The target compiles under the existing target-local strict
GCC policy.

Focused PIC producer and delivery regressions passed: PIC lifecycle, the new
command/priority smoke, hardware delivery, KBC/AUX, RTC, FDC, HDC, controller
authority, DMA/RTC authority, CMOS/RTC port, and VM HDC/FDC port smokes. The
complete current-gate and documentation-governance results are recorded with
the S2 acceptance commit.

# M5 T540 S64 Public Interface Intake

## Measured coupling

`src/app-nxvm/devices/machine_interface.h` is **694 lines** and has **174**
direct source/test includers. Its single public surface currently mixes:

- Neutral CPU, transaction, guest-time, memory, execution and observation
  contracts.
- A 152-file `core_machine_config`/clock-plan consumer group whose one
  configuration contains both neutral retirement/transaction inputs and
  board PIC/PIT/DMA/KBC/XT wiring inputs.
- A 48-file board topology/configuration group: display, RTC/CMOS, planar
  parity, D4, absent memory, DMA, FDC/HDC and frozen plan topology.
- A 47-file board operation group: device input, display snapshot,
  configuration, fault/observation and keyboard ingress.

The counts overlap; they are direct source/test reference inventories, not
an estimate of lines to edit. `machine.c` still makes six direct board
create/reset/finalize/NMI calls. S63 removed concrete chips from the private
header, but neither this public contract nor the Core implementation is yet
independently buildable without App board headers. Moving all of these at
once would conflate configuration ABI, board API, execution handoff and
physical relocation, and would touch too many consumers for one safe S.

## Linear receivers

S64 makes **no source change** and assigns the remaining work:

1. **S65:** Separate neutral Core construction values from board-selected
   configuration inside the existing single validated/frozen plan. Preserve
   one create/rollback route; do not mirror configuration at runtime.
2. **S66:** Move board topology and observation value definitions to one
   board public interface, and correct direct consumers. Keep neutral
   execution/time values in the Core interface.
3. **S67:** Move board-specific operations to that board interface and
   correct only their real callers; no forwarding compatibility layer.
4. **S68:** Remove `machine.c`'s six direct board lifecycle/signal calls by
   one bounded composition handoff; Core must not acquire a second board
   owner or another create/reset path.
5. **S69:** Prove a strict, independently compiled neutral Core target using
   the same production source and a minimal synthetic board binding. Do
   not call a mere include smoke an independent build.
6. **S70:** Physically move only the proven neutral source/tests to
   `src/x86/core` and `test/x86/core`, reconnect NXVM, delete old copies
   and verify both widths and four fixed profiles.
7. **S71 onward:** Receive individually audited IBM-PC common/AT/XT board
   extraction; final numbering follows source evidence.

S65-S70 are prospective bounded numeric receivers. Each can subdivide its
unaccepted work into later linear S numbers if direct source evidence shows
it is still too large. The former S65 physical move was never executed;
this measured split supersedes its prospective number.

## Verification

S64 changes no code, test, build, firmware, INI, media or EXE input. The
unchanged S63 baseline passed complete repository-only x64 and x86 unit
suites, **469/469** each. S63's both specialized gate sets and eight
single-run boots remain the executable baseline. Documentation governance
and diff checks cover this intake. There is no independent neutral Core
build claim until S69.

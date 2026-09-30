# T539 S36: FLAGS Test Owner Migration

## Source-to-owner receiving map

| Retired source / case family | Sole receiver |
| --- | --- |
| `core_machine_direct_flags_smoke.c`: five direct-FLAGS opcodes across the original CPU-profile, prefix, VM86 and real-mode identity matrices | `cpu_direct_flags_smoke.c` |
| Same source: protected guest bootstrap and real PIC IRQ | `core_machine_direct_flags_board_smoke.c` |
| `core_machine_lahf_sahf_smoke.c`: LAHF/SAHF profile, FLAGS/AH, prefix, VM86 and checksum sequence | `cpu_lahf_sahf_smoke.c` |
| Same source: protected guest bootstrap and real PIC IRQ | `core_machine_lahf_sahf_board_smoke.c` |
| `core_machine_pushf_popf_s47_smoke.c`: five-profile PUSHF/POPF matrix, 386 attribute forms, legacy-prefix and LOCK rejections | `cpu_pushf_popf_smoke.c` |
| Its included `core_machine_pushf_popf_smoke.c`: four real-mode width forms, protected IOPL cases, VM86 successful/GP cases and two synthetic SS-cache faults | `cpu_pushf_popf_smoke.c` |
| S47 source: real PIC IRQ around PUSHF and POPF | `core_machine_pushf_popf_board_smoke.c` |
| Protected guest POPFD stack-limit terminal fault | `core_machine_pushf_popf_board_smoke.c`, using guest LGDT/far transfer/SS reload and public machine state |

The two original S47/S21 programs are retired together. The S47 `#include`
of the S21 `.c` file is gone. CPU tests link only `x86-cpu`; board tests use
public machine operations, copied CPU snapshots and real PIC or guest
descriptor activity. Two shared test-only board fixtures factor the identical
protected bootstrap and IRQ wiring used by the direct-FLAGS/LAHF cases;
neither adds a production path or private CPU accessor.

The protected synthetic SS-cache rejection remains at the CPU owner because
an invalid non-writable loaded SS is not guest-reachable. The additional
board POPFD case uses a valid guest-loaded SS with an out-of-range ESP. Its
terminal exception can disturb the guest stack while attempting delivery;
the CPU-local test owns the original no-write/rollback assertion, while the
board test asserts the real fault and unchanged architectural CPU snapshot.

## Scope and verification

- No production CPU instruction, timing, machine, firmware, profile or ROM
  implementation changed. No product executable input changed.
- The 15 code/test/build/gate paths add 1,562 and remove 1,982 lines, net
  minus 420. The inventory and this report are the only documentation paths.
- CMake's T317/T332 original mixed-owner inventory moves from 46 to 42;
  six replacement targets are registered as unit tests, with the three CPU
  targets compiled warnings-as-errors. T337 marks the CPU-local #UD terminal
  disposition instead of pretending a real board IVT delivery.
- The CPU bus negative suite contains 173 deliberate source mutations and
  exceeded its former 60-second deadline under four-way host contention;
  its limit is 180 seconds. It passed alone in 41.69 seconds and in the
  complete x64 suite in 33.79 seconds. This changes only test timeout, not
  the negative checks or production behavior.
- Complete x64 and x86 repository-only unit suites: 406/406 passed on each
  width after the final board receiver and cleanup. Both widths' current
  specialized gate aggregate passed, including T317/T332/T337/T344,
  CPU/PIC authority and the direct-compilation matrix (405 x64 / 404 x86
  rows). All six unchanged Shared source/test manifests passed explicitly.

The actual implementation commit and independent review remain pending. This
document does not itself accept S36.

# M5 T539 S74 -- Remaining Consumer And Fixture Map

## Boundary

S74 closes the four residual test-only CPU receiver inputs from the T539 work
plan.  It changes names and test registration only; production CPU behavior,
public API, Shared sources, firmware, assets, INI inputs and executable inputs
are unchanged.

| Former input | Sole successor |
| --- | --- |
| `cpu_profile_gate_smoke.c` | `machine_cpu_profile_gate_smoke.c` |
| `fpu_escape_smoke.c` | `machine_fpu_escape_smoke.c` |
| `core_machine_fpu_interface_s65_smoke.c` | `machine_fpu_interface_s65_smoke.c` |
| `support/core_machine_cpu_fixture.h` | `support/machine_cpu_fixture.h` |

The fixture remains private to the NXVM Core-machine unit corpus.  Its 31
test-only include consumers now use the one successor name.  There is no
compatibility header, duplicate fixture, production include, or second test
path.

## Gate Reconciliation

The CPU-boundary negative source injects the renamed fixture deliberately and
continues to reject it.  The lifecycle and historical-shape checks name the
successor fixture/receivers, so a later reintroduction of the old mixed path
cannot pass merely because a verifier retained an obsolete spelling.

## Verification

- All three successor receivers passed focused x86 and x64 execution.
- All 31 affected consumer targets were rebuilt for x86 and x64.  They were
  rebuilt in sequential groups because concurrent Windows link jobs briefly
  contended for generated test executables; this is not a source failure.
- `unit.cpu-bus-boundary-negative` passed on both widths, reporting its
  baseline plus 72 negative controls, five board controls, and 96 migrated
  board controls.
- Final repository-only unit suites passed at the final source/CMake state:
  x86 `426/426` in 100.66 seconds; x64 `426/426` in 99.72 seconds.
- On both widths, the Types, T344 registration and historical-shape, T332
  lifecycle, VM lifecycle, CPU/PIC authority, T388 lexical/physical and
  documentation-governance gates passed.  `git diff --check` passed.

No executable rebuild is required: this packet changes no product executable
input.  MyNES files and binaries were not touched.

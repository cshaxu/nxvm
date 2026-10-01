# M5 T539 S98 timing and catalog boundary audit

## Shared owner

`src/x86/devices/cpu` remains the sole CPU timing implementation owner. No
second timing algorithm, formula table, or manifest interpretation is created
for the migration.

## NXVM runner owners

The following test-side receivers remain NXVM because their observable result
is a machine/profile/board contract rather than a CPU-only contract:

- `machine_8086_timing_manifest_runner.c` composes App device support and
  machine interfaces, and writes 8086/8088 result plus decoder artifacts.
- `machine_80186_timing_manifest_runner.c`,
  `machine_80286_timing_manifest_runner.c`, and
  `machine_80386_timing_manifest_runner.c` construct a `core-machine`, load
  generated catalog inputs, publish retirement observations, and emit their
  profile result artifacts.
- `machine_instruction_timing_ledger_smoke.c` and the 8086, 80186 and 80286
  ledger receivers use the NXVM machine fixture and validate board-visible
  timing context.
- `cpu-timing-manifest-catalog` is NXVM build composition: it invokes the
  NXVM exporter against the NXVM timing manifests and supplies the generated
  include to the machine runners.

Moving any of these would require a second machine fixture, a second result
publication route, or an App dependency in Shared. S98 therefore makes no
source move: this is the correct one-owner allocation, not an unimplemented
CPU extraction.

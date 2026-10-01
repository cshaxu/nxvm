# M5 T539 S90 CPU paging receiver

## Scope

S90 moves the seven CPU-only 80186 LGDT-gate, paging-control, INVLPG and CR0
mutable-control helpers from the NXVM execution-context smoke to the sole
Shared `cpu_execution_paging` receiver. The fixture uses an artificial
unavailable IDT and validates CPU #UD behavior; it does not exercise public
physical paging, PIC or board wiring.

The NXVM residual retains only debug and S91 protected fault/event helpers.
No production source, public API, firmware, asset, INI or executable input
changed.

## Verification

- Focused Shared and residual x64/x86 executables: passed.
- T332 fixture lifecycle: passed on both widths (44 owners).
- CPU/PIC authority, Shared test manifest/corpus, documentation governance and
  `git diff --check`: passed.
- Detached repository-only unit suites: x64 **443/443**, exit `0`, 76.48 s;
  x86 **443/443**, exit `0`, 76.43 s.

## Artifact determination

This is test/CMake/documentation-only ownership work. No executable rebuild is
required.

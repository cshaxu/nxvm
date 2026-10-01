# M5 T539 S96 LOCK and immediate-IMUL receiver acceptance

## Scope

S96 moves the direct CPU-only `cpu_legacy_lock_s1_smoke.c` and
`cpu_imul_immediate_s56_smoke.c` receivers into `test/x86/devices/cpu`. Both
reuse the existing Shared instruction fixture and retain their CPU encoding
and exception semantics.

`core_machine_legacy_lock_s1_smoke.c` remains NXVM: it owns real port-provider
and protected IOPL board wiring. This migration adds neither a broad LOCK
compatibility branch nor a production/API change.

## Verification

- Focused Shared and retained-board receivers: x64 **3/3**, x86 **3/3**.
- T317 test-type vocabulary, CPU/PIC authority, documentation governance,
  Shared manifest, Shared corpus and `git diff --check`: passed.
- Detached repository-only full units: x64 **462/462**, x86 **462/462**.

No executable rebuild is required for this test/CMake/documentation-only
ownership migration.

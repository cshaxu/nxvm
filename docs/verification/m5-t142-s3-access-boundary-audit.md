# M5 T142 S3: Access-Boundary Audit

## Confirmed Structure

- At S3, `vm_session` owned one `core_machine*` and an embedded
  `vm_composition_machine_access`. S5 supersedes that temporary record: both
  the access record and its forwarding source are deleted.
- At S3, `machine_access.c` was the only VM source that called core borrow
  APIs. S5 moves that responsibility directly to root composition and records
  the resulting composition-only source scan.
- VM profile/device composition receives only the necessary borrowed
  capability at bind time. Core owns all executor and shared-device lifecycle.
- The debugger target obtains CPU/RAM/port/execution/instruction capabilities
  through target-local helpers backed by the composition access record. The
  product Console continues to use only `core_product_debug_target`.

## Source Audit

Current source scans are empty for all prohibited active-path patterns:

```text
CORE_MACHINE_PROFILE|core_machine_enable_executor|shared_devices_enabled
vm_session raw CPU/RAM/port/PIC/PIT/DMA/KBC/VADP member access
core_machine_prepare_executor_* below src/vm or src/vdm
#include "vm/" or #include "vdm/" below src/core
_Thread_local or thread_local execution-selection state
legacy platform start_machine / detached pthread lifecycle
```

The only platform resource releases are backend finalizers, plus pre-worker
allocation-failure lease release. S5 replaces the former VM access record with
direct core-owned borrows confined to `src/vm/composition/`.

## Runtime Gates

The final task artifact and complete focused gate suite are recorded in the
T138--T142 contract audit. Windows GCC produced
`build/output/nxvm_0_5_0142.exe` with banner `0.5.0142` and SHA-256
`A8617DA8E203D048DEE0BE907649C92B91815CE1AFF11DEFF7E18606B4895A5C`
before S5. S5's current artifact hash is recorded in its own verification
record.

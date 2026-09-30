# T539 S48: call-gate privilege-entry migration

## Result boundary

S48 retires the 776-line private
`core_machine_call_gate_privilege_entry_smoke.c` receiver.  Its replacement
constructs a real 80386 protected-mode transition through the public machine
contract: a reset guest installs GDT/IDT/TSS bytes, enters ring 0, executes
`LTR` and `IRET` to ring 3, then executes a far call through a 32-bit call
gate.  It observes the result only through public snapshots, physical-memory
reads and the public delivered-exception diagnostic.

The shared bootstrap now has one explicit default-32 variant.  The D/B bits
are present in the original GDT image before the protected-mode far jump; its
ring-0-to-ring-3 sequence is encoded for the resulting default-32 code
segment.  This replaces the rejected approach of changing a descriptor after
the 16-bit bootstrap and then executing bytecode whose operand width no longer
matched the loaded segment cache.

No production source, public ABI, Shared component, firmware, INI, media or
executable input changes.

## Receiver map

| Retired private family | Public receiver | Preserved architectural behavior |
| --- | --- | --- |
| `cg_test_success` | `s48_outer_parameter_copy(0)` | Ring-3 to ring-0 32-bit call-gate transition, return frame and halt target. |
| Parameter-copy rows | `s48_outer_parameter_copy(2)` | Ordered 32-bit parameter copy and outer-stack frame shape. |
| DPL failure | `s48_gate_rejection(call-gate-32, 0)` | Gate DPL rejection and #GP delivery through the real IDT gate. |
| Gate-type failure | `s48_gate_rejection(call-gate-16, 0)` | Invalid 32-bit call-gate use rejects through the same #GP path. |
| Target-descriptor failure | `s48_gate_rejection(call-gate-32, nonzero)` | A call gate targeting the TSS selector rejects without corrupting user state. |
| Cached SS/ESP/TR corruption, forced delivery failures and shutdown flag rows | Retired | These mutate implementation caches or execution-control state which a guest cannot construct; no mutable test seam is introduced. |

`core_machine_call_gate_smoke.c` is intentionally unchanged: its direct
includer is the 80286 timing-manifest runner, and S57 owns that complete
timing closure.  Retiring it here would create a second timing route.

## Similar-issue sweep

The sweep searched every tracked Core-device test source and support header
for `executor_cpu`, `executor_memory`, `t_cpu` and `shutdown_requested`.
The new S48 board receiver and its public bootstrap contain none.  Remaining
matches are CPU-local fixtures or the retained timing manifests, including
`protected_16_timing_fixture.c`; they are assigned to S49--S61, not duplicated
in this receiver.  The T344 historical-fixture gate now classifies the six
public protected bootstrap consumers as one shared construction family and
reduces the direct-constructor inventory from 121 to 120.

## Verification

On 2026-09-30, the new `core-machine-call-gate-privilege-entry-smoke` target
emitted:

```
M5:T539:S48:CALL-GATE-PRIVILEGE-ENTRY:OK
```

It built and ran on both x64 and x86.  Complete repository-only unit suites
passed 416/416 on each width.  Both widths also passed the current specialized
gate aggregate, including the T344 protected-bootstrap and direct-constructor
classification.  Documentation governance and `git diff --check` passed.

# T488 S4 IBM 5160 8259A One-Owner Reconciliation

`M5:T488:S4:8259A-ONE-OWNER:ACCEPTED`

This is the required complete-batch reconciliation, not a PIC symptom repair.
It consumes the 21 frozen rows in T488 List 1 and T488 List 2.

| Frozen batch | Retained production route | Proof and disposition |
| --- | --- | --- |
| `PIC-R1`--`PIC-R5`, `PIC-F1`--`PIC-F6`, `PIC-F8`, `PIC-T1`--`PIC-T3`, `PIC-T5` | `src/core/machine/pic.c` owns PIC state, ICW/OCW commands, poll, priority, IRQ source binding and logical acknowledge. `src/core/machine/cpu_instructions.c` consumes the one `core_machine_pic_get_interrupt` result. | The retained command/priority, OCW3, lifecycle, IRQ-lifecycle and phase smokes pass. No VM parser, dispatch path or second PIC state exists. |
| `PIC-F7`, `PIC-T4`, `PIC-XT1` | `src/vm/profile/xt/xt_5160_268.c` freezes `CORE_MACHINE_PIC_TOPOLOGY_SINGLE`; `src/core/machine/machine.c` passes that immutable construction value to `core_machine_pic_initialize`. | `pic.c` always registers `20h` and `21h`, and registers `A0h`/`A1h` only for the cascaded value. `tests/core/core_machine_plan_smoke.c` asserts `20h` exists and `A0h` is absent for a single topology. |
| `PIC-XT2`, `PIC-XT3` | Device producers use `core_machine_pic_irq_source`; Core binds PIT IRQ0 and CPU delivery to the same master/slave object pair. The XT PPI/NMI route is a separate unit and does not create a PIC owner. | `core_machine_pic_get_interrupt` is the sole logical acknowledge and uses the documented IRQ7 no-request result. `vm-xt-5160-268-profile-smoke` confirms the immutable XT configuration and one constructed Core route. |

## Verification

The rebuilt focused executables passed:

- `core-machine-pic-irq-lifecycle-smoke` - `M5:T216:S1:PIC-IRQ-LIFECYCLE:OK`
- `core-machine-pic-command-priority-smoke` - `M5:T349:S2:PIC-COMMAND-PRIORITY:OK`
- `core-machine-pic-ocw3-smoke` - `M5:T349:S3:PIC-OCW3:OK`
- `core-machine-pic-lifecycle-s4-smoke` - `M5:T349:S4:PIC-LIFECYCLE:OK`
- `core-machine-pic-phase-s2-smoke` - `M5:T456:S2:PIC-PHASE:OK`
- `vm-xt-5160-268-profile-smoke` - all eight retained XT markers pass, including
  `M5:T484:S3:XT-FIXED-PROFILE:OK` and `M5:T484:S5:XT-B2-SHARED-TOPOLOGY:OK`.

The source-to-port sweep also confirms the construction test rejects the
secondary PIC read port `A0h` for the selected single topology. The only
retained timing boundary is unchanged: the manual defines causal and two-INTA
order, but no Core elapsed placement; that is L2. Electrical waveforms remain
L4/out of scope. No source, production code, ABI, test or artifact changed;
the correct complete implementation batch is empty.

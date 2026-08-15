# T360 S5: Four-Profile Timing Source Closure Audit

## Closure decision

T360 closes the bounded source-reconciliation task.  It reconciles the
authority and context of every selected successful-retirement timing row, but
does not claim a complete instruction corpus, PC/AT L3 fidelity, or a
cycle-exact profile.  Intel primary manuals remain the only numerical
authority; existing NXVM behavior, host measurement, Bochs, and PCjs remain
non-authoritative for allocation decisions.

The audit re-read the T360 proposal, S1--S4 evidence, the T359 S1--S7 ledgers,
the Queue, TODO, and the timing owners in `src/core/machine/machine.c`.
`core_machine_instruction_cost()` remains the sole successful post-refresh
publisher.  No handler, decoder, selector, memory, port, device, scheduler, or
delivery path writes elapsed instruction time.  Faulted, rejected, and
interrupted instructions do not publish a successful-retirement source row.

## Original-request mapping

| Requested outcome | Final evidence and result |
| --- | --- |
| Identify the four primary timing authorities and every current consumer. | [S1 inventory](t360-s1-four-profile-source-authority-consumer-inventory.md) binds each source edition and the ten private consumers. |
| Resolve or retain the 80286 `NOP` contradiction without averaging. | [S2 record](t360-s2-80286-nop-authority-precedence.md) retains the owner-selected Appendix-B `NOP = 3`; one precise TODO owns the prose/table revisit. |
| Resolve or transfer legacy range-only timing. | [S3 record](t360-s3-8086-80186-range-form-disposition.md) retains unallocated successful-retirement transfer for every no-formula range; one precise TODO owns future formula/observation admission. |
| Normalize selected 80286/80386 contextual rows without profile borrowing. | [S4 record](t360-s4-80286-80386-contextual-source-disposition.md) proves the capture/publisher boundary for selected rows and transfers uncapturable system/descriptor context exactly. |
| Prevent an L3 or cycle-exact overclaim. | The receiver table below separates source retirement, physical bus, device service, delivery, and cycle-exact work. |

## Final consumer disposition

| Sole consumer | Final source disposition | Evidence / receiver |
| --- | --- | --- |
| `core_machine_8086_source_instruction_cost` | Selected fixed forms retain 8086 table context; range-only arithmetic remains unallocated. | T359 S2--S4, T360 S3, dynamic-form TODO. |
| `core_machine_80186_source_instruction_cost` | Selected fixed forms retain Table 1-16 context; range-only arithmetic and immediate IMUL remain unallocated. | T359 S2--S4, T360 S3, dynamic-form TODO. |
| `core_machine_80286_source_instruction_cost` | Selected MOV/branch/control contexts retain Appendix-B EA, odd-word and next-byte capture; system/descriptor rows remain unallocated. | T357 S6, T359, T360 S2/S4, complete-corpus candidate. |
| `core_machine_80386_source_instruction_cost` | Selected fixed primary rows retain their profile-local source context. | T359 S2/S5, T360 S4. |
| `core_machine_primary_source_instruction_cost` | Selected ALU/data/Group-3 rows use completed shape/prefix capture only. | T359 S2, T360 S3/S4. |
| `core_machine_control_stack_source_instruction_cost` | Selected same-level successful rows use completed state and `byte_count`/`component_count`; transition and delivery are excluded. | T359 S3, T360 S4, cycle-exact candidate. |
| `core_machine_string_io_source_instruction_cost` | Selected primitive/repeat and ordinary-I/O rows use completed prefix/permission state only. | T359 S4, T360 S4; provider/service time is external. |
| `core_machine_80386_dynamic_multiply_cost` | Selected 80386 formula remains profile-local and exact. | T359 S2, T360 S4. |
| `core_machine_80386_secondary_source_instruction_cost` | Selected `0F` integer rows use exact source prefix, memory, `m`, and completed-state conditions. | T359 S5, T360 S4. |
| `core_machine_80386_privileged_source_instruction_cost` | Selected successful system rows use legal prefix/LOCK, ModRM, mode, and CPL capture; descriptor-granular `LSL` is unallocated. | T359 S6, T360 S4, complete-corpus candidate. |

## Remaining receivers

| Remaining class | Exact receiver and admission condition |
| --- | --- |
| 8086/80186 dynamic arithmetic ranges | `TODO(Medium)`: primary formula or separately approved observation contract, profile-local capture/rollback, complete form and consumer sweep. |
| 80286 Appendix-B/prose `NOP` conflict | `TODO(Medium)`: edition/page/semantic-context review plus every classifier, smoke, scheduler and device consumer. |
| 80286 system/descriptor forms and 80386 descriptor-granular `LSL` | Closed [instruction-timing corpus proposal](../../history/M5-T363-complete-instruction-timing-corpus-proposal.md): its profile-local form/context matrix and capture feasibility record remains available for historical audit. |
| Memory/I/O waits, HOLD/DMA ownership, prefetch/cache/alignment and pin phases | T369 closes logical HOLD/DMA ownership; physical values transfer to [Cycle-exact selected profile](../../proposals/m5-cycle-exact-selected-profile.md). [Retained proposal](../../history/M5-T369-bus-timed-pcat-operation-proposal.md). |
| Device command and service latency | Closed T370 [PC/AT device service-timing corpus](../../history/M5-T370-pcat-device-service-timing-corpus-proposal.md). |
| Exceptions, IRQ/NMI, task switch, outer-privilege/VM86 frames and rejected forms | Queue candidate [Cycle-exact selected profile](../../proposals/m5-cycle-exact-selected-profile.md); no synthetic successful-retirement clock. |
| x87 numeric execution/timing and VME/PVI | Their named TODO boundaries; neither is a T360 source-row receiver. |

## Sweep and verification conclusion

The sweep found no duplicate elapsed-tick publisher, no profile borrowing, no
prose-derived two-clock NOP consumer, no midpoint allocation, and no hidden
numeric allocation for the legacy range rows.  The retained static inventory
verifier protects the source-consumer topology.  T360 therefore closes only
the reconciliation prerequisite for later corpus and physical-timing work.

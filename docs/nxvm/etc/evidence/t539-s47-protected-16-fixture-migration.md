# T539 S47: protected-16 fixture migration

## Result boundary

S47 retires the five direct-private S3--S7 smoke consumers.  Their official
CTest targets now construct protected mode solely through the public Core
machine boundary: reset, guest instructions, physical-memory writes, public
debug snapshots, and the public planar-parity/NMI operation.  No production
or public ABI changed.

The former S3 source is retained under the honest name
`protected_16_timing_fixture.c`.  It is no longer an S3 smoke target or a
public-board fixture: the 80286 and 80386 timing manifest runners still depend
on its timing-specific private recipe.  S50 owns migration of those runners.
This preserves one timing recipe during that later migration rather than
creating a second one or misrepresenting it as public setup.

## Receiver map

| Retired source | Public receiver | Preserved observable contract |
| --- | --- | --- |
| `core_machine_protected_16_gate_s3_smoke.c` | `core_machine_protected_16_gate_board_smoke.c` | 80286/80386 interrupt and trap gates, frame shape, real user entry, absent/DPL/illegal gate outcomes and terminal diagnostics. |
| `core_machine_protected_16_external_s4_smoke.c` | `core_machine_protected_16_external_board_smoke.c` | Outer software entries, gate IF/TF behavior, DPL rejection, and NMI delivery through the board parity input. |
| `core_machine_protected_16_outer_s5_smoke.c` | `core_machine_protected_16_outer_board_smoke.c` | 80286 and 80386 16/32-bit TSS outer NMI stack transitions and interrupt/trap frame semantics. |
| `core_machine_protected_16_outer_iret_s6_smoke.c` | `core_machine_protected_16_outer_iret_board_smoke.c` | Same- and outer-CPL IRET/RETF/RETF-immediate frames, address-prefix form, and public register snapshots. |
| `core_machine_protected_16_call_gate_s7_smoke.c` | `core_machine_protected_16_call_gate_board_smoke.c` | 16-bit call-gate same/outer transfers, parameter copying, 80286/80386 TSS forms and DPL rejection. |

The old direct PIC assertions are not reproduced through a mutable Core
member.  PIC routing/IRR/ISR ownership remains covered by the component tests
`core_machine_pic_irq_lifecycle_smoke.c`, `core_machine_pic_ocw3_smoke.c` and
`core_machine_pic_command_priority_smoke.c`; S47's protected-transfer tests
observe their architectural result through real NMI delivery and public CPU
diagnostics.

The old deliberately-corrupted TR/cache and stack-cache cases cannot be
constructed by an architectural guest.  S47 retires those representation
assertions rather than recreating a private mutation seam or calling them
public behavior.  The separate timing fixture still contains only the
timing-runner recipe; S50 owns migration of that recipe and must not treat the
retired cache mutations as timing requirements.

## Verification recorded so far

On 2026-09-30, the five official targets built and emitted their original
markers on both x64 and x86:

```
M5:T323:S3:PROTECTED-16-GATE:OK
M5:T323:S4:PROTECTED-16-EXTERNAL:OK
M5:T323:S5:PROTECTED-16-OUTER:OK
M5:T323:S6:PROTECTED-16-OUTER-IRET:OK
M5:T323:S7:PROTECTED-16-CALL-GATE:OK
```

Both 80286 and 80386 timing-manifest runners also rebuilt with their renamed
timing-only fixture.  The complete repository-only unit suite then passed on
both x64 and x86 (512 tests per width); no current x86 failure list was
created and the pre-existing x64 failure list predates this replay.  The NXVM
documentation-governance gate and `git diff --check` also pass.  A targeted
private-access sweep over the five new board receivers and their shared public
fixture finds no `executor_cpu`, `executor_memory`, `shared_pic`, `flagNMI`,
`machine.h`, or `ram.h` reference.

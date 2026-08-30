# T512 S4 Shared FLAGS Image Repair

`M5:T512:S4:FLAGS-IMAGE:COMPLETE-PENDING-REVIEW`

## Source Rule And Bound

Intel *386 DX Microprocessor Programmer's Reference Manual*, order 230985-003
(1990), printed page 14-7, was rendered from the owner-managed original scan.
Its real-address rule says that `PUSHF`, interrupts and exceptions store FLAGS
differently from the 8086 in bits 12--15: bit 15 is always clear while bits
14--12 retain their last loaded values. Its virtual-8086 rule gives the same
bit-15 result. This is a Manual-L3 image rule.

The rule does not define a new 32-bit EFLAGS load contract or an exact value
for every reserved state bit. The repair therefore changes neither
`_e_eflags_load()`'s 80386 32-bit branch nor protected-mode frame production.

## One-Owner Repair

| Surface | Before | After |
| --- | --- | --- |
| `cpu_instructions.c`, `_e_real_flags_defined_mask()` 80386 branch | `FFD5h` admitted bit 15 into each 16-bit real/V86 image. | `7FD5h` retains the source-defined fields and clears bit 15 through the existing image helper. |
| `_e_real_flags_image_16()` callers | `PUSHF`, real-mode interrupt/exception frames and the bounded firmware service frame all used the one helper. | Same callers and one owner; no new frame or image route. |
| `_e_real_flags_load_16()` | Uses the same defined-field mask only for the bounded firmware result path. | Retains its deterministic reserved-bit canonicalization; no claim is made for a hardware 32-bit load. |
| 32-bit EFLAGS and protected frames | Separate existing paths. | Inspected and intentionally unchanged. |

## Regression Sweep

| Owner-local test | Coverage added or corrected |
| --- | --- |
| `core_machine_pushf_popf_s47_smoke.c` | Seeds bit 15 and directly requires an 80386 `PUSHF` 16-bit stack image to clear it. |
| `core_machine_software_int_s50_smoke.c` | Seeds bit 15 and requires each real software-interrupt 16/32-frame FLAGS image to clear it. |
| `core_machine_tf_db_s60_smoke.c` | Seeds bit 15 and requires the 80386 real debug-exception frame image to clear it. |
| `core_machine_iret_s51_smoke.c` | Stops treating reserved bit 15 as a defined IRET load result. |
| `core_machine_legacy_alu_s2_smoke.c`, `core_machine_scas_smoke.c` | Stop treating reserved bit 15 as defined architectural state in profile masks. |

The direct six-test cohort passed after an outside-sandbox incremental build:
6/6, 6.77 seconds real time. The required full repository-only unit gate then
passed 313/313 in 14.64 seconds real time, and
`Verify-DocumentationGovernance.ps1 -RepositoryRoot .` passed for
`vm-0-5-0511`. These results are pending coordinator actual-change review.

`M5:T512:S4:SOLE-OWNER:READY`

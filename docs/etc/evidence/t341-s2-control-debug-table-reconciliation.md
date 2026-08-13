# T341 S2: Control, Debug-Register, And Table-State Reconciliation

## Purpose

S2 closes the control, debug-register transfer, and descriptor-table form
ledger only. It does not claim ordinary hardware breakpoint matching or
vector-1 cause/delivery; S4 owns that state transition. It also does not turn
the functional no-persistent-TLB paging model into an Intel test-register
model.

The Intel 80386 primary reference lists `CR0`, `CR2`, and `CR3` for `MOV CR`,
and debug registers `DR0`--`DR3`, `DR6`, and `DR7` for `MOV DR`.
[Intel 80386 MOV special registers](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/MOVRS.htm)
therefore does not require a later-CPU DR4/DR5 alias policy.

## Actual State-Owner Matrix

| Form and state | Decoder and mutable owner | Validation, commit, and fault boundary | Accepted proof / transfer |
| --- | --- | --- | --- |
| `MOV r32,CR0/CR2/CR3` and `MOV CR0/CR2/CR3,r32` | `_d_modrm_creg`; `MOV_R32_CR`; `MOV_CR_R32`; `_s_write_cr0_80386`; `_s_write_cr3_80386`; direct CR2 write is the sole guest CR2 writer outside page-fault publication. | Register-only decoder rejects memory and reserved CR numbers before a GPR/control write. CPL check precedes decode/publication. CR0 and CR3 validate before commit; CR2 has no additional 80386 form validation. | T325 S1--S3 and `core-machine-descriptor-system-smoke` prove fixed 32-bit forms, real/CPL0 success, CPL3/VM86 `#GP(0)`, reserved and memory `#UD`, CR0/CR3 policy, CR2 symmetry, page-fault CR2, and no-cache CR3 selection. No reproduced owner defect. |
| `MOV r32,DRn` and `MOV DRn,r32`, `n=0..3,6,7` | `_d_modrm_dreg`; `MOV_R32_DR`; `MOV_DR_R32`; CPU DR fields. | Register-only and CPL checks precede any GPR/DR publication. `_d_modrm_dreg` maps exactly DR0--DR3/DR6/DR7 and rejects DR4/DR5. | `core-machine-debug-mov-s59-smoke` proves both directions, profile, real/protected CPL0, CPL3, attribute, LOCK, reserved/memory, and IRQ boundaries. DR4/DR5 remain Intel-reserved on 80386, not an alias gap. |
| `MOV r32,TR6/TR7` and `MOV TR6/TR7,r32` | `_d_modrm_treg`; `MOV_R32_TR`; `MOV_TR_R32`; CPU test-register fields. | The decoder admits only TR6/TR7, but no cache/test-array state consumes them. | External boundary: T325 proves no persistent translation cache and explicitly transfers TR6/TR7 until a cache lifetime, CR3/task-switch flush, and deterministic test-register matrix is admitted. No false register-only completion claim. |
| `SLDT`/`STR`/`LLDT`/`LTR` (`0F 00 /0--/3`) | `INS_0F_00`; `_s_load_ldtr`; `_s_load_tr`; selector/XDT cache owners. | Protected-mode decoder, fixed selector width, candidate descriptor/cache validation, then LDTR/TR and LTR busy-bit commit. | T316 S61 and `core-machine-dttr-s61-smoke`, with retained `core-machine-descriptor-system-smoke`, prove cache and busy publication, null/TI/type/present rejection, source/destination faults, profile/attribute/LOCK, and no partial load. Task images are S3. |
| `SGDT`/`SIDT`/`LGDT`/`LIDT` (`0F 01 /0--/3`) | `INS_0F_01`; `_m_write_table_pseudo_descriptor`; `_s_load_gdtr`; `_s_load_idtr`. | Table-memory decoder rejects register ModRM. Loads check protected CPL/VM86 before source decode and table-register commit; stores preflight the destination path. | T318/T319, T321 S5, and `core-machine-sgdt-sidt-smoke` plus `core-machine-vm86-lgdt-lidt-s5-smoke` prove six-byte table images, width, table atomicity, privilege, and VM86 rejection. No reproduced owner defect. |
| `SMSW`/`LMSW` and `CLTS` | `INS_0F_01`; `_s_load_cr0_msw`; `CLTS`. | Fixed `r/m16` path and protected CPL/VM86 validation precede CR0 mutation; PE stickiness remains local to MSW state. | T316 S62/S63 owner smokes and descriptor-system proof cover profile, fixed width, CR0 publication, source/destination fault, attributes, LOCK, and IRQ boundaries. |

## Similar-Issue Sweep

The review queried all tracked CPU execution, state, current-gate, prior
evidence, Queue, and TODO records using:

```powershell
rg -n "_d_modrm_(creg|dreg|treg)|MOV_(R32_(CR|DR|TR)|(CR|DR|TR)_R32)|"
  "_s_write_cr[03]_80386|_s_load_(gdtr|idtr|ldtr|tr)|INS_0F_0[01]|"
  "CLTS|SMSW|LMSW|DR[0-7]|TR[0-7]" src tests CMakeLists.txt docs
```

Every production hit has the owner above. The only test-register consumer gap
is the already explicit no-persistent-cache boundary; it is not silently
hidden by the storage fields. DR4/DR5 are reserved by the Intel 80386 form
table and have a deliberate `#UD` decoder outcome. No duplicated writer,
split validation-to-commit construction, or unclassified form was found.

## Verification

The exact current-gate owner set is:

- `current.core-machine-descriptor-system-smoke`;
- `current.core-machine-dttr-s61-smoke`;
- `current.core-machine-debug-mov-s59-smoke`;
- `current.core-machine-80386-paging-smoke`; and
- `current.core-machine-vm86-lgdt-lidt-s5-smoke`.

S2 reruns that set and the full current gate. The next S receives only VM86,
task, and paging caller composition; S4 receives ordinary DR6/DR7 breakpoint
cause and vector-1 delivery. No S2 implementation change is required.

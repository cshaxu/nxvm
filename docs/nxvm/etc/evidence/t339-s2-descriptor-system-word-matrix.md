# M5 T339 S2: 80286 Descriptor-Table And System-Word Matrix

## Mechanism Inventory

S2 owns the 80286 table-register store/load and machine-status-word forms:

| Form | Decode and materialization owner | Current-gate owner evidence |
| --- | --- | --- |
| `0F 01 /0` SGDT | `INS_0F_01`, `_d_modrm_table_memory`, `_m_write_table_pseudo_descriptor` | `current.core-machine-sgdt-sidt-smoke` |
| `0F 01 /1` SIDT | Same six-byte destination transaction | `current.core-machine-sgdt-sidt-smoke` |
| `0F 01 /2` LGDT | `INS_0F_01`, full pseudo-descriptor source read, `_s_load_gdtr` | `current.core-machine-lgdt-lidt-smoke` |
| `0F 01 /3` LIDT | Same full source read, `_s_load_idtr` | `current.core-machine-lgdt-lidt-smoke` |
| `0F 01 /4` SMSW | `INS_0F_01`, `_m_write_rm` | `current.core-machine-msw-s63-smoke` |
| `0F 01 /6` LMSW | `INS_0F_01`, `_m_read_rm`, `_s_load_cr0_msw` | `current.core-machine-msw-s63-smoke` |
| `0F 06` CLTS | `CLTS` | `current.core-machine-clts-s62-smoke` |

`INS_0F_01` is the only primary dispatcher for `/0`--`/4,/6`; the listed
load/store helpers are its only table or MSW commit writers.  The store helper
preflights all six pseudo-descriptor bytes before its one logical write.  The
load branches read the complete source image before changing GDTR or IDTR.
No S2 reproducer required a CPU handler change.

## 80286 Form And Boundary Evidence

| Matrix boundary | Evidence and result |
| --- | --- |
| Real/protected form disposition | SGDT/SIDT and LGDT/LIDT success loops run both 80286 and 80386.  SMSW/LMSW and CLTS success loops also run both profiles.  The retained 80186 `0F`/attribute rejection owners remain negative regression evidence, not 80286 proof. |
| 16-bit image and memory-only form | SGDT/SIDT assert the six-byte limit/base image and memory-only ModRM rejection.  LGDT/LIDT distinguish the source pseudo-descriptor image; SMSW/LMSW distinguish the low machine-status word; all retain EIP, defined CPU state, and unrelated table state as their form permits. |
| Protected CPL boundary | LGDT/LIDT now boot both an 80286 16-bit and 80386 gate fixture.  A CPL3 invocation delivers the protected `#GP` through vector 13, leaves table state and source image unpublished, preserves restart IP, and reaches the selected handler.  The 80286 bootstrap deliberately omits FS/GS loads, which are 80386-only. |
| Source/destination limits and atomicity | SGDT/SIDT destination preflight and LGDT/LIDT source-limit vectors prove no partial table image.  SMSW/LMSW memory-limit vectors retain MSW or destination state on failure.  The no-IDT terminal observables are recorded as delivery boundaries, not as unchanged post-delivery CPU state. |
| Successful IRQ order | SGDT/SIDT, LGDT/LIDT, SMSW/LMSW, and CLTS retained owner smokes prove a pending PIC IRQ is delivered after a successful form, with the saved IP after the instruction and ISR/IRR transition. |
| `LMSW` and `CLTS` 80286 CPL3 delivery | S2 proves their 80286 CPL0 behavior and records the protected privilege producer in the handler audit.  Complete 80286 CPL3 exception gate/frame delivery belongs to T339 S4's 16-bit protected-entry mechanism.  It must provide the selected 16-bit IDT/frame proof before that transfer is considered closed; S2 does not substitute a hand-built no-IDT terminal state. |

`66/67`, VM86, CR0 32-bit/paging effects transfer to T341/T342.  The complete
Accepted T328 owns the pre-386 `LOCK` policy matrix. S2 neither infers those
results from the shared 80386 decoder nor changes their owner.

## Configuration-Gate Correction

Fresh configuration failed before S2 testing because
`core-machine-control-transfer-smoke` appeared in both T337's explicit
terminal-real-delivery and explicit no-terminal-negative lists.  Its source
uses `test_core_machine_fixture_preflight_real_ud_terminal`, so the CMake
static gate correctly reported a conflicting disposition.

The complete `PROJECT_T337_UD_NO_REAL_NEGATIVE_TARGETS` sweep found no other
target using that terminal preflight.  S2 therefore removed only
`core-machine-control-transfer-smoke` from the stale no-terminal list.  This
does not alter CPU execution, target membership, or the T337 policy; it makes
the existing disposition list agree with the current owner smoke.

## Verification

- Fresh `mingw-gcc-x64` configuration passed after the correction.
- Focused SGDT/SIDT, LGDT/LIDT, SMSW/LMSW, and CLTS owners passed.
- `verify-current-artifact-target`, documentation governance, whitespace, and
  the full current gate are required at P1 acceptance.

This evidence retires into the T339 history record when T339 closes.

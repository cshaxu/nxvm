# M5 T309 S2: 80386 Form Audit And Debug-Capture Design

## Scope And Authority

This audit classifies executable form families, not product capability. Intel's
*80386 Programmer's Reference Manual* is authoritative: Chapter 2,
"Applications Architecture"; Chapter 3, "Basic Execution Environment";
Chapter 4, "Applications Instruction Set"; Chapter 5, "Memory Management";
Chapter 6, "Protection"; and Chapter 9, "Interrupts and Exceptions". The
read-only comparison identities are Bochs 2.6 `cpu/fetchdecode.cc`,
`cpu/bit16.cc`, `cpu/bit32.cc`, `cpu/data_xfer16.cc`, `cpu/data_xfer32.cc`,
`cpu/protect_ctrl.cc`, and `cpu/exception.cc`; and PCjs 2.00.0
`machines/pcx86/modules/v2/x86ops.js`, `x86op0f.js`, `segx86.js`, and
`x86help.js`. They are behavior references only; no source is copied.

The audit used `core_machine_cpu_instruction_metadata_get`,
`core_machine_cpu_profile_allows_form`, `ExecIns`, `INS_0F`, the initialized
primary and `0F` dispatch tables, profile checks in instruction handlers,
existing focused synthetic probes, and the CPU TODO/capability records. A
metadata entry states only the decoder's minimum-profile/validity opinion. A
form is **proven** only where a focused probe establishes its stated contract;
an initialized handler without that proof is **unproven**. **Rejected** means
the active profile gate reaches architectural `#UD`; **reserved/later** means
the encoding is not an 80386 form or is intentionally gated out; **absent**
means no reachable project implementation is claimed.

## Form Matrix

| Family and source route | 80386 form classification | Evidence and disposition |
| --- | --- | --- |
| `64h`--`67h`, repeated prefix loop, operand/address-size XOR | Proven for FS/GS, `66h`, `67h`, repeated-prefix reset, fetch, ModRM/SIB, stack, and strings | T301/T302 focused probes. Pre-80386 profile rejection is proven; no generic claim about every legacy opcode combined with every prefix. |
| Primary arithmetic, move, test, exchange, LEA, and immediate groups | Unproven as a complete 32-bit form family | The handlers use operand/address helpers and are reachable, but only the T302 cross-width/data subset is proven. Metadata defaults these opcodes to 8086 and therefore does not certify 32-bit semantics. |
| Primary 80186 extensions: `60h`--`62h`, `68h`--`6Fh`, `C0h`/`C1h`, `C8h`/`C9h` | Proven only for admitted stack and REP string forms; unproven for BOUND and immediate IMUL breadth | T302 proves PUSHA/POPA, ENTER/LEAVE, and INS/OUTS/string forms. The profile gate is present; no broader claim is made. |
| Near/far control transfer, conditional branch, loop, same-CPL far transfer | Proven within admitted 16/32 protected and real-mode matrix | T303 covers Jcc/loop, near CALL/JMP/RET, same-CPL RETF, and far immediate/indirect pointers. Outer returns are T306. Gates/tasks/V86 remain outside this row. |
| Segment loads, LxS, ARPL, LAR/LSL, VERR/VERW | Proven within the T301 form/profile/mode matrix | T301 covers legal segment-load widths, selector query rules, cache/accessed atomicity, and reserved/later rejection. |
| Descriptor tables, LDTR/TR, MSW, and admitted CR0/CR2/CR3 | Proven only for T304's explicit subset | `0F 00h`/`01h` and `0F 20h`/`22h` have more named handler surface than admitted evidence. Debug/test-register forms are executable-table entries but unproven and remain deferred. |
| 32-bit same/outer IDT entry, error frames, IRET/RETF, and call-gate privilege entry | Proven within T305--T308 exact planner matrices | The proof includes error-code frames, fault atomicity, and bounded contributor-pair `#DF`. Hardware/NMI, page faults, task/V86, and shutdown/reset are not inferred. |
| Paging | Unproven beyond the retained CPL0 4 KiB subset | T258 proves fetch/data/stack walk, A/D updates, CR2, and a non-present fault. CPL3 P/W/U, CR0.WP, cross-page, TLB, and paging-plus-task paths remain deferred. |
| 16-bit TSS task switch and I/O map | Proven only for retained T260/T261 forms | 32-bit TSS switching, task gates, nested return, and broad LDT behavior are unproven/deferred. |
| `0F 80h`--`8Fh` Jcc and `0F A0h`/`A1h`/`A8h`/`A9h` FS/GS | Proven | T303 proves Jcc width/profile forms; T301 proves FS/GS segment forms. |
| `0F 90h`--`9Fh` SETcc | Unproven | Metadata marks these 80386 forms valid and the `0F` table dispatches `SETO` through `SETG`; no focused form/flags/memory atomicity probe proves them. |
| `0F A3h`/`ABh`/`B3h`/`BBh` plus `0F BAh /4`--`/7` bit-test/modify | Unproven | Metadata and dispatch reach BT/BTS/BTR/BTC and immediate group forms. Register versus memory bit indexing, CF, write, and fault boundaries lack focused proof. |
| `0F A4h`/`A5h`/`ACh`/`ADh` SHLD/SHRD; `0F AFh` IMUL; `0F BCh`/`BDh` BSF/BSR; `0F B6h`/`B7h`/`BEh`/`BFh` MOVZX/MOVSX | Unproven | All are metadata-valid 80386 table entries, but no form-level focused evidence covers count/zero/flag/memory semantics or 16/32 crossings. |
| `0F 20h`--`26h` control/debug/test register moves | Mixed: CR0/CR2/CR3 proven; debug/test unproven | T304 deliberately admits only CR0/CR2/CR3. Handler names for DR/TR do not grant an admission or consumer. |
| `0F 09h` WBINVD, `0F 30h` WRMSR, `0F 32h` RDMSR, `0F A2h` CPUID, `0F AAh` RSM | Rejected/reserved-later | Some names exist in the table, but metadata rejects them before execution on the active 80386 profile. They are not evidence of implemented 80386 behavior. |
| `0F B0h`/`B1h` CMPXCHG, `0F C0h`/`C1h` XADD, `0F B8h`/`B9h` and remaining `0F` holes | Reserved/later or absent | The active metadata/table route rejects these; no later-CPU feature is admitted by this audit. |

The static sweep found no form which is both metadata-valid, dispatch-reachable,
and silently mapped to a second executor. `ExecIns` retains the one decoder
and profile gate; `INS_0F` repeats the `0F` form gate before table dispatch.
The project-wide capability matrix remains intentionally conservative: it is a
product evidence summary, not this exhaustive decoder inventory.

## Bounded Capture Contract

Future diagnosis begins with a focused prepared-state probe. An owner-supplied
checkpoint is permitted only after the probe cannot identify a first fault or
instruction form, and only with local ignored media. The trigger must name a
single stop condition: first terminal fault, first unexpected mode/selector or
control-state transition, or a fixed instruction address. The capture records
only copied observation state already available at the boundary: CS:EIP and
linear PC, instruction bytes/window, general registers, EFLAGS, selected
segment/cache and CR0--CR3 fields, stop/fault diagnostic, and existing port or
run-boundary trace events. It never borrows guest VRAM/RAM or changes executor
ownership.

| Limit | Contract |
| --- | --- |
| Instruction budget | At most 64 guest instructions from the named trigger. |
| Wall-clock budget | At most 5 seconds for local capture. |
| No-progress budget | Stop after 16 consecutive instructions with an unchanged named progress tuple. |
| Trace budget | At most 256 events and 64 KiB of locally ignored text; existing core trace batching is 32 events. |
| Media and artifacts | Guest media, checkpoints, traces, Bochs binaries, and bridge output remain outside the repository, default build, artifact, and CTest. |

The existing core trace provider records reset, port read/write, run-boundary,
stop, and fault events; VM debug trace count is a bounded pause aid. Neither is
a new capture API in S1. A paired local Bochs step bridge is allowed only after
Intel plus the two read-only references leave a concrete form ambiguous and
the coordinator approves it. It uses at most 32 synchronized instructions,
the same 5-second/64-KiB budgets, an explicit register/flags/segment mask that
excludes architecturally undefined fields, and stops at the first mismatch.

## Recommended Next Family

The highest-ROI next admission is the **80386 `0F` integer bit and data-form
family**: SETcc, BT/BTS/BTR/BTC including `0F BA /4`--`/7`, SHLD/SHRD,
BSF/BSR, MOVZX/MOVSX, and the already-decoded IMUL form. This is a single
decoder/data-helper family with metadata-valid and dispatch-reachable forms,
but no focused proof of register/memory, 16/32 operand/address, flags, limit,
or fault-commit behavior. It is therefore a concrete evidence gap rather than
a speculative hardware expansion.

It ranks ahead of CPL3 paging because paging needs a separate permission and
page-fault-delivery contract with wider consumer and policy impact. It ranks
ahead of debug/test registers, task gates, 32-bit task switching, V86, and
later system opcodes because those either lack a current consumer or cross the
frozen system/delivery boundary. A future packet must still start with a
focused prepared-state probe and may narrow this family if its first evidence
selects a smaller independently correctable subset.

## S2 Verification

Static queries covered `core_machine_cpu_instruction_metadata_get`,
`core_machine_cpu_profile_allows_form`, `ExecIns`, `INS_0F`, initialized
dispatch tables, profile checks, `CORE_MACHINE_TRACE_CAPACITY`, trace/provider
routes, all focused smoke names, `TODO.md`, and the current capability matrix.
S2 changes no CPU behavior, CMake target, artifact, Queue, or product path.

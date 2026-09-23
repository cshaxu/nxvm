# T336 Four-Profile Coverage Ledger

## Scope And Method

This active-task evidence reconciles the current decoder, metadata, focused
owner smokes, closed M5 records, `TODO.md`, and the Intel 80386DX closure map.
It is a ledger, not an architecture authority or a completion claim. The Intel
instruction and architecture manuals remain the behavioral authority.

`Complete bounded` means that the listed historical task proved its declared
matrix. It does **not** mean that the form has met the stronger four-profile
program requirement unless the profile and mode columns below say so. `Partial`
means an in-scope program row still needs exact-form reconciliation or a later
implementation task. `Outside` means later-than-80386; `External` means the
CPU-side interface is in scope but coprocessor execution is not.

## Decoder And Profile Inventory

`core_machine_cpu_instruction_metadata_get()` is the mechanical decoder
baseline. Primary forms default to 8086; `60`--`62`, `68`--`6F`, `C0/C1`, and
`C8/C9` declare 80186; `63` declares 80286; and `64`--`67` declare 80386.
The secondary `0F` table declares `00`--`03` and `06` at 80286, and its listed
control, conditional-control, bit/data, and segment forms at 80386. `D8`--`DF`
are external-coprocessor escapes, not ordinary 8086--80386 execution forms.

Handler reachability and metadata are discovery evidence only. Each row below
requires a focused proof of its valid profiles and Intel-defined rejection or
fault behavior before it can become program-complete.

The actual dispatch initialization is also a finite audit boundary:
`cpu_instructions.c` assigns the primary `00`--`FF` table and the secondary
`0F` table in one location. Its explicit `_______todo` owners identify
architecture-state work, not automatically missing opcodes: task switch,
conforming/non-conforming far-call construction, call-gate jump construction,
INT/INTO/exception/IRQ delivery, and IRET. These owners are assigned below to
the shared-delivery, 80286, or 80386DX candidate according to their mode and
layout requirements. `CPUID` and `RSM` are present only as explicit
later-CPU rejection routes, not deficits in an 80386DX claim.

## Reachable Form-Family Index

This index is derived from the metadata switch and the two dispatch-table
initializers. Ranges mean architecturally related primary forms, not that every
ModRM extension or mode outcome is assumed equivalent. The final S1 ledger
must expand each `Partial` family to the relevant ModRM and mode rows.

| Encoding family | Minimum profile in current metadata | Existing bounded evidence | Program disposition / sole next owner |
| --- | --- | --- | --- |
| Primary `00`--`5F`: binary arithmetic, segment stack, prefixes, INC/DEC, GPR stack | 8086 | T316 S2--S47 and T328 cover named slices. | **Partial four-profile:** **8086/80186 closure** owns inherited form/FLAGS/stack reconciliation; **80286** owns protected selector/stack semantics; **80386DX** owns alternate widths. |
| Primary `60`--`62`, `68`--`6F`, `C0/C1`, `C8/C9` | 80186 | T316 S38/S42--S45/S54--S56 cover named extension slices. | **Partial:** **8086/80186 closure** owns the full 80186 extension and rejection matrix. |
| Primary `63` (`ARPL`) | 80286 | T316 S53 proves bounded protected forms. | **Partial:** **80286 closure** owns remaining protected selector/mode/form reconciliation. |
| Primary `64`--`67` | 80386 | T316 S64 proves prefix semantic classes. | **Partial:** **80386DX closure** owns 32-bit attribute combinations not proven by the semantic-class closure. |
| Primary `70`--`7F`, `E0`--`E3`, `E8`--`EB`, `C2/C3`, `CA/CB`, `9A/EA`, `CC`--`CF` | 8086, with 80386 operand-width variants | T303, T316 S50--S52, T321/T323/T329 prove bounded transfer/return paths. | **Partial:** **shared delivery** owns producer/frame ordering; profile candidates own all residual form, privilege, and task/VM86 rows. |
| Primary `80/81/83`, `84/85`, `86/87`, `88`--`8F`, `90`--`BF`, `C4`--`C7`, `D0`--`D7`, `F6/F7`, `FE/FF` | 8086 plus declared later forms | T316 ordinary and T322 reconciliation cover named ordinary forms. | **Partial four-profile:** **8086/80186 closure** owns baseline classification; **80286/80386DX** own system/selector and width-specific residuals. |
| Primary `A0`--`AF`, `6C`--`6F`, `E4`--`E7`, `EC`--`EF` | 8086/80186 as metadata declares | T316 S30--S38/S55 proves declared data/string/I/O slices. | **Partial:** ordinary execution is evidenced, but I/O privilege and all form/profile compositions transfer to **shared delivery**, **80286**, and **80386DX** by mode. |
| Primary `D8`--`DF`, `9B` | External coprocessor / 8086 CPU-side interface | T316 S65 and T317 CPU-side contract evidence. | **External:** x87 execution is excluded; no profile task may claim it. |
| `0F 00 /0`--`/5`, `0F 01 /0`--`/6`, `0F 02/03/06` | 80286 | T316 S57--S63 and T318/T319 bounded forms. | **Partial:** **80286 closure** owns exact descriptor/table/system-form matrices; **shared delivery** owns their exceptions. |
| `0F 20`--`26` | 80386 | T316 S59, T321 and T325 cover bounded control/debug/test rows. | **Partial:** **80386DX closure** owns complete CR/DR/TR classification, privilege, state, and fault behavior. |
| `0F 80`--`8F`, `90`--`9F`, `A0/A1/A3`--`A5/A8/A9/AB`--`AD/AF`, `B2`--`B7`, `BA /4`--`/7`, `BB`--`BF` | 80386 | T303/T310/T316 S23--S28 and S57--S60 cover named data/control slices. | **Partial:** **80386DX closure** owns the complete 0F form/profile/mode matrix. |
| `0F A2`, `0F AA`, and other metadata-invalid/table-undefined holes | Later or reserved | Decoder rejects these forms. | **Outside:** final ledger records each as Intel-reserved/later rather than treating a handler name as support. |

## Horizontal Instruction Ledger

| Form group | 8086 | 80186 | 80286 | 80386DX | Current disposition and next owner |
| --- | --- | --- | --- | --- | --- |
| Base data movement, exchange, LEA, moffs, sign extension, stack and frames | Bounded real-form evidence exists. | Extension forms have bounded evidence. | Inherited/default forms have bounded evidence. | Operand/address variants have bounded evidence. | **Partial:** T322 reconciles ordinary 80386 forms, but no single four-profile form ledger proves every inherited encoding, flag/stack quirk, and rejection. **8086/80186 closure** owns baseline/extension reconciliation; **80286 closure** owns protected selector/stack semantics; **80386DX closure** owns 32-bit forms. |
| Base arithmetic, adjust, shifts, multiply/divide, BOUND, ARPL, immediate IMUL | Bounded ordinary evidence exists. | BOUND/immediate IMUL and shift extensions have focused slices. | ARPL/protected boundary has focused slices. | 32-bit variants and 0F integer slices have focused evidence. | **Partial:** form-family coverage is strong but historical slices are not a complete four-profile flag/exception table. **8086/80186** owns inherited and extension arithmetic; **80286** owns ARPL/protected distinctions; **80386DX** owns 32-bit/0F forms. |
| Near/far control, conditional transfer, LOOP/JCXZ, INT/IRET/RETF | Real ordinary slices exist. | Inherited slices exist. | Protected same-CPL and descriptor evidence exists. | 32-bit, outer return, and selected VM86 delivery evidence exists. | **Partial:** shared exception/IRQ/return ordering and real `#UD` IVT delivery remain open. **Shared state and delivery** owns producer/frame/return composition; profile candidates own instruction-form reconciliation. |
| Local FLAGS, HLT, prefix, LOCK, REP and strings/I/O strings | Bounded default and legacy-LOCK evidence exists. | Extension/legacy-LOCK evidence exists. | Protected LOCK distinction exists. | Prefix semantic classes and string attributes exist. | **Partial:** S64/T328 prove mechanism classes, not all profile/form combinations. **8086/80186** owns baseline and legacy extension closure; **80286** owns protected LOCK/privilege rows; **80386DX** owns 32-bit prefix/state rows. |
| Descriptor/query/table/control forms | Outside. | `0F` escape rejects. | Bounded descriptor/table/MSW forms exist. | Bounded 32-bit/table/control forms exist. | **Partial:** complete 80286 and 80386 system-form matrices require profile/mode/privilege reconciliation. **80286 closure** and **80386DX closure** divide those rows; shared delivery owns resulting faults/frames. |
| Paging, VM86, task/LDT, debug/test registers | Outside. | Outside. | 16-bit task/descriptor state has bounded evidence. | CPU-native paging, bounded VM86 delivery/task switch and DR moves have focused evidence. | **Partial:** ordinary debug/breakpoints, task/VM86 paging breadth, reset/trap policy and any remaining test-register classification need program disposition. **Shared state** owns delivery; **80386DX closure** owns state/form rows. |
| ESC and WAIT CPU interface | CPU-side no-provider behavior only. | Same interface. | Same interface. | Same interface plus 80386 attributes. | **External:** T316 S65/T317 evidence closes the CPU-side ESC/WAIT and `#NM` boundary; 8087/80287/80387 execution remains external. |

## Vertical State Ledger

| State transition | Current evidence | Program disposition and next owner |
| --- | --- | --- |
| Real ordinary execution and profile gating | T316/T322/T328 form evidence and profile checks. | **Partial:** real-mode `#UD` IVT policy and corpus-wide migration remain `TODO(High)`; shared delivery must resolve it before cross closure. |
| Protected segmentation, privilege, gates, and stack transition | T323 completes its non-task/non-VM86 boundary. | **Partial:** audit all remaining 80286/80386 table/gate/return forms into the 80286 and 80386DX candidates. |
| VM86-to-CPL0 exception/IRQ and bounded return | T320 closes the declared TSS/IDT/frame transition. | **Complete bounded:** retain VME/PVI as outside-80386; assign residual VM86 instruction/task/paging rows to 80386DX closure. |
| Exception, IRQ, NMI, and return producer ordering | T321/T326/T331 prove listed producer routes and real final-delivery construction. | **Partial:** debug/breakpoint, remaining real `#UD`, task/reset and broader trap policy must receive exact delivery classifications in shared state. |
| Paging/translation | T325/T326 prove native 80386 4-KiB walk, permissions, A/D, `#PF`, and pre-486 `INVLPG #UD`. | **Partial:** task/VM86 paging and any not-yet-classified CR/system interaction belong to 80386DX closure; persistent TLB/TR6/TR7 are outside this 80386DX program unless the Intel ledger says otherwise. |
| Task/LDT/debug | T329 closes the declared 16/32-bit task-transition state machine. | **Partial:** ordinary breakpoint/debug-register behavior and VM86 breadth remain 80386DX closure rows. |
| External coprocessor | T316 S65/T317 boundary evidence. | **External:** no x87 execution claim. |

## Required Audit Completion Work

Before T336 can close, replace every `Partial` broad row above with exact Intel
form rows and one of: existing proof after actual-smoke inspection; a bounded
candidate transfer; an explicit outside-80386 classification; or the external
coprocessor boundary. The later Queue candidates are dependency containers,
not evidence that a row is complete.

## Confirmed Residual Transfers

The current `TODO.md` supplies four already-classified residuals that cannot
be erased by historical handler reachability:

| Residual | Evidence and required disposition |
| --- | --- |
| Real-mode vector-6 `#UD` | `TODO(High)` records 41 retained owner fixtures that depend on terminal no-handler behavior. **Shared CPU state and delivery** must define IVT policy, frame/restart/IF-TF semantics, and migrate or retain every fixture deliberately. |
| 8086 corpus/form breadth | `TODO(High)` names arithmetic/FLAGS, conditional transfer, stack edges, and compare/scan strings. **8086 and 80186 profile closure** must replace corpus-only confidence with the Intel form matrix; a DOS path may reveal a gap but cannot define completeness. |
| System/descriptor and task breadth | `TODO(Low)` preserves withdrawn LDT/task/VM86/debug/test-register rows. The T336 ledger must distinguish rows T329 truly closes from residual 80286 or 80386DX system forms, then transfer each to its profile candidate. |
| VME/PVI and x87 execution | `TODO(Medium)` and `TODO(Low)` retain both as explicit non-80386 or external-coprocessor boundaries. They cannot block CPU-side cross closure once their exclusion is mechanically carried through the ledger. |

# M5 Td S182: NXVM TODO disposition

Baseline: NXVM 92c66ac04, static inspection of the current TODO, Queue,
Shared rules and selected production/test paths; no runtime or full CPU audit.
An unchecked historical debt is not proof of a current defect or of repair.

| Original item | Evidence and disposition |
| --- | --- |
| Shared package vocabulary | `docs/rules/DOCUMENT.md` still names `src/{lib,common,x86}`; actual roots are Lib, Emulator and X86 packages. Valid Shared-governance issue; do not edit Shared in NXVM Td. |
| Shared artifact wording | Execution still contains historical machine-key and generated INI paragraphs, alongside later four-App cutover; Shared owner should reconcile without changing product files. |
| Internal descriptor vocabulary | No current caller inventory or proven behavioral defect; retain low-priority review, do not queue a rename. |
| Debug control-register mutation | `src/x86-core/cpu/cpu.c` debug patch writes CR0/CR3 into the candidate before segment checks; guest MOV CR has its own checks in `cpu_instructions.c`. Raw-override policy and implications require a separate contract review; not proof of an actual user failure. |
| Broader x87 | No selected 287/387 complete numeric corpus has been qualified here; future selected-coprocessor scope, not an automatic defect in the current no-FPU profiles. |
| Other virtual-interrupt extensions | No newly selected PVI CPU consumer shown; retain conditional research, not an advertised current feature. |
| CPU fault reporting | No newly reproduced missing outcome in this static review; retain pending reproducible case. |
| Assembler checkop | No demonstrated defect or admission trigger; retain conditional item. |
| Unselected peripherals | Named device/board source contracts must be selected first; existing PC110 queue owns its selected hardware, other speculative features remain conditional. |
| Model40 x86 throughput | Historical 180-second host-specific finding is not reproduced in this audit. T564 S5 x86 floppy boot passed, but is not the old Turbo DOS-install terminal; retain pending like-for-like evidence and existing guest-qualification candidate. |
| Beyond digital timing | Scope-dependent research; no basis for claiming pin/analog accuracy or a present selected-feature defect. |
| Verifier-negative runtime | `test/x86-nxvm/CMakeLists.txt` now sets 300 seconds. The old 30-second account is stale; efficiency and retained assertions remain a possible future review, not a current timeout claim. |
| PC App test adapter | Direct `machine_private.h` includes exist across all four App test trees. Queue a bounded ownership audit at the tail without asserting each include is wrong. |
| Machine wake failure | `emulator_machine_resume`, `stop`, `reset` and debug cancel still ignore some event-signal results in `src/emulator/machine/machine.c`; other paths check failures. The potential contract issue needs a Shared failure-injection review, not a claim of observed production failure. MyNES's first candidate already covers Shared async wake review. |
| Native desktop isolation | Historical modal failure is not diagnosed; T564 S5 had another sandboxed modal failure which passed in a real session. Existing MyNES Shared-verification candidate includes modal isolation. Do not conflate this with MyNES Escape pause intermittency. |
| Console failure rollback | T538 observation is historical; latest imported Console has changed and no identical injected rollback reproduction was run here. Retain for separate Shared review. |
| Browser storage | Future browser consumer not selected; retain conditional capability, not a PC failure. |
| Direct strict compilation | `src/x86-core/CMakeLists.txt` retains targeted warning exclusions. This proves exclusions exist, not that all source violates warnings or that prior `-w` wording remains current. Retain targeted audit. |
| Linux runtime | No Linux execution was performed; source portability is not runtime qualification. Retain pending suitable environment. |
| DevBox | No explicit product goal or trust boundary; retain exploratory item, no implementation queue. |

The existing CPU differential and four-machine guest qualification candidates
remain in their original order; their current layout and INI descriptions were
updated. The PC App test encapsulation candidate was appended. All other items
remain conditional or unproven and are not duplicated as new NXVM candidates.
The Shared async-wake and desktop issues also have a MyNES queue receiver.
No production or Shared file was modified. A structural documentation gate
does not prove absence of defects, hardware correctness or detailed Shared
failure semantics; these limitations are dispositions, not Td blockers.

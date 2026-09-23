# M5 T358 S4: Paging And Linear-Memory Coherence

## Audit Boundary And Search

This S audits the existing 80386 non-PAE, 4 KiB paging mechanism as a
validation-to-publication path. Intel 80386 paging and page-fault semantics are
the architectural authority; T258, T311, T325, T329, and T320 supply retained
owner evidence. The audit does not infer later-CPU write-protect, TLB,
`INVLPG`, PAE, large-page, physical-wait, or device-time behavior.

The source sweep used:

```text
rg -n "_kma_(prepare|commit|physical)_linear|_kma_(read|write|test)_(linear|logical)|_m_(read|write|test)_(access|logical)|VCPUINS_EXCEPT_PF|cr2" src/core/machine/cpu_instructions.c src/core/machine/cpu.c src/core/machine/cpu.h
rg -n "core_machine_cpu_execution_(read|write)_linear|_kma_test_(access|logical)" src tests
rg -n -i "paging|page fault|accessed|dirty|CR2|translation" tests/machine/core_machine_80386_paging_smoke.c tests/machine/core_machine_task_switch_smoke.c tests/machine/core_machine_vm86_delivery_smoke.c
```

## Production Decision-To-Publication Ledger

| Route | Validation and private state | Publication/fault boundary | Disposition |
| --- | --- | --- | --- |
| `_kma_prepare_physical_linear` | Reads PDE then PTE, checks present and 80386 CPL3 U/S and R/W rules, calculates a private physical result, and sets `CR2` plus the exact `#PF` error code on rejection. | No A/D or guest-memory publication before a successful prepare. | Sole translation validator; retained 80386-specific permission rules are intentional. |
| `_kma_commit_physical_linear` | Consumes only a successful private translation. | Marks PDE/PTE accessed and PTE dirty for writes; preview does not mutate A/D. | Sole A/D publisher. |
| `_kma_read_linear` / `_kma_write_linear` / `_kma_test_linear` | A cross-page request prepares both page fragments before either fragment commits. | Both A/D images commit only after both translations pass; read/write then reaches the two physical fragments. | Common cancellation contract. T311 cross-page read, write, stack, and instruction-fetch probes retain it. |
| Guest operands, stack, strings, and port-string memory | `_m_read_logical`, `_m_write_logical`, and `_m_test_access` carry `_GetCPL`; `_GetCPL` maps VM86 to privilege 3. | Guest data/register publication remains in the instruction handler after the checked access; an access fault enters `ExecFinal` with old CPU state and recorded `CR2`. | One common guest route; 80386 paging owner smoke covers data/stack/string-shaped access and T320 covers VM86 delivery. |
| Instruction fetch | `ExecInit` fetches up to 15 bytes through `_kma_read_linear` with current effective privilege. | Fetch `#PF` precedes decode/handler publication and supplies the current instruction restart point. | Common code-fetch route; T311 cross-fetch and T320 VM86 paging-delivery evidence retain it. |
| Descriptor, IDT, LDT, TSS, task, gate, and return memory | Kernel helpers use the same logical/linear route with force/CPL0 where architectural system access requires it; transition preflight uses `_kma_test_access` when a real memory access must be validated before a commit. | Descriptor/table or stack publication occurs only after its local preflight; page faults use the existing finalizer and frame serializers. | T329 task-paging and S3 transition evidence cover adopted system callers. |
| `_kma_test_logical` uses | The three uses are zero-byte segment-limit/state feasibility checks (`_s_test_esp`, incoming TSS stack state, and LEAVE preflight); they intentionally do not claim a physical read or page translation. | The later real fetch/pop/read remains the page-translation publication point. | Intel-required distinction between segment feasibility and a memory access; no duplicate page-walk path. |
| External linear read/write helpers | `core_machine_cpu_execution_read_linear` and write counterpart route through the common walker at forced CPL0. | They preserve/restore pending instruction exception state for fixture/debug observation. | No guest instruction caller; retained test/debug boundary, matching the existing debugger-control TODO. |

## Reproduced Outcomes And Evidence

No accidental construction divergence was reproduced. The focused owner run
emits all retained paging markers and proves: absent code/data/stack pages,
exact `CR2` and `#PF` codes, CPL3 U/S and R/W combinations, 80386's retained
no-WP policy, PTE/PDE A/D publication only after success, cross-page
read/write/stack/fetch atomicity, CR3 reload, and reset.

`core-machine-vm86-delivery-smoke` additionally proves VM86 effective-CPL3
page-fault delivery and the saved delivery state. `core-machine-task-switch-
smoke` proves a 32-bit task image can select its `CR3` and that a TSS paging
failure remains an exception boundary, rather than a second translation
implementation.

## Similar-Issue Sweep And Transfers

All production hits named in the search are represented by the ledger. No
second page-walk, direct A/D writer, or guest bypass was found. The following
remain intentionally outside S4:

- persistent translation caching, `INVLPG`, PAE, large pages, and later-CPU
  paging controls;
- debugger raw control-register mutation, already tracked in `TODO(Medium)`
  as a separately admitted API/ownership decision;
- physical-memory provider failures, wait states, bus arbitration, and device
  service timing, which transfer to the ordered bus-timed and device-service
  Queue candidates.

## Current-Gate Scheduling Reconciliation

The configured default gate uses four concurrent CTest jobs. Its existing
`vm-timer-firmware-smoke` failed twice only in that parallel schedule at its
host-side pause watchdog (stage 5), while the identical isolated test passed.
The guest marker and timer assertions were unchanged. A same-shape sweep of
tracked current targets using `CreateThread` found the timer, DOS prompt,
keyboard, memory-fault, runner display cadence, debug-pause, and unified-debug
session smokes. CMake now marks that exact current-target set `RUN_SERIAL` and
asserts every member remains in the canonical current gate. This changes only
CTest scheduling metadata: no guest tick, timer, session, media, device, or
assertion behavior changed. The default parallel gate must pass after fresh
configuration to accept this reconciliation.

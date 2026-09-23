# T325 S3: Intel 80386DX Paging And Translation Closure Audit

## Scope And Authority

This record closes T325's intrinsic Intel 80386 non-PAE paging and
translation package. It reconciles the CPU executor with Intel 80386 PRM
[page translation](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s05_02.htm),
[page protection](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s06_01.htm),
[page faults](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s09_08.htm),
and [control-register forms](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/MOVRS.htm).
It does not implement a persistent translation cache, a later-CPU `INVLPG`,
PAE/PSE/CR4, task switching, or VM86 paging-family breadth.

## Requirement-To-Evidence Ledger

| Intel 80386 requirement | Executor route | Focused evidence and disposition |
| --- | --- | --- |
| `PG` selects optional 4-KiB paging and may not be enabled while `PE=0`. | `_s_write_cr0_80386`; `_kma_prepare_physical_linear`. | T325 S1 proves the mutable 80386 CR0 image (`PE`, `MP`, `EM`, `TS`, `ET`, `PG`), rejects `PG=1,PE=0`, and removes the later-CPU `CR0.WP` rule. |
| CR3 selects a 4-KiB-aligned page directory; CR2 retains the faulting linear address. | `_s_write_cr3_80386`; `MOV_R32_CR`; `MOV_CR_R32`; page-fault branches. | T304 retained control-form proof plus T325 S2's two guest-selected directories and delivered `#PF`/CR2 frame evidence. |
| A current walk uses PDE then PTE; P=0 faults and P/W/U reports not-present versus protection, read versus write, and user versus supervisor. | `_kma_prepare_physical_linear`. | T258/T311 real guest fetch/data/stack probes cover present, PDE/PTE U/S and R/W, `CR2`, exact error codes, and producer nonpublication. |
| Accessed is set at both levels before a completed access; leaf dirty is set for a completed write; failed split access has no premature A/D publication. | `_kma_prepare_physical_linear`; `_kma_commit_physical_linear`; split read/write/test paths. | Retained valid, permission, and cross-page probes check A/D state. T311 S4 proves two-page data, stack, and fetch preflight/commit atomicity. Directory dirty is not asserted because it is undefined on the 80386. |
| A page fault is restartable and can deliver vector 14 with its error-code frame. | `_SetExcept_PF`; `ExecFinal`; protected interrupt serializer. | T321 S2 and the retained paging delivered-fault vector prove saved restart EIP, error code, CR2, target handler transfer, and producer GPR preservation. |
| The processor may cache translations; system software flushes by CR3 reload or a task switch. | `_kma_prepare_physical_linear`; `_s_write_cr3_80386`. | The executor deliberately has no persistent cache: each access rereads PDE/PTE. T325 S2 proves a guest CR3 directory change and a host PTE edit between two guest accesses without a CR3 reload. This is a functional no-cache model, not timing, TR6/TR7, or task-switch-cache implementation. |
| `INVLPG` is not an Intel 80386 instruction. | `INS_0F`; `core_machine_cpu_instruction_metadata_get`; `INS_0F_01`. | T325 S3 adds `0F 01 /7` real-mode vectors for 80186/80286/80386, plus 80386 `66`, `67`, combined, and `LOCK` forms. Each has `#UD(0)` with the complete CPU snapshot unchanged. The `0F` metadata rejects `/7` before `INS_0F_01` and before the table-memory effective-address decoder, so no source access is attempted. 8086 is excluded: its `0F` byte is legacy `POP CS`, not an escaped form. |

## Similar-Issue Sweep

The audit reviewed all CR0/CR2/CR3 writers and readers, the PDE/PTE prepare
and commit paths, page-span split helpers, `#PF` producer/delivery evidence,
the `0F` metadata and `/7` branch, and current paging smoke registrations.
No stale-map state, cache generation, `INVLPG` handler, or page-walk owner
outside the CPU executor exists. No production change is required by S3.

## Required Transfers

- **Protected-mode `#UD` vector-6 delivery:** T325 S3 reproduces that
  `ExecFinal` delivers `#UD` through an IDT only for VM86. Ordinary protected
  mode reaches the later terminal `#GP` path even with a valid vector-6 gate.
  This is a shared exception-delivery defect, not a paging repair; it is
  recorded as `TODO(High)` with a complete producer-sweep admission condition.
- **Task CR3 and page-fault semantics:** task switches can load CR3 and have
  distinct fault contexts. They transfer to the Queue's 80386DX task/LDT/debug
  and VM86 system-state package.
- **VM86 paging breadth:** VM86 instruction/state coverage and page-fault
  composition transfer to that same Queue system-state package; T320's bounded
  delivery foundation is not a paging-family closure.
- **Persistent TLB/TR6/TR7:** the present executor has no cache consumer.
  A later implementation must admit cache lifetime, CR3/task-switch flush,
  test-register behavior, and deterministic invalidation evidence together;
  neither this no-cache proof nor `INVLPG #UD` is a substitute.

## Closure Boundary

T325 therefore closes the native CPU page-walk, protection, diagnostic, and
functional no-cache requirements. It does not claim task switching, VM86
paging breadth, protected `#UD` delivery, a physical TLB, post-80386 paging,
or Windows compatibility.

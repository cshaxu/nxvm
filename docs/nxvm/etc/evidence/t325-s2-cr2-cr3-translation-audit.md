# T325 S2: Intel 80386 CR2, CR3, And Translation Audit

## Authority And Scope

This evidence closes only the T325 S2 CR2/CR3 and no-persistent-cache slice.
The Intel 80386 form authority is [MOV special registers](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/MOVRS.htm),
[page translation](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s05_02.htm),
[page fault](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s09_08.htm),
and [TLB testing](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s10_06.htm).

The forms are fixed-width, register-only 80386 `MOV r32,CR2/CR3` and
`MOV CR2/CR3,r32`. They are valid in real mode and protected CPL0, reject
protected CPL3 and VM86 with `#GP(0)`, and do not gain a narrower operand from
the operand-size prefix. CR2 records the linear address of a page fault. CR3
selects the current page directory; a hardware 80386 CR3 reload flushes its
translation cache.

S2 excludes PAE, PSE, CR4, `INVLPG`, TR6/TR7, task-switch CR3 images, and a
persistent TLB implementation. Those are not silently represented by the
functional no-cache executor.

## Current Route And Similar-Issue Sweep

`MOV_R32_CR` and `MOV_CR_R32` are the only guest control-register form
handlers. `_d_modrm_creg` enforces register-only decoding, both handlers check
CPL before selecting CR0, CR2, or CR3, and `_s_write_cr3_80386` retains the
4-KiB alignment boundary. The page walker `_kma_prepare_physical_linear`
reads the current PDE and PTE from physical memory for every access; its
per-access candidate expires before the next access. Searches found no cache
entry, cache generation, stale-map consumer, flush operation, or `INVLPG`
route in the executor, reset, tracing, fixture, or build paths.

This is a functional no-persistent-TLB model: a page-table edit is visible on
the next access, and a guest CR3 reload changes the selected directory
immediately. It must not be described as a timing or TR6/TR7 implementation.
If a later task introduces cached translations, it must admit both the CR3
flush and the Intel 80386 test-register contract rather than relying on this
no-cache proof.

## Focused Proof

| Requirement | Evidence |
| --- | --- |
| CR2/CR3 forms | Retained `core-machine-descriptor-system-smoke` exercises symmetric CR2/CR3 reads and writes, reserved-control and memory-ModRM `#UD`, profile rejection, and protected CPL3 `#GP(0)` before control publication. |
| CR3 directory selection | T325 extends `core-machine-80386-paging-smoke` with two physical page directories. Guest `MOV CR3,EAX` selects directory one and reads `0x11223344` at linear `0x3000`; guest `MOV CR3,ECX` selects directory two and the next access to the same linear address reads `0x55667788`. CR0/CR3, EIP, flags, participating registers, and all nonparticipating GPRs have exact assertions. |
| No stale translation | A separate two-step vector edits the currently selected PTE between two guest dword reads of the same linear address, without reloading CR3. The second read obtains the replacement physical value and observes the replacement PTE accessed bit. Together with the current-PDE/PTE source sweep, this proves the executor has no persistent translation that can override a page-table edit or the selected CR3 directory. |
| Page-fault diagnostic | Retained T258/T311 probes verify `#PF` P/W/U codes and CR2 publication; the retained delivered-page-fault fixture verifies the exception frame and handler boundary. |
| Later boundaries | Persistent TLB timing, `INVLPG`, TR6/TR7, task CR3 loading, and VM86 paging breadth remain named transfers, not completion claims. |

The owner smoke remains target-local GCC strict. S2 adds no public API,
translation-cache data structure, exception policy, or product behavior outside
the direct CR2/CR3 and checked page-walk routes.

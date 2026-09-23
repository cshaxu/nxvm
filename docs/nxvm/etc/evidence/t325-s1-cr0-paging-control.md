# T325 S1: Intel 80386 CR0 And Paging-Control Correction

## Authority And Scope

This evidence closes only the T325 S1 CR0/page-protection slice. The Intel
80386 Programmer's Reference Manual is the form authority: its control-register
figure defines `PE`, `MP`, `EM`, `TS`, `ET`, and `PG`; its page-protection
rules state that supervisor accesses are readable and writable regardless of a
page entry's R/W bit. The relevant public reproductions are
[control registers](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s04_01.htm),
[paging tables and A/D state](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s05_02.htm),
and [page-level protection](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s06_04.htm).

`CR0.WP` is an Intel 80486-era control and is not an 80386 page-protection
input. The retained T311 document's former WP-specific assertion is therefore
historical evidence superseded by this correction; it is not an 80386
completion claim.

## Owner And Correction

`_s_write_cr0_80386` remains the only guest `MOV CR0,r32` write validator.
It now permits the 80386-defined `MP`, `EM`, `TS`, and `ET` controls in
addition to its retained `PE`/`PG` transition policy. Intel 80386
[section 11.1.1](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/s11_01.htm)
explicitly permits setting or resetting ET with `MOV CR0`; the prior ET
rejection was therefore removed rather than preserved as a configuration
boundary. `MOV CR3,r32` remains separately aligned and unchanged. The two
CPL0 branches in `_kma_prepare_physical_linear` that had consulted the
nonexistent 80386 `WP` bit are removed; CPL3 continues to apply the combined
PDE/PTE U/S and R/W checks.

No public interface, page-table format, exception mapping, TLB/test-register
state, task switch, or x87 provider contract changes. PE-clear/re-entry is a
separate state-transition transfer, not inferred from this control-bit slice.

The local caller/read sweep found one guest CR0 writer (`MOV_CR_R32` through
`_s_write_cr0_80386`) and the two former CPL0 `WP` branches, both in the
80386 page walker. The retained `CLTS` route and the S65 `WAIT`/ESC consumers
of `MP`/`EM`/`TS` were reviewed as downstream consumers only: their semantics
and interfaces are unchanged, while this S proves those control bits can now
be established architecturally through `MOV CR0`.

## Focused Proof

The retained `core-machine-80386-paging-smoke` now has target-local GCC
`-Wall -Wextra -Wpedantic -Werror` and proves:

| Requirement | Prepared-state proof |
| --- | --- |
| `MOV CR0` mutable controls | A real-mode 80386 guest writes and rereads `MP|EM|TS|ET`; CR0, EAX, and ECX publish exactly those bits while EIP, FLAGS, and all nonparticipating GPRs retain their expected state. |
| Privilege boundary | Retained `core-machine-descriptor-system-smoke` exercises protected CPL3 `MOV` control-register rejection before any control publication; T325 does not weaken that shared decoder/privilege boundary. |
| CPL3 paging protection | Existing fetch/read/write/stack vectors preserve U/S and R/W `#PF` codes 5 and 7, CR2, producer nonpublication, and successful user publication. |
| CPL0 page writes | Read-only PDE and PTE variants both complete a supervisor write even when the former synthetic bit 16 is set directly in the fixture CR0 image; the write and A/D publication follow the normal successful path. |
| Retained translation behavior | Existing T258/T311 valid walk, present faults, delivered `#PF`, A/D, and cross-page data/stack/fetch probes continue to pass. |

The full S1 gate records the exact current-gate and specialized-verifier
results at acceptance. Paging cache/test-register behavior, task CR3 loading,
and PE-clear/re-entry remain named later transfers.

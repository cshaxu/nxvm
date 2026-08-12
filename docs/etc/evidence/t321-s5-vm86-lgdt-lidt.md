# M5 T321 S5: VM86 LGDT/LIDT Privilege Boundary

T319 intentionally closed only non-VM86 `LGDT m` and `LIDT m`.  Its local
`INS_0F_01` predicate rejected protected CPL>0 only when VM was clear, allowing
VM86 `/2` and `/3` to decode and read their pseudo-descriptor source.

S5 changes only the two local checks: in protected execution, either VM86 or a
nonzero CPL causes `#GP(0)` before `_d_modrm_table_memory` and `_m_read_rm`.
`/0`, `/1`, `/4`, `/5`, `/6`, and `/7` are unchanged; no table serializer,
TSS planner, VM86 entry mechanism, or public/provider interface changed.

`core-machine-vm86-lgdt-lidt-s5-smoke` reuses the accepted T320 VM86
GDT/IDT/TSS fixture and independently exercises both `/2` and `/3`.  It
installs the vector-13 CPL0 interrupt gate and a distinguishable six-byte
source.  Each form delivers `#GP(0)` with restart EIP zero, retains the source
and both GDTR/IDTR images, preserves all non-stack GPRs, and writes the exact
ten-dword VM86 error frame: error code zero; EIP zero; VM86 CS/FLAGS/ESP/SS;
then ES/DS/FS/GS.  The handler HLT reaches 0101h.

The audit retains T319's real/protected non-VM86 and consumer matrix unchanged.
VME/PVI, task switching, broader VM86 instruction behavior, descriptor-table
refactoring, paging, and generic exception policy remain outside S5.

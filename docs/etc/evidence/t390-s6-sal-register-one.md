# T390 S6: Model-40 SAL Register-One Qualification

`M5:T390:S6:SAL-REGISTER-ONE:OK`

## Semantic Identification And Containment

S6 adds an opt-in `--terminal-bytes` diagnostic to the explicit external BYOB
capture helper.  It is disabled by default and retains only the first
source-unallocated terminal point in process memory.  When explicitly enabled,
it emits that one temporary local record; the owner-managed harness deletes its
output after the process exits.  No firmware/media identity, raw byte record,
path, digest, provenance or guest trace is retained in Git.

The contained diagnostic identifies S5's first terminal as real-address,
CPL-0, no-prefix, 16-bit register `SAL r/m8,1` (Group-2 D0 `/4`, register
ModR/M).  That semantic identity is an instruction form, not a retained
firmware payload.

## Exact Core Row

Intel's 1986 [80386 Programmer's Reference Manual, Table 8-1](https://datasheets.chipdb.org/Intel/x86/386/manuals/386intel.pdf)
lists non-carry shift/rotate by one as `3/7` clocks for register/memory.  S6
adds only the observed register `SAL r/m8,1` form to the sole 80386 Core source
ledger at 3 clocks.  The row remains CPU-core time under the table's
prefetched/no-wait/no-HOLD assumptions; it is not a ROM, memory, ISA, device,
board or physical-L3 time claim.

The classifier requires no prefixes, opcode D0, Group-2 extension `/4`, and a
register ModR/M form.  D0 memory forms and all other Group-2 extensions remain
source-unallocated and are rejected before elapsed/device publication under
the existing physical contract.  The focused ledger smoke proves the exact
3-tick register result and physical admission, plus physical rejection with
zero executed/elapsed ticks for the memory counterpart.

## Bounded Replay And Transfer

A fresh contained default replay has no terminal-byte output.  It observes the
qualified `sal-register-one` row at 3 ticks, then reaches 122 successful
retirements with 121 classified rows and stops at the next source-unallocated
real-mode `cld` form.  It reaches neither the `0:7C00` boot-sector checkpoint
nor a physical profile selection.  Temporary output is deleted after process
exit.

T390 transfers `CLD` semantic/timing disposition to its next bounded
continuation.  Physical retirement selection, board timing, device timing and
Model-40 L3 remain open.
## Coordinator Acceptance

Coordinator actual-change review accepts P1 `a4d508ea`.  It confirms the exact
Table-8-1 register row has one Core owner, the memory counterpart remains
nonphysical, default capture does not emit terminal bytes, owner-managed raw
output remains external, and the evidence truthfully transfers `CLD` before
any physical profile, board timing or L3 claim.  Full current gates,
documentation governance and diff hygiene pass.

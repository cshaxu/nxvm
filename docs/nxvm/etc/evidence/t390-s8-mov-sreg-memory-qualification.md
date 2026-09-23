# T390 S8: Model-40 Memory-Source MOV Sreg Qualification

`M5:T390:S8:MOV-SREG-MEMORY:OK`

## Exact Core Row

Intel's 1986 [80386 Programmer's Reference Manual MOV table](https://datasheets.chipdb.org/Intel/x86/386/manuals/386intel.pdf) lists `8E /r MOV Sreg,r/m16` as `2/5, pm=18/19` clocks (page 344). S8 adds only the observed no-prefix real-address memory-source form to the sole private 80386 Core ledger at five ticks. The existing register source remains two ticks; protected-mode memory forms remain source-unallocated because their distinct `18/19` rows are outside S8.

The focused timing ledger accepts the memory form physically at five ticks and preserves the register form at two. The classifier selects the new row only when the instruction is memory-source and the existing control-stack predicate says real address mode; all prefixes remain rejected at the prepublication boundary.

## Bounded Replay And Transfer

A freshly rebuilt owner-managed capture helper uses the normal Model-40 startup-media route, produces no terminal-byte output, and observes the memory-source form classified at five ticks. The bounded replay reaches 18,228 successful retirements then transfers one next source-unallocated normalized terminal. It reaches neither `0:7C00` nor a physical profile selection. No external asset identity, path, hash, provenance, byte record or trace enters Git; temporary output is deleted after the run.

## Verification

Focused timing-ledger smoke, product/current-gate build, documentation governance and diff hygiene pass.

## Coordinator Acceptance

Coordinator actual-change review accepts P1 c1021cef and corrective P2 86632441. P2 directly proves the same memory-source encoding under protected mode faults before retirement, elapsed-time or device publication. The exact real-address row, existing register row, prefix boundary, source containment and bounded terminal transfer remain correct. No physical profile, board timing or L3 claim is accepted.

# T390 S7: Model-40 CLD Qualification

`M5:T390:S7:CLD:OK`

## Exact Core Row

Intel's 1986 [80386 Programmer's Reference Manual, CLD entry](https://datasheets.chipdb.org/Intel/x86/386/manuals/386intel.pdf) gives opcode `FC` `CLD` an exact `2`-clock row (manual page 283). S7 adds that row only to the existing private 80386 Core source-timing ledger and selects it only after the existing no-prefix check. It is Core time under the manual's published assumptions, not a board, device, ROM, media, ISA or L3 duration.

The focused timing-ledger smoke proves normal `CLD` retires with two ticks and one execution-provider advance. Its physical-boundary proof accepts that exact no-prefix form at two ticks, while an otherwise equivalent segment-prefixed form faults before execution, elapsed-time or device publication. The existing classifier therefore remains the sole owner; no Core public ABI, VM/profile contract or test-only production interface is added.

## Similar-Form And Containment Sweep

The S7 sweep covers the tracked Core timing enum, 80386 ledger/classifier, timing-ledger smoke and the distinct 80286 flag-control path for `0xfc`, `CLD`, and adjacent flag-control forms. The observed 80386 no-prefix `CLD` is the sole S7 change. Adjacent unqualified 80386 flag controls remain source-unallocated and retain the existing physical prepublication rejection; the 80286 ledger is a different CPU owner and remains unchanged.

The owner-managed boot replay uses the existing normal Model-40 1.2 MB startup-media route. It runs with the default diagnostic: no raw terminal bytes, firmware/media identity, path, digest, provenance or trace is retained. The bounded, cleaned-up replay classifies `CLD` at two ticks, reaches 426 successful real-address retirements, and transfers the next first source-unallocated form as a memory-source `MOV Sreg,r/m16`. It reaches neither the `0:7C00` checkpoint nor a physical profile selection. This is a corpus-progress observation only; it establishes no board timing or L3 claim.

## Verification

The focused timing-ledger smoke, product build, full current-gate build, documentation-governance gate and diff hygiene pass. The capture runner is explicitly rebuilt before the external replay so the replay uses the S7 Core row; temporary output is removed after the process exits.

## Coordinator Acceptance

Coordinator actual-change review accepts P1 `b6deb9c4`. It confirms one private Core owner for the exact no-prefix CLD row, normal two-tick and physical/nonpublication regressions, preserved prefixed rejection, a rebuilt redacted external replay, and no external asset or trace in Git. The replay truthfully transfers only memory-source MOV Sreg,r/m16; no physical profile, board timing, boot-sector or L3 claim is accepted. Full current gates, documentation governance and diff hygiene pass.

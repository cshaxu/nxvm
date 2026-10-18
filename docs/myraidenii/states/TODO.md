# Unplanned Debt

## Resolved: Lossless Graphics Publication Contract

### Original T1 Finding And Disposition (Superseded)

Original owner: Lib KVM Window's graphics contract, with Emulator and product Machine
consumers. T1 S1 confirmed that `kvm_window_graphics_frame` has 8-bit indices
and 256 palette entries, whereas the selected board reference has 2048 palette
entries. Dimensions fit; the number of distinct simultaneous colours is not
proven to fit. Existing PC/NES qualification does not qualify this board.

Risk: a complete game frame cannot be guaranteed to cross the current contract
without lost colours. This is a public output-contract limitation, not a reason
to truncate palette indices, quantize colours or add a second presenter.

Retained proposal, NOT approved: one canonical `lib_u32` 0x00RRGGBB graphics
pixel contract, removing the host indexed-palette path. Guest palette state
would remain in each Machine/Core owner; text would be unchanged. Review Lib
frame validation/copy/render, Emulator receiving tests, PC/MyNES adapters,
memory/copy costs and complete dual-width consumer/artifact qualification.

Admission condition: explicit owner approval of a later Shared change after
the concrete interface, affected files, before/after semantics and diff estimate
are reviewed. On 2026-10-17 the owner declined approval, required no Lib change
and directed this proposal into TODO. Do not implement it under the standing
MyRaidenII goal. It remains a required full-colour release gate, not waived
by finishing CPU, board or indexed test frames. Other independent work proceeds.

### Resolution Through Accepted Shared Work

On 2026-10-17 the owner separately admitted and accepted NXVM T564 S1:
Shared `d30b69b2a`, MyNES `906e61b0c`, NXVM `b9433b31d`, closure `53e1e775d`.
That complete import provides the proposed RGB32/four-slot contract, with full
receiving dual-width proof and ten verified EXEs. This supersedes the original
non-approval above; the host palette-capacity gap is resolved without any
MyRaidenII-local Lib modification. See the
[accepted import evidence](../../nxvm/etc/evidence/t564-s1-shared-frame-import.md).
Actual RAIDEN II board palette, pixels, rotation and publication are not yet
implemented or qualified; their video task remains a required release gate.

V30, COP, video/sound and host integration remain planned Queue work, not debt
silently substituted for their implementation proposals.

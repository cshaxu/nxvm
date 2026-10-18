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

## Medium: Selected-SX System-Event Completion Metadata Review

Owner: Shared X86 Core selected-80486 SMM/STPCLK event-prefix accounting.
S29's completion sweep finds two different publishers in
`cpu/cpu_instructions.c`: `_execution_refresh()` after `_smm_enter()` and
`_system_boundary()` after interrupt arbitration during STOP-GRANT release.
They can set the no-retirement delivery flag after an attempt which has also
stopped CPU execution. Unlike ordinary HALT wake, these selected-SX paths own
accepted-prefix/event_ticks accounting and existing source/L2 qualifications.

Risk: the flag's successful-delivery meaning may diverge on these failed
system-event paths. No current guest failure is established: Machine reports
the fault before consulting that flag, and V30 cannot enter either SX-only
path. Do not change their event time, accepted-prefix work or shutdown policy
merely to make their Boolean resemble the ordinary HALT path.

Admission: the next explicitly admitted Shared CPU completion/failure audit,
or before an actual consumer relies on this flag after a failed SX system
event. Reproduce failed SMI and STOP-GRANT interrupt entry through their real
providers, pin existing failed-prefix/time expectations, then qualify the
result contract on both widths. This is outside S29's ordinary V30/HALT
result batch and is not a replacement for planned V30 physical timing work.

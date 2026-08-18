# M5 T402: DeskPro 386 D4 Memory-Controller Reference Reconciliation

## Task Record

T402 is the owner-authorized successor to the four-profile CPU audit. It
reconciles the original 1986 DeskPro 386/16 D4 memory-control surface with the
existing private Model-40 implementation using original D3PE material first
and 86Box only as explicitly labelled secondary reference behavior. It is a
functional hardware-baseline task and cannot establish physical timing or L3.
## S1 Acceptance

P1 `1c41bc53` implements the complete admitted functional D4 matrix.  It keeps
D4 ownership inside the Model-40 VM profile, expands `80C00000h` from the
incorrect one-byte exposure to the Tier-2-labelled four-register, 4 KiB
nonresponsive reference window, and adds focused coverage for the diagnostic
high byte, setup word and ignored page byte.  Original D3PE mapping/control
facts remain primary; reference-only subforms remain labelled in the indexed
[evidence](../etc/evidence/t402-s1-d4-memory-controller-matrix.md).

Focused D4 tests pass, the full current CTest gate passes 285/285, and
documentation governance passes.  The actual change review confirms no Core or
public Core/VM interface change.  S1 is accepted as functional D4 progress;
physical D4/cache/DRAM/ISA timing and L3 remain transferred to the later
board/device timing receiver.

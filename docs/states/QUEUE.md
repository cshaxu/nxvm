# Queue

This is the ordered, unnumbered candidate queue. It does not activate work,
reserve a numeric task identifier, or define a technical baseline.

Each candidate links to its proposal. The shared admission and program context
is retained separately so this queue remains a short ordering surface.

## M5 Candidates

The [four-profile CPU-completeness program](../proposals/m5-four-profile-cpu-completeness-program.md)
is the common admission contract for this sequence. The order is intentional:
each later candidate consumes the earlier state-machine and profile evidence;
the final candidate is verification only and may return a missing row to its
proper implementation owner.

1. [8086 and 80186 profile closure](../proposals/m5-8086-80186-profile-closure.md). Close the real-mode baseline and 80186 extensions, including the 8086/80186 `LOCK` legality matrix.
1. [80286 protected-mode and descriptor closure](../proposals/m5-80286-protected-mode-closure.md). Close 80286 descriptors, gates, transfers, and the 80286 `LOCK` legality matrix on the now-stable shared delivery basis.
1. [80386DX extended instruction and system-state closure](../proposals/m5-80386dx-extended-state-closure.md); [80386DX admission policy](../proposals/m5-80386dx-candidate-policy.md). Close 32-bit, VM86, paging/debug, and residual system-state rows without reclassifying older-profile behavior.
1. [Four-profile cross-closure verification](../proposals/m5-cpu-profile-cross-closure.md). Reconcile the completed ledgers and state transitions; it may not substitute a smoke count for an unresolved implementation row.
1. [Windows 3.x readiness map](../proposals/m5-windows-3x-readiness-map.md).
1. [M5 closure audit](../proposals/m5-closure-audit.md).

## M6 Candidates

The [M6 mantle experiment program](../proposals/m6-mantle-program.md) applies
to every candidate in this section.

1. [Pre-decode transition gateway and mantle probe](../proposals/m6-predecode-transition-gateway.md).
1. [Ordinary-RAM transaction for staged runtime input](../proposals/m6-ordinary-ram-transaction.md).
1. [Transition-local combined commit](../proposals/m6-transition-local-combined-commit.md).
1. [Minimal mantle single-session closure](../proposals/m6-minimal-mantle-single-session.md).

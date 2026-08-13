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

The [80286 closure context](../proposals/m5-80286-protected-mode-closure.md)
and [80386DX closure context](../proposals/m5-80386dx-extended-state-closure.md),
together with the [80386DX admission policy](../proposals/m5-80386dx-candidate-policy.md),
are shared planning references for the corresponding candidates below; they do
not allocate a numeric task.

1. [80286 descriptor-table and protected-transfer closure](../proposals/m5-80286-descriptor-transfer-closure.md). Reconcile the remaining 80286 `0F` descriptor/table forms, selector and gate transfers, 16-bit protected stack/frame rules, and their validation-to-delivery boundaries.
1. [80286 `LOCK` and profile closure](../proposals/m5-80286-lock-profile-closure.md). Establish the complete 80286 legal-versus-illegal `LOCK` matrix, then reconcile every inherited and 80286-only form/mode row before 32-bit work can rely on it.
1. [80386DX width, prefix, and integer-form closure](../proposals/m5-80386dx-width-integer-closure.md). Close all residual 32-bit operand/address, prefix, primary, and integer `0F` rows as form families, without borrowing an older-profile disposition.
1. [80386DX system, VM86, paging, and debug closure](../proposals/m5-80386dx-system-state-closure.md). Close the residual 80386DX state and privileged-form rows by their state owner: VM86/task interaction, paging/system control, and debug/breakpoint behavior.
1. [80386DX profile closure](../proposals/m5-80386dx-profile-closure.md). Reconcile the completed 80386DX form and state ledgers; return any missing implementation row to its owning candidate rather than closing it by audit.
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

# T390 S21: M40-C0 Attribution-Coverage Audit

## Corpus result

A fresh owner-managed, deleted normalized replay reaches the frozen M40-C0 protected-return terminal. It has 18,255 successful classified retirements, zero source-unallocated successes, 75 aggregate context rows, no fault/status terminal, and no retained asset identity, local path, hash, bytes, PC, or raw trace.

The S19 lookup identifier covers 22 aggregate rows and 918 successful retirements. Fifty-three aggregate rows and 17,337 successful retirements are classified but report `CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED`. The lookup-backed identifiers observed are 2, 3, 6, 7, 9, 10, 24, 25, 26, 27, 32, 73, 74, 76 and 78. These identifiers describe NXVM classifier selection only; they are not Intel citations.

## Owner sweep and disposition

`rg -n "core_machine_source_timing_lookup|core_machine_source_timing_mark_unallocated|\*out_ticks =" src/core/machine/machine.c` confirms one successful lookup owner, but also successful direct-calculation returns in the 80386 primary branch, secondary `0F`, privileged `0F`, string/I-O, dynamic arithmetic and control/stack classifiers. Such paths intentionally remain classified under T388, yet cannot receive S19's ledger-only identifier. The C0 result is therefore internally consistent: it proves successful classification, not a complete exact Intel-row attribution.

No tick value, opcode/group aggregate, or post-execution result may fill this gap by inference. In particular, the high-volume direct string/control and common primary forms require the classifier's selected source category and their already-required lexical, repeat, memory, privilege, branch-outcome or dynamic facts before any exact Intel row can be admitted.

## Receiver

The earliest receiver is one shared Core **classified timing-origin attribution** mechanism. It must reset with each cost attempt and allow every successful timing classifier--lookup-backed or direct--to select one opaque source-category ID before the copied observer publishes. It must retain the explicit unattributed sentinel for sources not yet mapped, preserve the existing no-raw observer boundary and T388 physical rejection, and expose no profile-local classifier or mutable Core state.

A later finite C0 matrix slice can use that origin plus its normalized contextual captures to map exact Intel-primary rows and regressions, or transfer an underdetermined class whole. This audit adds no timing row, source inference, physical retirement, C1 boot, DeskPro board/device timing, or L3 claim.
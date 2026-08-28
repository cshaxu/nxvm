# M5 T498 S3 Model-40 External-ROM Startup-Chain Ledger

`M5:T498:S3:MODEL40-CHAIN:SELECTED`

## Scope and method

This ledger consumes the accepted T498 S2 external-ROM replay.  The supplied
ROM pair and 1.2-MB DOS media remain owner-managed BYOB inputs: no file path,
hash, byte, or firmware-derived trace is retained here.  A semantic terminal
is not inferred from instruction count or a timeout.

The audit distinguishes the production startup chain from the test observer.
It changes no Core, VM, profile, controller, firmware, or media behavior.

## Chain disposition

| Segment | Sole owner | Direct current proof | Disposition |
| --- | --- | --- | --- |
| External ROM admission and interleaved carrier | VM Model-40 composition | The supplied pair is accepted by the existing BYOB manifest and the retirement capture enters the real reset stream. | Proved; no ROM import or profile shortcut. |
| 80386 reset address, ROM aliases, and A20 policy | Core checked memory; Model-40 selects immutable construction data | The retained S31 regression covers the selected A20 policy and external capture passes reset initialization into protected-mode work. | Proved for the reached route; ordinary reset-ROM routing correction remains separately governed below. |
| CPU transition and instruction execution before the storage consumer | Core CPU/memory/port transaction path | The external capture reaches the recorded protected-mode return checkpoint rather than an initial unallocated fetch. | Proved only through that checkpoint; later firmware control flow remains a receiver. |
| Copied display observation | Core VADP snapshot; VM presentation | S2 corrected the generic observer to treat an unavailable copied EGA snapshot as observation-unavailable, not a guest failure. | Ruled out as the startup cause; this does not claim visible EGA output. |
| Boot-policy/FDD command initiation | Firmware through Core ports | The longer S2 diagnostic sees no FDC command or terminal observation. | First unresolved consumer boundary.  Do not repair FDC, DMA, or PIC until firmware-to-port progression and its predecessor batch are distinguished. |
| 8272A, DMA2, and IRQ6 transfer/service | Core controllers | T386 S24 proves the selected logical FDC/DMA2/IRQ6 route with a production DMA transfer, but the external ROM has not invoked it in this replay. | Not implicated by current external evidence; retain as an available downstream route, not a cause. |
| FDD image bytes and persistence | VM media adapter | The same 1.2-MB media reaches a DOS terminal on the accepted default-at/80386 row. | Shared media format is ruled out; this does not prove Model-40 firmware has selected it. |

## First repair receiver

The selected next implementation batch is **Model-40 firmware-to-board
startup progression**, bounded from the first post-capture checkpoint through
the first boot-policy/FDD port request.  It must first obtain an observation
that distinguishes CPU/memory/port predecessor ownership from a firmware
compatibility/input boundary.  It may then repair one complete existing owner
batch only.  It may not add a Model-40 boot path, inject an FDC command, make a
display terminal mandatory, or treat an elapsed run as success.

## Construction baseline correction

The worktree contains an uncommitted batch headed `M5 T496 S7 Reset-ROM Owner
Batch`.  Its reset-ROM route has a distinct, documented owner and focused
smoke, but the worktree batch also includes XT topology, time-axis, diagnostic
output, generated-result, and broad fixture edits.  The baseline commit
cannot fresh-configure because it retains two stray source-file tokens at the
end of `CMakeLists.txt`; the current uncommitted CMake edit removes those
tokens and adds the candidate reset smoke target.

Therefore this batch is **preserved, not accepted or discarded**.  Before any
Model-40 repair is built from it, its reset-ROM minimum closure, the necessary
fresh-configure repair, and every unrelated edit must be separated into the
proper corrective receiver.  T498 S3 neither adopts nor validates those
uncommitted changes.

The candidate reset-only mechanism was separately rebuilt and run in the
current optimized Release tree.  Its focused smoke reports
`M5:T496:S7:RESET-ROM-ALIAS:OK`; the existing firmware-capability smoke also
reports its firmware, ROM-alias-lifecycle, and failure-propagation markers.
This is evidence that the identified minimum mechanism is live, not
acceptance of the mixed worktree batch.

## Residual input boundary

The existing supplied ROM pair is sufficient to reproduce the first unresolved
boundary.  If a subsequent observer cannot distinguish a Core transition from
firmware-specific execution without retaining vendor-derived data, this ledger
requires an explicit `INPUT-GAP` result rather than a guessed timing or
profile-side workaround.

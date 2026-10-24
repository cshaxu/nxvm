# MyPC110 Video And Board Initialization Source Gate

## Admission

T578 is the bounded successor to closed T577.  It consumes the real TFT
firmware path after the corrected 8042 reset/A20 and finite open-bus probe
route.  It is not admission of the broad PC110 product qualification candidate.

## Objective

Use the existing MyPC110 real-firmware diagnostic to identify the earliest
single video or board initialization contract after the corrected 8042 reset.
The diagnostic must record ordered register, port, memory and retirement facts
on both widths; it must not presume the old `F000:4CA9` INT 10h checkpoint is
reachable.

## Boundaries

- Start with diagnostic/evidence work in `test/app-mypc110` and NXVM records.
- A source-proven PC110 App wiring/configuration correction may be made in
  `src/app-mypc110`; an existing generic component may be corrected only when
  the trace proves its generic contract is incomplete.
- No synthetic IVT/BDA/BIOS data, guest-address dispatch, borrowed PC/AT
  video ROM, host display fallback, second executor, or raw asset commit.
- External PC110 repositories remain L2 research only.  Their source is not
  copied or transliterated into production.

## S Plan

1. Freeze the bounded dual-width trace after the 8042 reset and classify the
   first live firmware wait/initialization route before selecting any video
   or INT 10h candidate.
2. If—and only if—the trace yields a single App or generic owner, implement
   that narrow contract with a direct regression and repeat the real-firmware
   checkpoint on both widths.
3. Sweep the corrected defect class, run required repository-only unit gates,
   and record whether the valid INT 10h vector advances.  Product, POST,
   display, Setup, DOS and artifact qualification remain separate.

## Verification And Stop Conditions

The S1 diagnostic must prove its observation is real guest execution, retain
bounded output, and preserve the normal Core interrupt contract.  Stop and
record rather than fabricate progression when the trace remains ambiguous,
needs unproved device semantics, requires a broad compatibility model, or
would cross into synthetic firmware behavior.  A valid vector alone is not a
display or boot claim.

## S1 Delivery

The source gate found that the reset path does not yet reach C000/video-ROM or
the earlier INT 10h checkpoint.  It instead stops in the firmware's port-61
edge-wait loop.  The existing Board offers an elapsed-tick compatibility
source, but the available primary/L2 research establishes neither that source
nor a PC110 period.  The facts and explicit transfer are recorded in
[`T578 S1 evidence`](../etc/evidence/t578-s1-pc110-video-board-source-gate.md).
No production owner was changed.

# MyPC110 protected-reset source gate

## Admission

T577 follows T576's corrected firmware map.  The corrected dual-width trace
establishes that the later `INT 10h` checkpoint is reached only after an
earlier CPU shutdown/reset.  It therefore cannot establish the original
firmware's video-initialization order.

## Objective

Use the existing App-owned diagnostic to retain the final ordered retirement
window before the first ordinary CPU shutdown/reset after the admitted
`F000:605F` data reload.  Classify one actual owner: an existing generic CPU
contract, an App-owned board input, or a source-gated unknown.

## Boundaries

- S1 begins as diagnostic/evidence work under `test/app-mypc110` and NXVM
  records.  A traced App-owned board input or bounded App decode window may be
  repaired in `src/app-mypc110`; Core remains excluded unless the trace proves
  a complete generic contract.
- No CPU relaxation, selector/address special case, BIOS/IVT/BDA injection,
  borrowed ROM, PC/AT fallback, second executor or raw asset commit.
- External PC110 projects remain L2 research, never imported implementation.

## Verification

On both widths, the diagnostic retains a bounded, ordered final window before
the first reset and records the reset cause.  Existing MyPC110 unit tests,
the full repository-only dual-width unit suites and documentation gates remain
required before S closure.  A later repair is admitted only if this trace
isolates a complete generic or App-local contract.

## Stop condition

If the trace does not establish a single owner, record the finite evidence and
transfer it to the qualification candidate rather than fabricate progression.

## Delivery

The final pre-reset window identifies a normal guest `MOV AL,FE` / `OUT 64h,AL`
sequence, not a CPU shutdown after the data reload.  The existing 8042 is its
only owner.  The trace also exposed two bounded MyPC110 board omissions: reset
A20 state and the BIOS's first-64-KiB-above-RAM probe.  The App configuration
now selects the existing full 8042 with output port `03h` and models only that
probe span as L2 open bus.  Exact two-width facts and limits are recorded in
[`T577 S1 evidence`](../etc/evidence/t577-s1-pc110-protected-reset-source-gate.md).

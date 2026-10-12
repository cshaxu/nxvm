# M5 T555 — Selected Controller And Board Qualification

## Admission

T555 is admitted from the first M5 Queue candidate on 2026-10-10. It qualifies
retained controller and board behavior only after a current-path inventory; it
does not assume the pre-migration proposal's old component names are current.

## S1 — Complete: Baseline audit

Read the current Core and fixed-App composition graph, all relevant unit and
integration registrations, existing evidence/manual ledgers and the selected
controller proposal. Produce one implementation/evidence/receiver matrix for
PIC, PIT, DMA, RTC/CMOS, KBC/keyboard, FDC, HDC, video and board glue. Every
gap must state its owner, fact class, evidence tier, current receiver and a
bounded repair/proof recommendation. The resulting [baseline ledger](../etc/evidence/t555-s1-controller-board-baseline.md)
uses the current Core/App paths and is the only admission input for follow-up
controller work. This S did not modify source, tests, assets or external
materials.

## S2 — Complete: PIC/PIT/DMA and refresh/IRQ

The [S2 reconciliation record](../etc/evidence/t555-s2-controller-route-reconciliation.md)
disposes the complete selected register/mode/reset/request/consumer batch
against original Intel and IBM sources. Chip mechanisms remain chip-owned;
board wiring and single-bus scheduler service remain distinct owners. One
approved appended refresh-duration contract value is the only public change.
No CPU instruction, Shared-six or MyNES source/test/artifact changed.

Final complete repository-only units pass 546/546 per width; the four selected
integration groups pass 32/32 per width with unchanged checkpoints/budgets.
DeskPro x86 boot passes in 179.93 seconds; earlier failures remain evidence,
and that narrow-margin pass is not a statistical host-throughput guarantee.
Eight 0555 optimized stripped PC artifacts replace the prior 0551 pairs;
four adjacent owner INIs remain hash-identical. Manual desktop acceptance is
not claimed. Source is +244/-111 (net +133), tests/registration +814/-44
(net +770), excluding documentation, manifests and generated artifacts.
Coordinator actual-pushed-diff review accepts implementation `69dca87fb`.
It reconciles the S1/S2 finite batch, actual source/test/build/artifact changes,
primary-source classifications and final receiving results; no unresolved
confirmed S2 repair is hidden by a later boot checkpoint. Required packet,
ownership, naming, source-policy and documentation checks are satisfied.
Existing electrical/timing exclusions remain explicit, not promoted by boot.
T555 remains open for S3-S6; the eight admitted PC artifacts are 0.5.0555.

## S3 — Complete: RTC/CMOS and keyboard/KBC/XT-PPI

S3 reconciles the complete 38-row RTC/CMOS,
KBC/enhanced-keyboard and XT-PPI/keyboard batch in its
[source ledger](../etc/evidence/t555-s3-rtc-keyboard-source-ledger.md).
Current-production units pass 546/546 per width; current-fixture four-App
integration passes 32/32 per width. All eight 0555 optimized stripped
artifacts are rebuilt and PE/hash verified. Source is +683/-219 (net +464),
tests/registration +1951/-93 (net +1858), excluding manifests/documents/EXEs.
Compaq speed/read/HOLD remains required S6 repair work; the unexplained
earlier DOS keyboard result and narrow-margin DeskPro x86 boot remain
explicit limitations. Shared-six, MyNES and owner INIs are unchanged.
Coordinator actual-pushed-diff review accepts implementation `2ad955473`.
The review maps every changed source/test/build/artifact and the finite row
dispositions to the original S3 brief, including failure atomicity, stream
ownership, selected model grades and the test observer correction. No
confirmed transferred gap is hidden by a successful boot. S3 closes; T555
remains open for S4--S6.

## Remaining Ordered S Work

Bounded S5 implementation `3cfadc6d4` is pushed and accepted after actual-diff
review. Complete units pass 546/546 per width and four-App integrations 32/32
per width; DeskPro's GC scanout-selection failure is repaired in S5 and passes
within its original 180-second budget. Eight optimized stripped 0555 PC EXEs
are deployed and checked; Shared-six, MyNES and owner files are unchanged.
Source C/H +632/-276, tests +2470/-286, project firmware +129/-4. Remaining
video classes are explicitly transferred, not qualified. S6 is now admitted
for the already-approved Shared copied-glyph capacity and ten-App-artifact
qualification; owner acceptance is required after its delivery.

| S | Boundary | Exit condition |
| --- | --- | --- |
| S4 | FDC/media and HDC personalities | Each selected controller/media route has an explicit grammar, request/IRQ/DMA consumer and reset/deadline disposition. |
| S5 | Bounded video delivery, owner split 2026-10-12 | Existing register/raster/palette/capture/publication and firmware repair batch passes full units, selected integrations and eight artifacts; unresolved video classes transfer intact, not qualified. |
| S6 | Approved Shared 32-scanline glyph slots | Default remains 16; fixed slot layout, validation/render/copy and Core adapter are consistent; public-six quality gates, full dual-width tests and all ten App artifacts pass; owner acceptance follows delivery. |
| S7 | Remaining video qualification | V08/V09/V17--V20/V28 residual hardware producers and source/receiving classes are exhausted with direct and actual wiring proof; copied-capacity proof alone does not qualify live hardware. |
| S8 | Cross-profile closure, formerly planned S6 | Four fixed-App route matrix, Compaq speed/AUX-PIT/HOLD class, exact remaining L1/L2/L4 exclusions, relevant unit/integration proof and documentation closure are complete. |

## S6 Receiving Delivery

Shared P1 `a7e69a14b` is pushed: fixed copied glyph slots are 32 rows,
Base uniquely owns the default height 16, Console line storage is CAPACITY
and Debug's unchanged argument storage is CAPACITY. Core retains normalized
16-row construction input and copies/pads into the enlarged output slots.
Production C/H is +38/-25 (net +13); receiving tests +105/-24 (net +81).
No new Shared function/type/state, helper, thread, queue or renderer is added.

Final full units pass 591/591 per width; four-PC integrations pass 32/32
per width and MyNES integrations 12/12 per width. The final x86 Model 40
180-second failure remains recorded. The owner explicitly approved a
300-second startup budget, with bounded outer cleanup allowances; the final
DeskPro receiver passes in 172.91 seconds x64 and 206.06 seconds x86.
No guest timing or production input is changed for that budget adjustment.
All ten optimized stripped receiving EXEs are rebuilt, deployed and PE-width
checked; owner INIs, media and snapshots remain unchanged. See
[S6 delivery evidence](../etc/evidence/t555-s6-copied-glyph-capacity.md) and
[six-tree naming audit](../etc/evidence/t555-s6-shared-constant-naming-audit.md).
Automated native desktop checks pass; no manual desktop or complete live
font/electrical qualification is claimed. The owner's next-S admission accepts
the delivered S6 batch after its actual-pushed-diff audit. S6 closes; T555
stays open and S7 is admitted.

## S7 Admission

S7 consumes the complete transferred V08/V09/V17/V18/V19/V20/V28 batch,
not the first subsequent guest failure. It starts with original-source and
current-path reconciliation of RAM gating, clock/reset consumers, live
plane-2 fonts, raster/status/IRQ, panning/split/stride, CECG Restore/blanking
and retained VGA regression limits. Report exact file surface, necessary
interfaces/state and expected diff before production edits. Core owns the
repair; Shared six, MyNES and sibling repositories stay read-only.
Final dual-width repository units, four-PC integrations, affected 0555 pairs
and actual-diff acceptance remain required. DeskPro retains the approved
300-second startup wait. T555 stays open for S8; admission alone changes no
executable input and does not justify new binaries or runtime qualification.

## Original Audit Frozen Boundaries

Vendor manuals and ROMs remain in `nxvm-assets`; S1 may only read their
repository evidence/provenance. Shared Lib/Emulator/x86 code is read-only. No
asset, firmware, media, INI, test or production change is permitted until the
owner reviews the audit and admits a later S.

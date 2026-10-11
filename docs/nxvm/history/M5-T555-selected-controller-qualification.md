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

## Remaining Ordered S Work

S3 executor delivery reconciles the complete 38-row RTC/CMOS,
KBC/enhanced-keyboard and XT-PPI/keyboard batch in its
[source ledger](../etc/evidence/t555-s3-rtc-keyboard-source-ledger.md).
Current-production units pass 546/546 per width; current-fixture four-App
integration passes 32/32 per width. All eight 0555 optimized stripped
artifacts are rebuilt and PE/hash verified. Source is +683/-219 (net +464),
tests/registration +1951/-93 (net +1858), excluding manifests/documents/EXEs.
Compaq speed/read/HOLD remains required S6 repair work; the unexplained
earlier DOS keyboard result and narrow-margin DeskPro x86 boot remain
explicit limitations. Shared-six, MyNES and owner INIs are unchanged.
Coordinator actual-pushed-diff acceptance and S3 closure remain pending.

| S | Boundary | Exit condition |
| --- | --- | --- |
| S3 | RTC/CMOS and keyboard/KBC/XT-PPI | Current reset, port, NMI, clock and firmware-facing routes are classified; no controller-local timing or board duplicate is introduced. |
| S4 | FDC/media and HDC personalities | Each selected controller/media route has an explicit grammar, request/IRQ/DMA consumer and reset/deadline disposition. |
| S5 | Video and selected board display routes | CGA/EGA/CECG owner, aperture, clock and copied-frame routes are qualified without adding a renderer or second framebuffer. |
| S6 | Cross-profile closure | Four fixed-App route matrix, exact remaining L1/L2/L4 exclusions, relevant unit/integration proof and documentation closure are complete. |

## Frozen Boundaries

Vendor manuals and ROMs remain in `nxvm-assets`; S1 may only read their
repository evidence/provenance. Shared Lib/Emulator/x86 code is read-only. No
asset, firmware, media, INI, test or production change is permitted until the
owner reviews the audit and admits a later S.

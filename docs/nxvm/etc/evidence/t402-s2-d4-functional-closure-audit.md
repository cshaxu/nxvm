# T402 S2: Independent D4 Functional Closure Audit

`M5:T402:S2:CLOSURE-AUDIT:OK`

## Decision

T402 closes as a bounded DeskPro 386 D4 **functional** reconciliation.  It does
not close physical D4 memory timing, cache timing, ISA timing, firmware
behavior, DeskPro hardware fidelity or L3 readiness.

## Independent reconciliation

| Completion requirement | Evidence and decision |
| --- | --- |
| One complete selected D4 functional matrix | S1 names replacement, compatibility/high alias, control, diagnostic high, setup word, nonresponsive page, parity/IOCHK and all residual physical members.  Each has one disposition. Accepted. |
| Original evidence precedes reference behavior | D3PE addresses the D4 aperture, mapping, control and parity topology.  S1 labels only register subforms and same-page nonresponse as local 86Box/PCjs Tier 2. Accepted. |
| Every confirmed discrepancy is repaired with focused proof | P1 `1c41bc53` changes only Model-40 D4 registration/state/callbacks and its existing map smoke. The prior one-byte aperture is replaced by four visible bytes plus a 4 KiB nonresponsive page. Focused map and parity smokes pass. Accepted. |
| Core/VM ownership is preserved | Actual P1 source sweep finds D4 address/state only in `vm/profile/model40`; Core is unchanged and continues generic registered-memory dispatch. Accepted. |
| Reset, write, alias and parity variants are retained | Map smoke covers reset, replacement, write protection, all four selected bytes, setup-low write/reset and ignored byte. Parity smoke covers lane status, control clear and IOCHK. Existing T386 records cover compatibility/high alias. Accepted. |
| Full regression and documentation result | P1's focused tests pass; its full current CTest gate passes 285/285; documentation governance passes. S2 replays source/evidence/status sweeps and governance with no divergence. Accepted. |
| Physical residuals are explicit and earliest-owned | D4 DRAM RAS/CAS/READY/DMA phases, memory/cache/ISA waits, cache effect and external measurement remain assigned to the later board/device timing receiver. No physical timing value, runtime hook or L3 predicate is claimed. Accepted transfer. |

## Actual-change review

The audit reviewed `1c41bc53` rather than relying on its report.  The read,
write and query callbacks consistently bound the exact registered 4 KiB window;
only offsets 0--3 expose state, while other positions read `FFh` and ignore
writes. Reset restores the selected private 1 MiB values. The test proves each
new behavior both before and after reset. No unrelated source, asset, ROM,
media, vendor path or third-party code entered the task.

## Closure statement

All members of T402's admitted functional D4 universe are conforming, repaired,
explicitly Tier-2-derived or transferred.  The D4 functional baseline is thus
complete, and T402 may close. The next DeskPro work must select a distinct
board/device timing receiver and use its own original/reference/generic
provenance ledger; no outcome here authorizes a physical or L3 assertion.

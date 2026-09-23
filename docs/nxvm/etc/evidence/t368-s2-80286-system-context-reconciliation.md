# T368 S2: 80286 System-Context Reconciliation

## Authority and method

Intel's *80286 and 80287 Programmer's Reference Manual*, order
210498-005 (1987), Appendix B is the authority for the 80286 successful
instruction rows.  The read-only manual archive is
[available at Bitsavers](https://bitsavers.org/components/intel/80286/210498-005_80286_and_80287_Programmers_Reference_Manual_1987.pdf);
no manual content is imported.  Existing T366 evidence already records the
applicable Appendix-B rows: VERR/VERW register `14`, memory `16`; LAR/LSL
register `14`, memory `16`; SMSW register `2`, memory `3`; and LMSW register
`3`, memory `6` in real address or protected CPL0 context.

S2 executed the focused 80286 timing, prefix and LOCK smokes and examined the
semantic route for every residual 0F family.  It distinguishes successful
selector query and MSW work from rejected/faulting protected forms; a decoder
form alone is not evidence of retirement.

## Reconciled outcomes

| Form/context | Finding | S2 disposition |
| --- | --- | --- |
| `0F 00 /4,/5` protected VERR/VERW, register/memory | The T366 branch admitted the forms but accidentally shared SLDT/STR's `2/3` scalar.  The focused run observed `2` for a VERR register form although the accepted row is `14`. | Corrected the existing sole 80286 classifier to publish `14/16`; no new publisher or source value was created. |
| `0F 02` protected LAR | The classifier already published `14/16`.  The ledger fixture expected pre-access descriptor rights `0x9200`; the semantic path correctly observes `0x9300` after the descriptor access bit is set. | Corrected only the stale fixture observation; valid and invalid selector timing contexts remain as T366 S27. |
| `0F 03` protected LSL | Existing `14/16` classifier route is reached after the corrected VERR branch. | Retained; no new allocation. |
| `0F 01 /4` real/protected SMSW | The real-mode fixture set CR0.PE directly without establishing protected segment state, then called it a real-mode row. | Repaired the real-mode fixture to use non-PE MSW bits `0x000c`; it now verifies the existing `2/3` rows without manufacturing invalid execution state. |
| `0F 01 /6` LMSW protected CPL0 | The fixture reaches a real `#GP(0)` at its supposedly CPL0 protected register case, before retirement.  The instruction handler's `_s_load_cr0_msw` rejects according to its observed `_GetCPL` state. | **Not timed in S2.** S3 must reconcile the protected boot fixture, decoded CPL observation, and existing 80286 LMSW semantic owner before retaining or changing the T366 S31 protected claim. |
| `66`/`67`, rejected LOCK/privilege, reserved 0F, selector/table/memory fault forms | Existing prefix and semantic smokes observe `#UD`/`#GP` or other non-retirement outcomes. | No successful tick is allocated. |
| Legal 80286 LOCK-prefixed ordinary forms | T328 proves legal real-mode and protected `CPL <= IOPL` execution, while S1 shows many are not source-timed by primary/control classifiers. | Remains a separate successful-prefix timing receiver after exact Appendix-B context/reference-model review; no one-tick value is accepted. |

## Verification record

- Rebuilt `core-machine-80286-instruction-timing-ledger-smoke` from source.
  Before correction it stopped at VERR/VERW (exit `13`, observed two ticks);
  after the VERR and fixture corrections it proceeds through VERR, LAR, LSL
  and SMSW and stops at the unproven protected LMSW row (exit `18`, `#GP(0)`).
- `core-machine-instruction-timing-smoke`,
  `core-machine-legacy-lock-s1-smoke`, and
  `core-machine-prefix-attributes-s64-smoke` pass.  Their expected diagnostic
  `#UD`/`#GP` output is the tested non-retirement boundary.
- The full current gate is still not claimed: the independent
  `vm_request_bridge_smoke.c` stale request-interface compile failure recorded
  by T367 remains outside this S, and the focused 80286 ledger is intentionally
  red until S3 resolves protected LMSW.

## Transfers and non-claims

S2 repairs one already-accepted VERR/VERW routing error and two stale fixture
assumptions only.  It does not close protected LMSW, legal-prefix timing,
remaining system/default fallback, CPU retirement timing as a whole, bus,
device service, physical time or 5170 L3.  S3 receives protected LMSW semantic
and retirement reconciliation; a later T368 subtask receives remaining legal
prefix and final-default successful routes.

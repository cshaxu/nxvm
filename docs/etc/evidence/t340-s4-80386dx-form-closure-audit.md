# T340 S4: 80386DX Form Closure Audit

## Original-Request And Proposal Review

The owner requested Queue-ordered, holistic implementation through the final
four-profile audit. The T340 proposal assigns only width, prefix, FS/GS, and
non-privileged integer-form mechanisms. S1 first froze that boundary; S2 and
S3 then reviewed the actual shared production owners and their focused proof.
This audit adds no runtime claim beyond those declared matrices.

## Final T340 Ledger

| Assigned boundary | Evidence and actual owner | Final disposition |
| --- | --- | --- |
| Prefix state, `66`/`67`, address/operand materialization, FS/GS form selection, and `LOCK` classification | T340 S2; T316 S64/S23/S24/S31, T328, and the owner-smoke registration audit | Complete for T340's declared form mechanisms. Single prefix loop, width expressions, effective-address route, FS/GS loader route, and `LOCK` classifier were confirmed. |
| Ordinary primary form width variants | T316 S23--S56 and T322 S1, cross-checked by S2 | Accepted prior form proof; T340 does not duplicate completed ordinary matrices. |
| Non-privileged `0F` integer/control: near Jcc, SETcc, bit operations, double shifts, IMUL, bit scan, MOVZX/MOVSX | T340 S3; T303 and T310 S3--S8 owner smokes; secondary metadata/dispatch audit | Complete for T340's declared non-privileged form matrix. Each valid assigned secondary family has one dispatch, FLAGS/publication owner, and focused proof. |
| 32-bit gates/frames, selector/table cache state, `0F 00/01`, CR/DR/TR, VM86, paging, task, breakpoint/debug | T339 transfer audit, T320/T321/T325/T329/T337 histories, S1 ledger | Transfer once to T341. These require privileged state, frame/layout, translation, or delivery composition and are not form-owner work. |
| 8086/80186/80286 baseline or protected-only semantics | T328/T338/T339 retained evidence | Accepted predecessor boundary; T340 neither repairs nor reclassifies it. |
| VME/PVI, later CPUs, persistent TLB/test-register models, x87 numerical execution, timing/devices/Windows | TODO external boundaries and 80386 closure map | Explicit external or post-80386 boundary. No T340 completion claim depends on these rows. |

## Closure Sweep And Verification

The S1--S3 source/registration queries were rerun against prefix, width,
FS/GS, `LOCK`, secondary dispatch, and all 14 named current owner smokes.
Every named smoke is registered and passed. The source review found no second
form mechanism, duplicate writer, unregistered replacement smoke, or
ambiguous receiver. The full current gate, documentation governance, and diff
check are required and recorded by the delivery.

## Conclusion

No T340 row is partial, missing, or unclassified. T340 may close after
acceptance; its retained proposal moves to task history, the Queue advances to
the T341 system-state candidate, and a later audit must not reopen this form
ledger without a reproduced earliest-owner gap.

# M5 T435: Core CPU Instruction Timing Program Requeue

## Task Record

T435 began the four-profile Core instruction-timing program. It established
the Core-private timing foundation and accepted earlier profile/audit work, but
its 80286 and 80386DX implementation closures remain incomplete. The owner
directed on 2026-08-22 that the two profiles become separate front-of-queue
implementation candidates and that this task be brought to a truthful close.

## Accepted Progress

The retained program history includes the accepted 80286 and 80386DX source,
manifest and decoder-ledger baselines. The latest 80286 implementation work
also extends the protected outer-IRET timing path. Its foundation runner
observed 606 records, but that was not a conforming closure report: the
80286 manifest verifier still reported 286 base forms, 459 generated contexts,
62 combinations, `conforming=0`, `missing_input=47`, and `missing_test=239`.

The retained 80386DX decoder-ledger audit reconciles the current decoder
universe with its manifest; the implementation audit remains deliberately
nonconforming: 450 L3 base forms, 961 legal contexts and 1,411 canonical
dispositions. It records 257 keys lacking focused results, 192 keys lacking
one or more normalized inputs, and an unallocated XLAT form. Neither profile
has a final result-contract artifact or an all-keys-conforming marker.

## Owner-Approved Stop And Requeue

The remaining profile work is explicitly transferred to the separate
[80286 instruction timing closure](../proposals/m5-80286-instruction-timing-closure.md)
and [80386DX instruction timing closure](../proposals/m5-80386dx-instruction-timing-closure.md)
candidates. They are the first and second entries in the M5 Queue and preserve
the required 80286-before-80386DX dependency. The former four-profile proposal
is retained as non-candidate planning history.

## Task Outcome

**Stopped by owner-approved requeue; not completed.** T435 has no final
four-profile closure claim, no 80286 or 80386DX all-keys-conforming claim, and
no transferred board/transaction timing ownership. Each new candidate must be
admitted, receive its own numeric implementation identifier, and independently
meet its complete closure standard before it can report success.

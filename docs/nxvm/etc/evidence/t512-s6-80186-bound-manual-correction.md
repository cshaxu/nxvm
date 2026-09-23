# T512 S6 P4: 80186 BOUND Manual Correction

## Evidence

- Normative source: Intel *iAPX 86, 88, 186 and 188 User's Manual* (1985),
  order no. 210912-001, Table 2-9, PDF page 183 / printed page 2-17.
- Asset: owner-managed `../../../../nxvm-assets/manuals/cpu/intel-8086-8088-80186-80188-users-manual-1985.pdf`.
- SHA-256: `2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF`.

The table gives `BOUND` a **33--35 clock range**. The prior P1 conclusion that
it was an exact 35-clock Manual-L3 row was a transcription error. The range is
retained as an explicit L2 midpoint model, selecting 34 at the sole existing
80186 selector; it is neither an invented exact Manual-L3 scalar nor L1.

## Scope and result

- Code delta: restores the existing sole-owner scalar from `35u` to the
  disclosed `34u` midpoint; no owner, API, profile path, compatibility path,
  or duplicate timing route was added.
- Manifest/ledger delta: `I186-BOUND` is named L2 midpoint 34 within the
  33--35 Intel range; the base denominator is 253 Manual-L3 and 26 named L2
  keys.
- Direct rebuilt runner: 279 base, 54 REP phase, 19 LOCK, 19 LOCK+segment,
  87 odd-word, 88 segment, 63 REP-combination and 89 combination observations;
  verifier reports 616 conforming keys.
- Complete repository-only unit suite: 315/315 passed.
- Documentation governance: passed.

No L1 row was discovered by this correction.  S6 remains active for the
complete 80186 owner/manual sweep; this evidence records only the completed
P1 repair.

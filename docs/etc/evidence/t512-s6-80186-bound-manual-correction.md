# T512 S6 P1: 80186 BOUND Manual Correction

## Evidence

- Normative source: Intel *iAPX 86, 88, 186 and 188 User's Manual* (1985),
  order no. 210912-001, Table 2-9, PDF page 180 / printed page 2-14.
- Asset: owner-managed `assets/manuals/intel/80186/210912-001_iAPX_86_88_186_188_Users_Manual_1985.pdf`.
- SHA-256: `2516D66CC75076D9AC9EE048E8420C09C35655FB25ED34DDA6351A3EA4E0AFFF`.

The table gives `BOUND` a scalar **35** clocks.  The prior ledger and Core
selector incorrectly recorded a `33--35` range and chose 34.  This is a
source-backed correction, not an estimate: the row is now Manual-L3 at the
sole 80186 timing selector and its existing manifest runner expects 35.

## Scope and result

- Code delta: one scalar changed, `34u` to `35u`; no owner, API, profile path,
  compatibility path, or duplicate timing route was added.
- Manifest/ledger delta: `I186-BOUND` changes from named L2 midpoint to
  Manual-L3 exact; the base denominator is now 254 Manual-L3 and 25 named L2
  keys.
- Direct rebuilt runner: 279 base, 54 REP phase, 19 LOCK, 19 LOCK+segment,
  87 odd-word, 88 segment, 63 REP-combination and 89 combination observations;
  verifier reports 616 conforming keys.
- Complete repository-only unit suite: 315/315 passed.
- Documentation governance: passed.

No L1 row was discovered by this correction.  S6 remains active for the
complete 80186 owner/manual sweep; this evidence records only the completed
P1 repair.

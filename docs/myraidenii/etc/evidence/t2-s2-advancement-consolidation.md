# T2 S2: Remove Identical CPU Advancement Forks

## Scope, Design And Actual Diff

Baseline `50eaf75b7`. S1 found 83 `>=80386` forks whose true and false
arms each contain only `_adv;`. CPU profile is serialized owned state, not
a volatile observation or side-effecting expression. Replace each fork with
the same single advancement; retain every handler, callback, table and error
macro. No native/NEC/8080 CPU is added or aliased by this batch.

Counted code paths are `src/x86-core/cpu/cpu_instructions.c`,
`src/x86-core/verify_corpus.cmake`, and `test/x86-core/verify_negative.cmake`.
Using Git numstat against the baseline, excluding manifests, docs and EXEs:

| Owner/file | Added | Deleted | Net |
| --- | ---: | ---: | ---: |
| CPU instruction unit | 86 | 667 | -581 |
| X86 Core owned source gate | 4 | 0 | +4 |
| Existing owned negative test | 2 | 0 | +2 |
| Total source/test | 92 | 667 | -575 |

Git aligns some adjacent identical lines differently; the exact transformation
still consists of 83 eight-line forks becoming one line each, net -581.
The initial CPU estimate was net -581; static/test additions were estimated
12--20 and actually require six. Two manifests only update three file hashes.
No helper, public interface, persistent state, thread, interpreter or fallback
is introduced. The new static check belongs inside the existing CPU owner
gate, with a case in the existing negative fixture; no extra test framework.

## Similar-Issue Sweep And Review

The full original S1 ordinal query now returns 223 candidate lines: 214 in the
instruction unit, one instruction header, six timing model and two Machine.
All 83 exact duplicate advancement forks are gone. Unequal branches remain:
their operands, widths, descriptor/bus effects or exception semantics require
the family/form/entry/timing qualification already named in the V30 ledger.
They are not silently rewritten or treated as resolved by this deletion.

Review compares the complete file to the exact normalization of the Git
baseline, not merely selected samples. Equality passes; no other instruction
change exists. Static `_adv;` sites reduce from 547 to 464 because one unused
copy per pair is removed; actual admitted execution still advances once.
The source gate rejects a reintroduced redundant profile fork in an owned
CPU fixture. Real current source passes; original negative boundary cases
continue to pass. Instruction tables and existing formatting are untouched.

## Verification And Publication

- Both existing Release Ninja/ccache graphs build successfully. No new graph,
  compiler, system setting or cache directory was introduced.
- x64 complete repo-only units: 655/655 pass, 282.74 seconds.
- x86 initial suite: 654 pass, unchanged X86 NXVM verifier-negative times out
  at its existing budget. Isolated unchanged retry passes in 18.05 seconds.
  Final complete x86 suite passes 655/655 in 78.42 seconds at `-j4`.
  No timeout/expectation/source change is used to obtain the final pass.
- Both suites include all 166 X86 Core tests, 229 X86 NXVM tests, 45 MyNES
  units and every other registered repo-only unit. Their passing coverage is
  old-profile preservation, not new V30 qualification.
- Source/test manifests and inward dependency/static/negative gates pass.
- Existing four-PC version 0563 is retained. Each host graph is reused for
  XT, AT, DeskPro and default product links, then restored to default selection.
  No Shared CPU recompilation is needed between those product links.
- All eight deployed EXEs pass PE width, product-name/version and adjacent
  INI-name checks. [Artifact hashes](t2-s2-pc-artifacts.json) identify them.
- All four user INIs, external firmware/media and MyNES pair are unchanged.
  Lib, Emulator, X86 Debug, X86 IBMPC and X86 NXVM source are unchanged.

The recorded builds/tests are background process results, not manual desktop
or RDP checks. No new full external-ROM integration, physical audio, guest GUI
or RAIDEN II gameplay run occurred. None is reported as passed. The bounded
source change preserves the old sole path by exact equivalence; final V30 T2
and M1 still require their original runtime/integration qualifications.

## Complete-P And Acceptance Boundary

Shared delivery: `415199682`; NXVM artifact delivery: `b08aa7aad`, both pushed.
The compiled CPU source SHA-256 is
`C9E84D2CA4DBF9AB670369B099213A4E3E3D71E571C8E2BCACB9FE348B182F9B`.
Artifact records below this task retain that source delivery and the unchanged
0563 receiving version, rather than adopting MyRaidenII's independent T2 as a
PC release identity.

One P per actual target: Shared source/tests/manifests, NXVM changed artifacts,
then MyRaidenII packet/history/evidence. These are the assigned cross-target
deliveries of one S, not unrelated App edits. Coordinator reviews all actual
diffs and proofs before the separate governance P closes S2. Full V30 work,
including the remaining 223 candidate judgments, remains unimplemented.

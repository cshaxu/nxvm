# T539 S21: LEA/MOVX Migration

## Intake (not acceptance)

Baseline `7e7a7e156`; worktree clean before admission. Current owns status.
The proposed 18-file S21 contained 11,335 lines, not a bounded small batch.
The revised work plan assigns every original file to S21-S29 and shifts only
unadmitted subsequent packages to S30-S40. Historical accepted identifiers
are unchanged. Line counts are PowerShell `Get-Content` array lengths at the
baseline, not semantic case counts or claimed coverage.

Both selected files were read completely, including helpers and main dispatch.

## Original Case Map

| Original family | Frozen contexts and assertions | Receiving constraint |
| --- | --- | --- |
| LEA real forms | Four CPU profiles x four operand/address forms; IP, EAX, untouched EBX/ESI/FLAGS; invalid-prefix UD preserves state. | Chip forms plus explicit fault boundary; do not drop legacy profiles. |
| LEA register-direct | Four profiles reject memory-only encoding; IP/EAX/FLAGS preserved. | CPU legality and fault observation. |
| LEA LOCK | 386 rejects LOCK LEA with original state preserved. | CPU legality. |
| LEA protected | Four forms following real guest GDT/LMSW/far-jump setup. | Preserve original guest setup and results. |
| LEA null DS | Artificial invalid DS cache proves effective address needs no operand read; selector/validity unchanged. | CPU-owned private invariant, not a mutable board accessor. |
| LEA IRQ | Real PIC vector, IRQ0 and saved frame IP=3; no interrupt shadow. | Board composition stays NXVM-owned, copied CPU observations only. |
| MOVX forms | Four opcodes x two widths x register/memory; flags, ECX and IP. | Preserve all 16 table contexts. |
| MOVX address prefix | 67/66 MOVSX word through ESI; ESI and flags unchanged. | CPU form with original bytes. |
| MOVX legacy read boundary | Two profiles x four declared opcode iterations; zero provider reads, UD, preserved ECX/FLAGS/IP. | Keep actual original eight byte sequences; separately correct missing intended opcode coverage. |
| MOVX protected limit | Guest-created descriptor limit; terminal DF and unchanged ECX/FLAGS/IP. | Preserve fault escalation and board terminal expectation; chip GP alone is not equivalent. |

## Verified Construction Gap

In `movx_test_read_boundaries`, the template is `0F B6 0E 00 50`, but
`form_code[2] = opcode` writes the ModR/M byte rather than opcode byte 1.
The existing eight iterations therefore do not prove rejection of all four
declared MOVZX/MOVSX opcodes. This is source evidence, not a new CPU defect.
Retain those original byte cases and add the intended opcode variants during
migration; do not silently rewrite their claimed provenance or remove them.

## Boundary Findings

The public register patch loads architectural segment selectors; it cannot
represent arbitrary invalid hidden caches, and assigning EIP does not clear
HLT. Neither limitation authorizes a private-pointer bridge. The existing CPU
bus fixture is CPU-owned but has a 2 KiB modulo memory and fixed port contract;
it cannot be reused unchanged for these larger physical-address/board cases.
Design the minimal receiving fixture at its true owner before migrating them.

## Implementation Progress (not acceptance)

The 16 MOVX register/memory/width cases and the address-prefix case now live
in `cpu_movx_smoke.c`, linked only to the existing `x86-cpu` target. Their
opcode table, source values, result expectations, FLAGS and IP assertions are
retained. A CPU-owned fixture supplies a bounded 64 KiB byte array without
modulo aliasing, a PC machine, peer device, or external asset. No production
source or public contract changed. The fixture is staged alongside the current
CPU owner; S39 owns its eventual Shared move.

The original MOVX board test retains its memory-provider and protected-mode
fault cases. Its legacy-negative loop now covers both byte positions: eight
original ModR/M sequences plus eight intended opcode sequences. Both the
CPU-only target and board target compile and execute successfully on x64 and
x86. An initial CTest filter omitted the `unit.` prefix and selected zero
tests; it is not counted as verification. The corrected runs execute two tests
per width, all passing.

## Completed Migration And Executor Review

LEA's 26 chip cases now live in `cpu_lea_smoke.c`: 16 real forms, four
register-direct rejections, one LOCK rejection, four protected forms and one
invalid hidden-DS-cache case. The original GDT/bootstrap/HLT bytes remain in
the protected chip fixture. CPU-local negative cases retain the original
IDTR limit, terminal stop, copied fault and register rollback assertions.
The board file retains IRQ0, actual PIC ISR/IRR and the saved IP=3 assertion;
it uses public register patches, copied snapshots and physical-memory reads.

MOVX's remaining board cases also use public patches/snapshots. The negative
memory provider denies the vector-6 IVT read rather than editing private IDTR;
the CPU's existing failed-delivery path preserves the original UD and fault
registers. The source provider still must receive zero reads. This is an
explicit fixture-mechanism change, not a claim that IDTR-limit and bus refusal
are the same hardware event. CPU-local LEA retains IDTR-limit coverage.
The protected MOVX bootstrap stops after its ten setup instructions instead of
executing HLT and privately clearing it. The measured instruction still starts
at protected CS:0, with the same descriptor bytes, and must produce terminal
DF with unchanged ECX/FLAGS/IP. No CPU behavior was changed to pass these cases.

The original MOVX opcode loop allocated machines before skipping six unrelated
opcodes, leaking those test objects. Filtering now precedes construction.
The same-class check covered both moved tables and both board files; no other
skipped-allocation path was found there. No general test framework was added.

Both board files have zero `executor_cpu`, private CPU include or legacy CPU
fixture references. A static gate protects this result with eight additional
negative controls (85 total including previous CPU/board controls). T332/T344
inventories explicitly distinguish the two provider-free public-board setups;
they retain exact membership and lifecycle requirements. T337 follows the UD
assertions to the new CPU owner and names the IVT-refusal terminal fixture.

## Verification And Limits

- Both existing build trees compile; complete unit runs pass 373/373
  on x64 (30.86 seconds) and x86 (29.14 seconds). The two added targets contain
  moved original cases, not replacement coverage for deleted cases.
- All 66 specialized steps pass. Initial failures correctly exposed stale
  lifecycle/constructor inventories; these were reconciled rather than bypassed.
  The T345 intentional-negative self-test diagnostic remains expected.
- All six Shared manifests pass unchanged; documentation and diff checks pass.
- Counted with `git diff --cached --numstat` over the ten changed test/build/gate
  paths: +626/-499, net +127. Growth is the independently linkable CPU fixture,
  explicit board/CPU test separation and boundary prevention, not production code.
- No production or executable-link input changed. Existing eight 0539 EXEs,
  Shared corpus, MyNES and owner INIs remain unchanged; no EXE rebuild or external
  integration is claimed or required for this test-only S.
- Three existing build trees and the S18 recovery patch remain needed for later
  CPU packages. No new build tree or external input was created.

Executor review maps all 53 original cases to the four receiving tests, plus
eight intended MOVX opcode cases. The complete two-file batch is implemented;
coordinator actual-commit acceptance remains separate. Other private consumers,
opaque allocation and Shared relocation remain S22-S40, not accepted here.

Final source review restored the original explicit no-first-fault assertion in
the CPU-only MOVX success helper. The subsequent x64 full run passes 373/373
(27.98 seconds). A concurrently launched x86 full run passes 372/373 but fails
unchanged `library.kvm_window_modal` at line 74: its native move loop had already
exited. Both trees exercise the same host desktop; interference is a plausible
explanation, not a proven root cause. That run is not counted as a pass. Repeat
verification runs the x86 full suite alone and serially, without changing Shared
code, expected assertions or exclusions; its result is recorded below.

The isolated serial x86 complete run passes 373/373 (86.07 seconds), including
the unchanged modal-window assertions. No persistent test failure remains in
the admitted batch. The cross-tree desktop risk is retained in NXVM TODO for
an explicitly admitted Shared review; a passing isolated run does not prove
that concurrency defect fixed. Final S21 verification is x64 373/373 and x86
373/373, with six unchanged manifests and the documentation/diff gates passing.

## Coordinator Actual-Commit Acceptance

Reviewed pushed NXVM P1 `1049b9021` after switching from executor to coordinator.
The actual test/build diff preserves the original instruction tables, expected
registers and faults, all 53 original contexts and the eight newly corrected
opcode contexts. Public board operations replace private CPU access; artificial
hidden-cache cases stay CPU-owned. IVT refusal and bounded bootstrap setup are
explicit fixture changes, not unchanged hardware claims. No CPU algorithm,
timing grade or production ABI changed. Gates retain exact constructor ownership
and add negative boundary controls rather than exempting the moved tests.

The 18 changed paths belong exclusively to NXVM. Source/test accounting remains
+626/-499 across ten paths; positive growth provides independently linkable
CPU tests and boundary prevention. The source tree, Shared manifests and deployed
artifacts are unchanged. Documentation maps every original consumer to the
revised receiving plan; historical identifiers remain immutable. Current's
packet is removed on acceptance, and S22 is next. T539 and the CPU ledger row
remain open. Desktop-test isolation is explicitly retained in TODO.

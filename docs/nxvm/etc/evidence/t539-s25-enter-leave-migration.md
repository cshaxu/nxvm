# T539 S25: ENTER/LEAVE Test Migration

Baseline 413375179. Coordinator actual-commit review accepts implementation
P1 2cb8b64f7. Current owns task status; this consumes only the S25 entry in the
[incremental inventory](t539-cpu-incremental-inventory.md).

## Original Coverage

The original 698-line core_machine_enter_leave_smoke.c defines these explicit
instruction contexts, not CTest process counts:

| Family | Contexts | Receiving boundary |
| --- | --- | --- |
| Defaults | 16 | CPU: ENTER levels 0/1/3/33 and LEAVE on 186/286/386, plus 186 level 255 |
| Attributes | 6 | CPU: operand/address-size forms and preserved stack images |
| Rejections | 25 | CPU: two 8086, eighteen legacy attributes and five LOCK forms |
| Protected 32-bit stack | 2 | CPU: sequential ENTER/LEAVE, full register/cache and image assertions |
| Protected faults | 2 | CPU private rollback plus board descriptor/fault/partial-write composition |
| IRQ without shadow | 2 | Board: ENTER/LEAVE, PIC ISR/IRR, frame IP and stack image |

Total: 53 original contexts. Execution coverage is 51 CPU and four
board contexts, because two protected faults have complementary private-cache
and actual board observations. No new instruction qualification is claimed.
Original arrays, profile loops, expected values and assertions are authoritative.

## Design And Sweep

Reuse the existing bounded CPU instruction fixture. Preserve CPU-private
cache comparisons there; do not replace them with incomplete public copies.
Board setup uses real guest GDT bootstrap followed by existing paused register
operations to prepare the original target preconditions. Faulted physical RAM
and real PIC wiring remain board-owned. No new fixture framework or production
API is needed. CPU lifetime and physical move remain S38/S39.

Search direct includers and build/fixture gates together, migrate UD ownership
to the CPU receiver and extend rejection checks for the migrated board file.
Other private consumers remain explicitly assigned to S26-S37. Artifact inputs
remain unchanged unless actual implementation evidence contradicts this plan.

## Implementation Review

The CPU receiver links x86-cpu only. The defaults, attributes and rejections
functions and both complete register/cache comparison helpers remain byte-for-
byte identical to the original. All 28 constant byte-array declarations were
reconciled across the two receivers: 27 remain unchanged after whitespace
normalization. Only the bootstrap-only HLT array is removed; a ten-instruction
budget stops before it. Real IRQ HLT remains. The 186 level-33/255 distinction,
parent display words, allocation/width calculations, sequential 32-bit protected
ENTER/LEAVE and all fault sentinels remain.

CPU-owned protected tests retain the original private cache preconditions and
full rollback comparisons. The two board fault contexts instead reload the
real SS descriptor through existing paused register operations after bootstrap.
They still verify terminal DF, register/copied-segment rollback and ENTER's
three committed stack writes followed by the untouched faulting slot; LEAVE
preserves its faulting memory word. The two IRQ contexts retain PIC source
delivery, ISR/IRR, interrupted IP and stack/BP observations. No production
handler, API, timing grade or machine implementation changed.

## Verification And Similar-Issue Sweep

- Fresh full builds pass on x64 and x86. The first receiving-test run passed;
  no production or fixture fault required a workaround.
- Complete unit suites pass sequentially between widths: x64 380/380 in
  27.36 s; x86 380/380 in 29.77 s.
- All 66 specialized gates pass. The strict matrix has 379 rows: 346 strict
  and 33 unchanged deferred. T344 classifies 105 inventoried and 112 total
  constructors, including 21 shared tails and 84 explicit shapes.
- Six unchanged Shared source/test manifests pass. Documentation governance
  and git diff --check pass; final delivery reruns these document/diff checks.
- The migrated-board negative set now covers nine files and 36 rejected
  private-access forms, alongside the existing 72 CPU and five board controls.

`rg -n 'core_machine_enter_leave_smoke.c' test/app-nxvm -g '*.c' -g '*.h'`
finds no direct includer. The private-consumer query from the inventory finds
92 files: 91 remaining original consumers and the deliberate negative-fixture
string owner. ENTER/LEAVE is absent. Its build registration, UD owner,
constructor/lifecycle classification and boundary negative controls were
updated together. S26-S37 retain the other cases; no unassigned transfer.

## Size And Artifacts

Git numstat over seven test/build paths, including the new CPU receiver and
excluding documents/artifacts, gives 679 added and 518 removed lines, net +161.
The increase preserves two complementary private-cache fault regressions and
makes board setup explicit. The obsolete reset-provider/private fixture route
is removed from the board file; instruction matrices have one CPU-owned home.
No Shared, production, MyNES, INI or asset input changed, so the current eight
0539 EXEs remain valid without a binary rebuild. Retained build trees and S18
recovery patch remain needed for subsequent CPU batches.

## Coordinator Acceptance

Actual P1 review covers both complete C receivers, changed build/UD registrations,
constructor/lifecycle and negative checks, and the original scenario map.
Private cache equality is still checked at the CPU owner rather than inferred
from incomplete copied segments. Original nested ENTER partial-write images,
LEAVE rollback and real PIC delivery remain checked. The ten-instruction
bootstrap budget removes the private halt-resume dependency, not an instruction
scenario. No ABI or production behavior change is hidden in the test split.

The active packet fields, S/P numbering, Queue and remaining-consumer receivers
agree with the actual commit. Executor and coordinator roles were performed
sequentially in one session; this is not an independent-agent review. S25 closes
only its assigned consumer. The 91 original consumers, opaque allocation,
physical Shared relocation and whole CPU/T539 acceptance remain pending.

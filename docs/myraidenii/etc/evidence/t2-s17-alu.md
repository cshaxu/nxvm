# T2 S17 Native Scalar ALU Qualification

Baseline `53e1e775d`, after owner-accepted NXVM T564 S1. This consumes V07's
native scalar ADD/ADDC/SUB/SUBC, OR/AND/XOR/CMP/TEST, INC/DEC/NEG/NOT and
CVTBW/CVTWL (the retained CBW/CWD handlers). Decimal adjustment, multiply/divide,
mode/8080, PSW/entry/control, full prefixes/LOCK, clocks, Machine/board/game
remain explicit receivers, not implied results of arithmetic tests.

Shared implementation P1 `65c16fb0b` contains the complete test batch and
manifest. Product-local evidence follows in P2; coordinator actual-change
acceptance and compact-status closure are recorded by the subsequent
governance delivery. No NXVM/MyNES artifact P is required by unchanged inputs.

## Source And Production Ownership

The pinned NEC October 1986 manual, SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`,
printed 12-29--12-40, 12-47--12-49, 12-64--12-81, supplies the instruction
encodings, signed/unsigned results and defined flag effects. Logical AC is
explicitly undefined; this is the only masked result flag. INC/DEC preserve
CY, NOT and sign extension preserve every flag. No timing value or external
implementation is imported or certified by this result batch.

The existing primary handlers, Group-1/Group-3/FE/FF dispatch, arithmetic/lazy
flags and scalar operand paths pass the new matrix. No production defect was
reproduced, so no speculative implementation, private capability, interface,
state, alternate interpreter or handler reformatting is added. Instruction
source SHA-256 remains
`CC8FD2C29CFD14306340AC5A10FE71FD3A0E61DD891C7315947148750F9BD8AB`.

The native arithmetic and metadata paths were swept with:

```text
rg -n '^static void (ADD|ADC|SUB|SBB|INC|DEC|NEG|CMP|AND|OR|XOR|TEST|NOT|CBW|CWD)|static void _a_' src/x86-core/cpu/cpu_instructions.c
```

NEC-native sixteen-bit branches reuse the accepted private architecture
capabilities; they are not a new Intel profile alias. Existing Intel 32-bit,
protected and count/decimal/multiply/divide paths retain their distinct owners
and tests. Reserved forms remain S12's admission receiver; no reserved opcode
is reinterpreted to enlarge this batch's success set.

## Finite Independent Proof

`test/x86-core/cpu/cpu_v30_alu_smoke.c` constructs actual instruction bytes
from explicit input operations/forms. It does not ask production metadata or
another emulator for expected values. Wide signed-range, unsigned carry and
nibble-borrow arithmetic independently compute results and defined flags.
The complete CPU object is compared, including unaffected high halves and
other registers, segments, instruction pointer and non-result flag bits.

The 917,460 successful concrete input vectors comprise:

| Input family | Cases |
| --- | ---: |
| Binary/TEST encodings, all ModRM and five segment selections, two flag seeds | 95,540 |
| Every byte operand pair for nine operations, with both ADC/SBB input carries | 720,896 |
| Twelve-by-twelve word boundaries for nine operations and both carries | 2,592 |
| Unary Group-3/FE/FF, byte/word, every address/register, prefix and value seed | 30,720 |
| Short register INC/DEC aliases | 1,920 |
| Complete byte-to-word and word-to-long sign-extension input domains | 65,792 |

Every encoding/context family receives observational preview and real execution;
the additional exhaustive byte-value sweep executes actual instructions without
redundantly previewing the same immediate form for every operand pair. Preview
must preserve CPU state, issue only observational fetches and perform no data
transfer or phase. Success must retire exactly one instruction without an
unexpected fault, interrupt, shadow or stop.

The existing one-MiB real-storage fixture records every physical transfer and
external BEGIN/COMMIT/CANCEL phase. The independent checker enforces selected
segment/default SS, sixteen-bit effective offset, even/odd native word lanes,
all reads before writes, exact address/width/accepted ordinal and resulting
bytes including neighbouring guards. CMP/TEST cannot write. Every applicable
read and write position is refused in turn; failure must stop, retain original
CPU state, deliver no fabricated guest exception, and preserve exactly the
already accepted memory-write prefix. Extra or wrong-address accesses fail.
The accepted S8 contiguous data-tail behavior stays explicitly L2; neither
these observations nor sequential arithmetic prove electrical LOCK/prefetch.

## Fixture Cost And Actual Change

One CPU-local real-storage fixture serves this test and its existing consumers.
Its new private restart operation resets CPU and probe metadata without clearing
RAM; the ALU caller initializes every code/operand/guard byte and rejects any
unexpected transfer. Original consumers still call prepare, which clears all
RAM before invoking restart, preserving their original isolation and assertions.
No second provider, modulo-address fixture, public test hook or production
buffer exists. The large value sweep therefore avoids a redundant one-MiB
clear per case, without dropping state, bus or result checks.

Initial estimates were production +0..100/-0..100 and tests approximately
+450..650/-0..40. Actual production is +0/-0/net 0. Tests/registration are
+358/-2/net +356: the new CPU unit +346/-0, existing fixture +11/-2 and
registration +1/-0, excluding manifests/documents/artifacts. Shared fixture
reuse and one input/result checker keep the increase below the estimate
without compressing the original handler style or duplicating a bus. The
first x64 focused matrix passed;
after named-form/read-only-preview/exact-retirement cleanup it passes in
2.50 seconds, including all 65,536 CWD inputs. No production patch or test
budget change was needed.

## Receiving Baseline And Qualification Boundary

NXVM T564 S1 separately accepted the canonical RGB32/four-slot contract and
published PC 0564/MyNES 0044. Its accepted import resolves the former host
256-entry palette-capacity TODO. MyRaidenII's actual board palette/frame/
rotation and real game publication remain the video receiver; no game frame
or new MyRaidenII executable is claimed. Lib/Emulator are not modified here.

Both original Release Ninja/ccache graphs build all targets, sequentially by
architecture. First complete repository unit runs pass x64 667/667 in 106.49
seconds and x86 667/667 in 110.29 seconds, including 180 X86 Core units.
The new matrix takes 2.62406/2.39364 seconds in those suites. No retry,
deadline increase or assertion removal is needed. [Results](t2-s17-results.json)
retain both JUnit suite time and CTest wall time. The current total derives
from the accepted T564 666-unit baseline plus this one unit, not S16's earlier
pre-import 668-unit inventory; the canonical import separately changed tests.

Fresh checks match all ten accepted stripped EXE signatures/widths, sizes and
hashes in the [retained artifact inventory](t2-s17-artifacts.json) to T564.
All production/build/configuration/artifact paths are unchanged. No receiving
executable input changed, so no artificial PC/MyNES relink/deployment or binary
commit is required. PC stays 0564, MyNES stays 0044, and all INIs/ROMs/media and
the other Shared source/test packages are untouched. Existing warning
exceptions remain explicit rather than claimed as waiver-free source.

Manifests, Types, inward dependencies/ownership/negative gates are in the full
suites; direct manifest/corpus and product documentation governance checks pass.
Both specialized targets pass, including all 498 strict rows; no production
warning exception is added. Final actual-change review strengthened two
existing obligations: every preview record must be observational/non-data,
and refused transfers cannot publish an interrupt shadow. After that test-only
assertion addition and manifest refresh, both widths pass the affected
four-case ALU/manifest/Types/ownership recheck. The unchanged fixture/registration
and all other unit results retain their complete-suite proof rather than
needlessly replaying every unaffected test. No external-ROM integration,
manual desktop/RDP, physical audio
or gameplay verification is inferred from ROM-free CPU proof. Integration
suites are not re-executed by this test-only S; the unchanged executable hashes
retain T564's qualification rather than claiming a new game/integration run.
T2's full original native/mode/time/Machine/board domain remains open.

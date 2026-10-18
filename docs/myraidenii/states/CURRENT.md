# MyRaidenII Status

## Current Work

M1 T2 remains open after accepted S23. There is no active S packet between
acceptance and the next admission. The full playable RAIDEN II goal remains
unchanged; ordinary 8080 execution, timing and board/game are not complete.

## Accepted Progress

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S23 | BRKEM/CALLN/RETEM, write-enabled RETI/POP PSW, NMI/INT native entry and return, reset/initialization and prohibited nesting disposition are implemented and qualified at existing CPU owners. Ordinary 8080 execution, general priority/step/HLT, physical retirement/time and Machine/board/game remain open. |

[T2 history](../history/M1-T2-v30.md) retains S1--S23 and scoped deliveries.
The [convergence ledger](../etc/t2-v30-ledger.md) preserves the complete
native/NEC/8080, entry, physical bus/timing and Machine/board domain.

Shared P1 `ad55ccf3f`, NXVM receiving P2 `ad47d64f9` and MyRaidenII evidence
P3 `357586236` are pushed. P3 retains the complete sixteen-field S23 packet.
Production +105/-24/net +81 and tests/registration +406/-1/net +405 yield
code +511/-25/net +486, excluding manifests/records/artifacts. Two private
helpers own control lexical sizing and execution; one documented private
hardware MD-write-enable bit is required, distinct from architectural MD.
There is no new public API, enum, layout, worker, provider or execution loop.
Lib/Emulator and private App source remain unchanged.

## Accepted Evidence And Verification

[S23 evidence](../etc/evidence/t2-s23-mode-transition.md),
[results](../etc/evidence/t2-s23-results.json) and
[ten artifact identities](../etc/evidence/t2-s23-artifacts.json) retain proof.

Final original Release Ninja/ccache graphs build all targets. Complete
repository unit suites pass 673/673 per width, including 186 X86 Core units,
in 94.51/103.27 seconds. The final finite matrix passes 328,192 inputs plus
host refusals/guards in 0.862028/1.16264 seconds. Both specialized targets
retain 498 strict rows; Types, dependencies, ownership/negative, manifests,
documentation and whitespace gates pass. Existing warning exceptions remain.

Eight PC 0564 EXEs are rebuilt/deployed; fourteen selected receiving
integrations plus one restored-default identity pass per width. DeskPro boot
groups finish in 120.85/180.91 seconds within existing budgets. Both original
graphs are restored to default; receiving loops recompile zero CPU objects.
All ten PE/name/width/size/hash identities pass. MyNES 0044 and user INIs/
media/ROMs remain unchanged; existing receiving-PC embedded-firmware artifact
authority applies with no raw ROM import. Test-only physical-alias
strengthening changes no executable input or receiving artifact identity.

Coordinator actual-change review accepts P1--P3 after inspecting the original
request/full packet, actual source and complete new unit, reused transfer
checker, primary manual/typo dispositions, final raw logs, manifests, ledger
and receiving hashes. Dirty-context initialization and physical code/vector
stack aliases were corrected before acceptance; the complete final suites
include both. The earlier pipeline interruption followed a real source
correction, not an observation timeout. No old assertion/budget was weakened.
All final handles have exited; owned rendered PNGs are removed. Governance
closes S23 only, not T2.

The architectural mode remains solely in PSW. Only actual 8080 NOP and NEC
control encodings are currently admitted in emulation; unfinished ordinary
slots stop diagnostically, never fall through to native or silently NOP.
Complete 8080 register/ALU/stack/memory/port behavior is the next V12 receiver.
Packed-adjust/non-ten and CHKIND signedness source questions, general
priority/step/HLT, full prefixes/LOCK, physical prefetch/clocks and neutral
Machine/board/game remain open. Existing data-tail/BRK sampling grades and
V15 DE/BR retirement accounting remain explicit. No fresh complete external,
desktop/RDP/audio, full V30 or gameplay qualification is implied.
No runnable MyRaidenII EXE exists yet.

## Previous Work

T1 source/board/set and integration-contract freeze closed at `7f7d23938`;
[T1 history](../history/M1-T1-board-and-source-freeze.md) retains thirteen
runtime image identities and F01--F13, not game execution.
M0 Td S1 closed at `f21a22997`; M1 remains open toward the full playable goal.
Accepted NXVM T564 supplies RGB32/four-slot host publication; actual RAIDEN II
board palette/frame/rotation integration remains unqualified.

## Queue

[Queue](QUEUE.md) contains five unadmitted receivers after open V30 T2.

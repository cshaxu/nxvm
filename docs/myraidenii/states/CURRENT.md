# MyRaidenII Status

## Current Work

M1 T2 remains open after accepted S22. There is no active S packet between
acceptance and the next admission. The full playable RAIDEN II goal is
unchanged; native software entry is a prerequisite, not emulation or gameplay.

## Accepted Progress

| Task progress | Accepted result / remaining work |
| --- | --- |
| T2 S22 | BRK3, BRK imm8, conditional BRKV, CHKIND BRK5 and ordinary-native RETI frames/returns are qualified through existing owners. Native CHKIND now saves following PC, not the Intel restart PC. Mode/8080, general priority/step, physical retirement/time and Machine/board/game remain open. |

[T2 history](../history/M1-T2-v30.md) retains S1--S22 and scoped P deliveries.
The [convergence ledger](../etc/t2-v30-ledger.md) preserves the complete
native/NEC/8080, entry, physical bus/timing and Machine/board domain.

Shared P1 `470391cf0`, NXVM receiving P2 `2f227277c` and MyRaidenII evidence
P3 `1d40f3ce8` are pushed. P3 retains the complete sixteen-field S22 packet.
Production +5/-4/net +1 and tests/registration +258/-0/net +258 give code
+263/-4/net +259, excluding records/artifacts/manifests. There is no new
production helper, public API, state, worker, provider or execution path.
Lib/Emulator and private App source remain unchanged.

## Accepted Evidence And Verification

[S22 evidence](../etc/evidence/t2-s22-software-entry.md),
[results](../etc/evidence/t2-s22-results.json) and
[ten artifact identities](../etc/evidence/t2-s22-artifacts.json) retain proof.

Both original Release Ninja/ccache graphs build all targets. First complete
repository unit suites pass 672/672 per width, including 185 X86 Core units,
in 323.40/136.98 seconds. The new finite matrix passes 655,265 inputs plus
host refusals in 8.16742/9.66592 seconds. Both specialized targets retain
498 strict rows. Manifests, Types, dependency/ownership/negative,
documentation and whitespace gates pass; existing warning exceptions remain.

Eight PC 0564 EXEs are rebuilt/deployed. Fourteen selected receiving
integrations and one restored-default identity check pass per width.
Receiving-profile loops recompile zero CPU objects; both original graphs
are restored to default. All ten PE/name/width/size/hash checks pass.
MyNES 0044 and user INIs/media/ROMs remain unchanged. The existing receiving
PC embedded-firmware artifact exception applies; no raw ROM is imported.

Coordinator actual-change review accepts P1--P3 after examining the original
request/full packet, source repair, complete new unit and reused fixture,
primary manual, before witness, raw logs, manifests, receiving artifacts
and ledger. The positive CHKIND witness fails before and passes after the
central repair without weakening assertions. Intel restart and S19 NEC
divide policy are retained. All owned build/test handles have exited.
Final governance P closes S22 only, not T2.

CHKIND negative-limit signedness retains its existing model, not a stronger
silicon claim. Packed-adjust questions, non-ten conversions, remaining
native/NEC semantics, mode-sensitive PSW/IRET, general BRK/priority/step,
full prefixes/LOCK, 8080, physical prefetch/clocks and neutral Machine/board/
game remain open. S8 data-tail and existing BRK-sampling grades are unchanged;
DE/BR delivered-fault retirement accounting remains explicit in V15 before
Machine admission. No desktop/RDP, physical audio, fresh complete external
integration suite, complete V30 or gameplay qualification is implied.
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

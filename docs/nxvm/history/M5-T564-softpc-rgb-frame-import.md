# M5 T564 — SoftPC RGB Frame Import

## S5 — Committed shared import

The owner accepted and closed S5 on 2026-10-22. Shared `09ebe0476`, MyNES
`c79285fef`, and NXVM `abb2b10f2` deliver the import, ten EXEs and receiver
qualification. The [S5 evidence](../etc/evidence/t564-s5-softpc-shared-import.md)
records the one native Console failure and passing isolated retry; its cause
remains unresolved. T564 remains open and S2 acceptance remains separate.

Owner-admitted on 2026-10-17 after T563 closure. S1 imports committed SoftPC
dfc85aac4387600033d3ed11d7fb26e74b9eb631 and adapts all ten receiving EXEs.
The [proposal](../proposals/m5-softpc-rgb-frame-import.md) freezes scope;
[Current](../states/CURRENT.md) owns the active packet and acceptance.

The owner accepted S1 on 2026-10-17 and explicitly requested its closure.
S1 is closed; T564 remains open with no active S and no successor admission. The
[qualification evidence](../etc/evidence/t564-s1-shared-frame-import.md),
[results](../etc/evidence/t564-s1-results.json),
[exact import](../etc/evidence/t564-s1-canonical.json) and
[ten artifacts](../etc/evidence/t564-s1-artifacts.json) retain the full batch:
666 units, 48 product integrations and four shared native integrations per
width, both specialized targets and 498 strict rows. The imported bytes also
match latest SoftPC 5e76dd49; X86 Core/MyRaidenII and owner inputs are unchanged.
Initial failures and the two receiving gate corrections are not omitted.
Shared, MyNES and NXVM are separate sequential P deliveries of this one S.
Shared P1 is `d30b69b2a`; MyNES P2 is `906e61b0c`. Both were pushed immediately.
NXVM P3 `b9433b31d` delivers the PC releases and complete receiving proof.
The coordinator reviewed the actual three-commit change set and rechecked all
ten deployed hashes, the 297 frozen import files and the dual-width result
records at closure. The complete original active packet is retained in P3.
The closure governance commits only record owner acceptance and remove the
active S packet; they change no executable inputs and need no rebuild.
Manual RDP/physical-output limits and prior MyNES TODOs remain unchanged.
T-level closure is not requested or claimed.
MyNES governance P4 `366b16a3e` records receiver acceptance separately; NXVM
P5 records S1 closure. Each governance P is pushed immediately.

## S3 — PC Window frame readiness

The owner accepted S3 on 2026-10-22. An unprogrammed EGA reset exposed a
1-column, 1-row, 1-scanline frame; Shared P1 `319bc813e` suppresses that
specific unready capture without changing guest registers or the host renderer.
NXVM P2 `b71dae54d` contains the NXVM reset and Model 40 start/reset/restart
frame regressions, plus eight rebuilt and PE-width-checked PC 0.5.0564 EXEs.
The Model 40 first-start regression failed against the old implementation and
passed with the fix. Real Window behavior was accepted by the owner; the
automated Machine frame observer is not a desktop or RDP measurement.

The first full unit runs exposed three App tests requiring the former 1x1
reset frame, and an x64 Lib boundary timeout. After correcting the test
assumptions, a further x86 full run passed those tests but the X86-NXVM
negative verifier exceeded 30 seconds. Shared P3 `aa39f2308` raises only that
test's timeout to the owner-approved 300 seconds, without changing checks;
NXVM P4 `7a6cdbb33` preserves configured-display, palette, publication and
blink coverage under the new reset contract. Final repository-only unit runs
passed 697/697 on each architecture; the verifier took 37.20 seconds on x64.
Documentation governance and scoped diff review passed. These test-only
corrections do not change runnable inputs, so the eight P2 EXEs remain current.
The owner INI edits were not included. S3 is closed; S2 acceptance and T564
closure remain separate and are not implied by this record.

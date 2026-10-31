# M5 T582 — MyPC110 performance

The owner admitted performance work after accepting T581 on 2026-10-29.
[Current](../states/CURRENT.md) owns the active packet and the
[proposal](../proposals/m5-pc110-startup-clock-performance.md) retains the
whole-product objective. Local optimization is not playability qualification.

## S1 audit

Closed at `990266ae0`. Public-factory measurements and bounded sampling
identified redundant planar pixel expansion and continuing CPU/memory costs.
The RAM-range candidate did not establish a reliable improvement and was not
landed. The [audit](../etc/evidence/t582-s1-performance-audit.md) records the
measurements, rejected candidate, limitations and admitted planar design.

## S2 implementation

Admitted at `96ec0ddca`. The F65535 loop expands eight pixels per four source
bytes, preserving complete snapshot semantics and guest timing. Existing
regression coverage is extended rather than creating another fixture.
The [S2 evidence](../etc/evidence/t582-s2-planar-performance.md) records the
fourteen receiver artifacts, tests, Console-fixture qualification repair and
completed integrated comparison. Shared `d8a94dc61`, NXVM `7c7a98a13` and
MyRaidenII `0d48682cf` are pushed. Coordinator closes S2 under the owner's
automatic sequencing authorization after the full review. S2's single paired
run improves sampled guest ticks by 7.79% on x64 and 2.67% on x86; realtime
performance and playability remain unqualified, so T582 stays active.

## S3 implementation and review

The memory owner proves unambiguous plain RAM spans before omitting repeated
byte queries; the executor reuses one local timing result. Production +31/-10,
existing test +23/-0, no API/state addition. Matched Release fixed-work pairs
execute 30 million instructions at identical ticks/PC, reducing host time
4.57% x64 and 6.61% x86. Earlier unequal-trace probe measurements are explicitly
rejected, not counted as performance evidence. All 14 EXEs were built and
verified, both widths passed 712 units and 48 registered integrations plus
XT/AT/DeskPro selections, and external MyRaidenII receiver checks passed.
The [S3 evidence](../etc/evidence/t582-s3-execution-performance.md) retains
commands, hashes, actual-diff review and limits. Native RDP/Window and whole
product realtime performance remain unqualified; T582 is not complete.

## S26 bounded batch and transfer

Coordinator reviewed the delivered Shared CPU/test change `7a9b89d05`, PC
receiver `8c3045014` and MyRaidenII receiver `602292076`, along with the
preceding S26 evidence indexed in [the investigation](../etc/evidence/t582-s26-investigation-index.md).
The latest CPU change simplifies ROL/ROR without changing its count, flags or
timing contract; production source +36/-161, test source +64/-0, excluding
manifests and documentation. Both-width CPU differential, unit and integration
results, the corrected x86 manifest failure, 14 artifact identities and the
bounded Game observation are recorded in [the qualification](../etc/evidence/t582-s26-rotate-constant.md).
Automated desktop tests are not native interactive RDP acceptance.

Under the owner's automatic S sequencing authorization, S26 is accepted for
its bounded implemented and evaluated work. This does not claim a reliable
speedup, interactive playability or T582 completion. The remaining end-to-end
throughput analysis, implementations and final receiving transfer to S27–S32
in the active [proposal](../proposals/m5-pc110-startup-clock-performance.md).

S27 recorded the dual-width synthetic and existing product workload envelope in [the evidence](../etc/evidence/t582-s27-performance-envelope.md), commit `95d609df0`. Documentation governance and diff checks passed. Current x86 Game, desktop/RDP and actual usability remain open for later receiving; T582 remains open.

S28 traced the CPU-to-device path and evaluated bounded candidates; [the execution-chain evidence](../etc/evidence/t582-s28-execution-chain.md) records rejected experiments and the S29 shift design. No runtime optimization was delivered in S28; PC110 usability remains open.

S29 qualified constant-work SHL/SHR/SAR and fourteen receiver artifacts; [evidence](../etc/evidence/t582-s29-shift.md). The fixed-work comparison did not establish a speedup. T582 remains open for PC110 usability.

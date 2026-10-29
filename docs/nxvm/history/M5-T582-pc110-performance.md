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

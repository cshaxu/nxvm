# M5 T463 Core RTC CMOS Phase Contract

## Closure

T463 closes the MC146818A/IBM AT finite ledger: 16 rows are either implemented
with manual L3 semantics and focused proof, or retained as an explicit L2
board/power/CPU-visibility boundary or excluded L4 electrical limit. The Core
has one 64-byte RTC owner and one guest-clock input route. Profile composition
copies immutable timing values only; it has no runtime callback or setter.

## Evidence And Artifact

S1 source/corroboration is in the RTC checklist; S2 implementation and
boundary mapping is [retained evidence](../etc/evidence/t463-s2-rtc-phase-owner.md).
P2 is `8a384476`; its S2 acceptance is `ad80dad5`. The Debug current gate ran
294 registered tests successfully. The stripped Release developer artifact is
`nxvm_0_5_0463.exe`, SHA-256
`AFA07C1DF22C624B3E1562E002F9FBC92F55AE4EB64DA254EF7F4DA75C34C7FB`.

## Boundaries

Board power/VRT policy, host persistence, exact board-to-PIC/CPU visibility,
and an unavailable time-register bus result during the manual update window
remain L2. Electrical oscillator, pin and bus AC tables remain excluded L4.
IBM AT byte 32h is board/firmware map state, never a generic RTC century field.

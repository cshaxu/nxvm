# T582 S6: display observation proof

An exploratory task-owned wrapper checks existing F65535 captured generation
and cursor phase, returning UNSUPPORTED instead of executing capture when
unchanged. It is expressly not production: it ignores force semantics, borrows
private chip layout, and does not implement a proper observation contract.
It only bounds performance potential. A 30-second startup run skipped 1,445
captures and invoked 124; it exited normally.

At fixed 10,000,000 retired instructions / 79,964,294 guest ticks / PC
0003dbc5, x64 baseline was 19,552 ms with 1,017 captures and candidate
20,303 ms with 941 skipped; x86 baseline was 22,998 ms with 1,134 captures
and candidate 28,496 ms with 1,149 skipped. These single sequential pairs
do not prove a stable regression, but neither establishes useful early-boot
speedup. The guest reaches different screen phases as host time advances.
A separate 300-second paired x64 product-path observation is in progress to
inspect late graphical frames and guest progress. No production code changed.

A chip reset clears captured and restarts generation. Any eventual observer
must request first capture even if an old acknowledged numeric generation
coincides. A VSync-derived cursor phase can change without memory generation;
a force request must bypass observation. Captured-generation acknowledgment
only after a valid complete publish and failures remaining visible are
mandatory. The provisional wrapper does not prove any of these contracts.

## Completed 300-second product-path pair

Both x64 probe runs exited zero at 302.61/302.64 host seconds. Baseline last
sampled 841,609,350 guest ticks and 155,247,314 retired instructions; the
exploratory bypass sampled 864,330,995 ticks and 159,052,871 instructions
(about 2.7% and 2.5% higher). Baseline captured 16,705 times; the candidate
skipped 15,739 and captured 944. Both first matched the retained exact
Personaware menu frame at sample 24, about 250 seconds. Final RTC seconds
were 25 and 26 respectively. One sequential pair is not a variance study.

The bounded improvement does not justify extending the cross-component
display contract and its semantic risk as an answer to a system still far
from realtime. The private wrapper is invalid as production because it
bypasses force and returns UNSUPPORTED; no implementation is landed. The
unchanged-frame work remains a measured opportunity, but T582 continues
toward a larger hotspot with no claim that PC110 playability is fixed.

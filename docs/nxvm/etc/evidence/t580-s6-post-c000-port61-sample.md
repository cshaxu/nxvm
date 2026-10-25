# T580 S6 — Post-C000 port-61 sample

## Question

S5 established that the selected C&T bank returns to `F000:3F6C` and then
reaches the pre-existing port-61 wait. This S checks whether that second wait
is caused by a stalled port-61 status source before assigning another device
owner.

## Method

`mypc110.diagnostic/real-firmware-c000-port61-follow` uses the ordinary Core
executor and the admitted external TFT BIOS. It first requires the existing
C000 boundary and return, then records the first sixteen retired byte reads of
port `61h` after that return. The 13,000,000-instruction ceiling is containment
only; the pass condition is the sixteen actual reads, not the count.

The diagnostic consumes Core's existing copied retirement observation. It does
not add a port interception, a second clock, a firmware-address behavior, or a
production device model.

## Result

Both x64 and x86 pass. They have the same checkpoint and sample:

```text
after-c000=f000:00003f6c bytes=8b c5 5d 89
pc=000fea86: port-61 values ... 23, 33, 33, 33, 23, 23, 23, 33 ...
ticks ... 80779496, 80779521, 80779546, 80779565, 80779584 ...
```

`23h` has port-B bit 4 clear and `33h` has it set. The values alternate on the
sole Core elapsed-tick axis. The BIOS is still executing its own `EA86h--EA90h`
delay body when the finite sample ends; its remaining loop count is not a
device failure.

## Disposition

There is no stalled port-61, PIT, Core-clock or F65535 receiver. This sample
does not qualify firmware POST, panel output, Easy-Setup, storage, a guest OS,
or the unavailable original-DSTN variant. A later T580 receiver must start
from a named qualification checkpoint rather than treating the next arbitrary
retirement as a hardware defect.

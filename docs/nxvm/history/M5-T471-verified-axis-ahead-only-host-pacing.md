# M5 T471 Verified-Axis Ahead-Only Host Pacing

T471 corrects the T470 closure finding that a future verified-axis Standard
HLT path could advance to a Core deadline before comparing completed guest time
to wall-clock budget. It establishes one session-local comparison baseline:
Core advances guest time; Standard waits only when completed Core time is
ahead; Turbo removes only that wait.

The comparison is overflow-safe rational arithmetic over copied Core ticks/rate
and a raw monotonic host counter/frequency. It neither derives guest ticks from
host time nor adds a second scheduler. A synthetic verified-axis PIT deadline
proves the Standard gate and Turbo omission; all product profiles remain
unqualified until the existing physical-timebase debt closes their full source
chain.

The stripped Release `build/output/nxvm_0_5_0471.exe` is target
`vm-0-5-0471`, banner `0.5.0471`, SHA-256
`38D78E1C1AE6B4E877B116B3E3EBF92B6F8E83F378CA9D5075C4871955E8D2FD`.
The closure audit retains focused and full-gate verification.

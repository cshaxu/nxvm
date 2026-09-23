# M5 T35 S2 - Six-Input Execution Evidence

The owner-local probe now includes NROM in addition to MMC1, UxROM and the
three MMC3 roles. With the six supplied local inputs configured, both x64 and
x86 completed graphics and text runs for every role: 1,000 driver slices each,
26--28 published frames and no Core trap.

The test host had no native audio endpoint. Driver construction correctly left
the optional audio sink absent; the probe reports a zero queue and still proves
machine execution/frame behavior. It does not claim host-audio delivery from
that environment. ROM paths and bytes are not recorded.

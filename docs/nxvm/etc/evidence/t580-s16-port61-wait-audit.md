# T580 S16 — PC110 post-C000 port-61 wait audit

The real TFT diagnostic reaches the firmware delay loop at `F000:EA86` after
the C000 option-ROM return.  It samples port 61h bit 4 while the loop runs and
later executes the routine return at `F000:EA92`; it is therefore not a
permanent halt.

The current MyPC110 board plan selects the existing Board port-B owner with
`ELAPSED_TICK_TOGGLE` and a three-tick period.  That owner is deliberately on
Core's sole elapsed-tick axis. It exposes bit 4 while preserving the existing
PIT channel-2 bit and writable low port-B bits; it does not add a second clock
or claim a physical refresh frequency.

The direct x64 run after the C000 return observed port-61 values
`20, 23, 33, 33, 33, 23, ...`: bit 4 changes as required by the firmware
poll, while the established bit-5/PIT behavior remains visible. Existing
two-width `real-firmware-c000-port61-return` regressions already prove the
routine returns. The plan construction unit also asserts the selected source
and period.

No model defect was reproduced. S16 deliberately changes no production or
test code; treating an additional post-return instruction ceiling as proof of
the next device would reintroduce the rejected arbitrary-progress criterion.

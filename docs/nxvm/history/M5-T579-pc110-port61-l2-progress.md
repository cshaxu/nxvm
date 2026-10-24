# M5 T579 — PC110 port-61 L2 progress

## S1 closed delivery

The MyPC110 plan selects the existing elapsed-tick port-61 status source with
a three-tick L2 period.  The ordinary TFT BIOS reset enters and then exits its
unprogrammed-PIT1 wait loop at `F000:EA86--EA90`, reaching `F000:DD05` without
a delivered exception on x64 and x86.  App unit tests and the bounded
external-firmware diagnostic pass on both widths.  This closes only S1's
finite progress gate; it does not qualify POST, video, Setup, guest boot or a
runnable PC110 artifact.

The current packet owns the still-active S3 scope and completion facts.

## S2 closed delivery

The bounded continuation found the actual BIOS's `3BCh--3BEh` control/status
probe after the port-61 wait.  A MyPC110-only L2 LPT placeholder records the
firmware's `21h` control write, returns `90h` by default and `D0h` while that
probe is active.  It is based on independently reviewed PC110-EMU behavior,
not represented as physical printer hardware.  Both widths pass the new unit
contract and retire beyond the probe to `F000:55B2`.  S3 remains active for
the next distinct finite boundary.

# T592 S3 — Original OmniBook pop-out mouse

The owner verified that Alt navigates Windows menus and that closing
Windows returns to DOS. The guest showed a pointer but it did not move.

The original `SYSTEM.INI` selects `D:\OBMOUSE.DRV` and the matching
`MOUSE.INI` selects `OEMMOUSE`. The OEM branch of the original driver
programs `0x238` and `0x23B`, reads four consecutive bytes starting at
`0x238`, and uses the high bits of the last byte for active-low buttons.
The previous App forwarded incoming mouse events to the 8042 auxiliary
device. In the original Windows replay those reports reached the board
with their expected deltas and returned `LIB_STATUS_INVALID_STATE`.
The synthetic PS/2 regression had enabled that auxiliary device itself.

The App handles relative input through its existing Machine profile
callback and exposes two wrapping 12-bit counters, button state and
movement/button interrupt controls on the HP ports. It uses a source on
the existing IRQ12 PIC path. The BIOS auxiliary-device path remains
available for its own commands. The model uses the OB300 guest evidence
and the [OB600 pop-out mouse driver report](https://lkml.rescloud.iu.edu/0401.0/0861.html)
as corroboration for the related part. Host-delta mapping and interrupt
rearming are compatibility models. The hardware behavior is not claimed
to be measured on a physical OB300.

The existing board smoke now takes the OEM IRQ through actual guest
execution and verifies coordinate wrapping and released buttons. The
Machine fixture also feeds a neutral mouse event through its production
driver and reads the changed OEM registers. Both host widths passed.

On the x64 original Windows replay the driver wrote `08` at `0x238`
and `30` at `0x23B`. It initially read zero coordinates and released
buttons. Four test motions drove the first raw counter through `FE9`,
`FD2`, `FBB` and `FA4` while the second advanced through `00B`, `016`,
`021` and `02C`. The driver read each report from its own four ports.
Published-frame capture showed the guest pointer displaced to the right
and down in Program Manager. Native host capture and physical clicking
were not exercised by the background replay.

| Verified observation | SHA-256 |
| --- | --- |
| Prior build rejects real mouse motions on the auxiliary path | `61b47c504aff089edbf607ea7e65323eb574713124d7f39695af7d0387ed4288` |
| Original driver's OEM port programming and coordinate reads | `ce9841ce2532338700c18a9c6a8762e80d466111882bbe1a44a176ee344a7888` |
| Original Program Manager frame after movement | `c40fa490c676d484dbfed803f3b84535480df44551a16966b75d6d6ade8f09c5` |
| Published dual-width PE and exact native launch checks | `9c92cebee8cbff44704a13e8abedfe60acac33c12b6c3d53cff363ee7196e3ed` |

# T589 S6 OmniBook 300 input device qualification

The [prior input handoff](t589-s5-ct65510-display.md#input-handoff) combines
the HP service manual's 8042 AUX POST path with the original System 1.0
DOS mouse driver's INT 15h C200h enable/disable calls. The service manual
covers several host models. Apply its controller protocol and validate the
actual OB300 board configuration with the original firmware in the next
stage. The DOS driver also performs HP-specific initialization outside
the established mouse enable/disable calls.

The existing X86-NXVM PS/2 Mouse, Keyboard, and 8042 implementations satisfy
the sourced device subset. Reuse the AT controller adapter for serial
delivery, guest-command acknowledgements, reset and IRQ wiring. It exposes
an explicit extension callback for otherwise unhandled board-specific
commands. Bind a callback only if an actual HP command needs it.

The existing tests passed **8/8 on x64 and 8/8 on x86** in the cached
release trees. The batch includes the keyboard and mouse chip contracts,
8042 combined contract, the board AUX route, controller/serial cadence
and the default NXVM host keyboard and guest AUX paths. ROM and media
are not required by the batch. The guest AUX test uses a synthetic entry
plan, writes D4h/F4h through ports 64h/60h, checks the ACK on IRQ12, then
passes relative motion and buttons through the existing host ingress and
observes the three bytes in the guest IRQ handler. The keyboard contract
checks scan sequences, reset and typematic. The 8042 test exercises
interface commands and separate keyboard and AUX reset replies.

No Shared or App code change was required. The HP board must still
establish the 60h/64h mapping and AUX presence, IRQ1/12, reset/A20 outputs,
strap and command-byte values, and any firmware-specific controller
extension. The existing tests prove the device and reference board
contracts; original HP POST, INT 15h callback delivery in the owner ROM,
and Windows mouse behavior are later receiver proofs.

| Verification log | SHA-256 |
| --- | --- |
| x64 | `5914131c8a889e48a3ac17cd7d7fd9cc800b03bd13d03be9c4d1b42e9a0e00bc` |
| x86 | `d1b9fababec70ae25f5e8d89c95b9fafeb960bb2d60fc6bf5d7a547d4f70b644` |

# M5 T580 S58 — RB5C396 empty-controller receiver

## Scope and evidence

The unchanged TFT BIOS image is an admitted external input, SHA-256
`232101C88466F311BCC32FBC215A4D7569F695CE19F9C07CA67CE2AEE5232312`.
Static disassembly identifies its PCIC index/data access routines at 3E0h and
3E1h.  The existing S55 observation also records the firmware's earlier
selector probes `00h`, `40h`, `80h` and `C0h`; no prior App route owned those
ports.

Ricoh's *RF5C296/RF5C396L/RB5C396 PC Card Controller User's Manual*,
EA-028-9804 (June 1995), SHA-256
`7EB037B495D7ED3F4334A1CDC6C6582C8FA9CEADF0FD680EBADEC2575B60C7EC`,
is the primary register source.  Its printed pages 27 and 43--45 establish
the index/data register interface, device and socket selector bits, two
socket banks, read-only `83h` identification and `B2h` chip identification,
and the interface-status pins.  The receiver is therefore source-labelled L2:
it models documented controller-facing behavior, not an observed physical
PC110 card population or timing capture.

## Implemented ownership

`x86-nxvm/chips/rb5c396` owns one controller's selected index, two 60-byte
register banks, read-only identification and board-supplied socket input
levels.  Reset clears controller-local selection/register state while
preserving physical socket inputs.  The model has one controller device, so a
selector with device bit 7 set reads `FFh`; socket bit 6 selects socket 0 or
1.  `app-mypc110/pc110_pcic` owns only the existing Core byte-port decode for
3E0h/3E1h and does not introduce a second machine path.

The App deliberately supplies no card-present input.  It therefore does not
claim a PC Card memory/I/O window, card IRQ, PCMCIA-ATA route, medium owner,
or insertion/removal behavior.  Those require a source-backed PC110 card and
board-wiring receiver and remain open.  No BIOS-PC interception, CMOS seed,
ROM/media mutation or host PCMCIA API is used.

## Verification

The new chip contract checks both socket banks, documented IDs, empty and
present socket status, power status, independent writable registers, invalid
controller-device reads, read-only registers and reset retention of physical
inputs.  The App contract installs the actual Core routes and verifies the
same index/data behavior through the bus.

Both x64 and x86 rebuild and pass:

- `x86-nxvm.chips/rb5c396/rb5c396_contract`;
- `mypc110.unit/pcic`.

The existing bounded real-firmware Setup trace is extended only to record
3E0h/3E1h accesses.  The previously observed Enter/default route did not
reach the BIOS PCIC routine, so this fixture is a future non-regression route
observation rather than proof that the newly modelled registers are consumed
by that UI path.  It must not be used to claim card, Setup, DOS or product
qualification.

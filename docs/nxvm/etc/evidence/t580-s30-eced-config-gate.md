# T580 S30: EC/ED configuration gate

## Scope

This receiver corrects the PC110 App's ownership of the existing `ECh`/`EDh`
configuration window.  It does not add a chipset model, modify the selected
firmware, alter Shared/Core contracts, or claim that the admitted BIOS has
reached this later path.

## Evidence and model

`Open-Source-PC110` commit `02ccf2e8e19b298a2cf04c5d430278eb087cc368`,
`Discovery/Chipset/readme.md` section 13j.2, records the two-byte protocol:
`out FBh` opens the EC/ED configuration window and `out F9h` closes it.  The
same record describes the gate as wrapping an access block and documents a
live EC/ED sweep using that order.  Its BIOS-flash research independently
records the identical open, EC/ED accesses, close sequence.

The App's existing firmware binding is the sole owner of the selected index,
EC/ED side effect and C-segment shadow policy.  It therefore now retains the
one reset-closed gate bit.  While closed, EC/ED reads yield `FFh` and writes
are ignored: this preserves ordinary successful I/O rather than promoting a
closed hardware window into a Core execution failure.  `FBh` opens and `F9h`
closes; no latch value, delay, controller clock, ROM-address rule or host
service was invented.

The independent PC110-EMU and PC110-QEMU checkouts remain research references
only and no code was copied from them.

## Verification

`mypc110.unit/config-route` exercises closed read, open access, close, and
Core reset reopening requirements through actual installed ports.  The real
firmware diagnostic now retains `F9h` and `FBh` in its existing trace filter,
but its present checkpoint does not observe either port.

On 2026-10-25, GCC x64 and x86 each built the affected test graph and passed
all 253 `x86-nxvm|mypc110` tests, including 19 MyPC110 cases and eight bounded
real-firmware diagnostics.  This is receiver-contract evidence only, not
POST, setup, storage boot, panel or artifact qualification.

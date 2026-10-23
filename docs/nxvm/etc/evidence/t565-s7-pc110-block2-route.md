# M5 T565 S7 — PC110 Block2 port gate

## Evidence and ownership

Open-Source-PC110's chipset discovery records the PC110 Block2 indexed window
at `24h/25h`, the four read gate `FC23h`, `F023h`, `C023h`, `0023h`, and the
`22h` bit-8 relock operation.  The existing X86 Core VL82C420 component already
models that ordered protocol independently of I/O addresses.  Therefore the
PC110 App route adapter owns only address decoding and calls the existing chip
operations; it does not carry a second progress or lock field.

## Guest-facing gate

While Block2 is closed, `25h` reads return `FFh` and `24h/25h` writes are
ignored.  This is an L2 board-facing representation of the observed protected
window: it prevents the internal `INVALID_STATE` guard from becoming a guest
I/O error.  The four reads in order open the existing indexed bank; a write to
`22h` with bit 8 set relocks it.  No Block2 reset image or register meaning is
claimed here.

## Direct verification

`mypc110.unit/config-route` drives the production Core port bus and verifies
the closed state, an out-of-order sequence, the exact ordered sequence,
read/write access after unlock, and relock.  It also retains the previously
admitted SCAMP and EC/ED route checks.  `mypc110.unit/firmware-binding` is run
alongside it because both use the PC110 board library.

Both established Ninja/ccache graphs passed:

- `mingw-gcc-x64-release-ccache`;
- `mingw-gcc-x86-release-ccache`.

This is not BIOS, POST, display, storage, input, power, desktop or hardware
qualification.

# T7 S71 — MAME L2 Sound Boundary

## Bounded Result

The local `O:\repos.external\mame-0.289\mame.exe` reports MAME v0.289.  Its
one `raiden2 -verifyroms` attempt against the owner-supplied external archive
ended before emulation: the archive lacks `jj4b01__mmipal16l8bcn.u0341` and
`jj4b02__ami18cv8-15.u0342`, the two PAL identities required by that MAME
revision.  The latter is reported by external MAME material as having no
known-good dump.  No PAL was synthesized, fetched, copied or retained.

The task-owned output directory was empty after the failed verifier and was
removed.  This record retains no ROM/PAL bytes, hashes, frames, audio, guest
addresses, opcodes, traces or third-party source.

## L2 Source Boundary

The local MAME v0.289 source snapshot at
`O:\repos.external\mame-raiden2-l2\raiden2.cpp` was inspected only as an L2
reference.  Its Sound map assigns the two OKI roles at the same local device
addresses already owned by `src/myraidenii/core/sound.c`.  That finite layout
agreement does not prove a ROM run reaches either voice device and does not
license source, table, PAL or decryption import.

Accordingly this S selects no Sound code change.  The absent MAME runtime is
not a physical-evidence blocker: future bounded L2 receivers may continue to
use independently inspected external-emulator behavior where it supplies a
specific contract.  A genuine runtime observation requires a lawfully
owner-supplied complete set compatible with the chosen MAME revision.

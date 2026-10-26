# M5 T580 S61 — PC110 extended-CMOS seed disposition

## Current implementation boundary

`x86-nxvm/machine` accepts an optional 64-byte CMOS payload and deliberately
copies only configuration indices `0Eh` through `3Fh` into the existing Board
RTC defaults.  The Board already decodes the PC110's extended `70h`/`71h`
address range through `7Fh`, but its default-byte capacity is also exactly the
same 50 entries.  Therefore the current route cannot initialize an extended
configuration byte such as `44h`, `72h`, `73h`, or `78h`.

This is a real representational gap, not a live-RTC ownership gap: Core still
owns RTC registers and the Board remains the sole default-image consumer.

## Source and asset audit

Open-Source-PC110's `Discovery/Live-Dump/README.md` says that a full 128-byte
live CMOS capture was recorded in `ports_indexed.json`; its PS2 disassembly
record independently identifies persisted settings in the `40h`–`7Fh` range
and a `40h`/`41h` checksum.  The locally pinned repository commit `02ccf2e`
does not contain that JSON file or any raw `.cmos`/NVRAM payload.  Its public
documentation supplies several individual field meanings, but not enough
authoritative bytes or checksum semantics to construct a faithful complete
seed.

The external `nxvm-assets` PC110 profiles contain the selected BIOS and font
payloads, but no CMOS seed.  The source policy requires such a protected/raw
payload to remain external with provenance and a hash; this task did not fetch,
create, mutate or commit one.

## Disposition

Do not expand the generic seed from 64 to 128 bytes yet.  Such an interface
change would be mechanically straightforward but would add an unconsumed
capability while silently inviting invented bytes.  A later receiver may be
admitted only after an owner-supplied or lawfully retained exact 128-byte
capture is placed in the external PC110 profile archive with provenance and
SHA-256.  It must then extend the existing single seed/default route once,
preserve Core's live-RTC ownership, test `0Eh`–`7Fh` coverage and prove the
selected firmware consumes the supplied configuration.

Normal Easy-Setup persistence, guest boot, PC Card mapping and product
publication remain unqualified.  No production/test code changed in S61.

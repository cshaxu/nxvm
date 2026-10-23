# M5 T565 — PC110 Board, Chipset And Firmware Mapping

## Closure

The owner asked that T565 continue through a clean, bounded closure.  It is
closed as the source-backed PC110 mapping foundation, not as a complete or
bootable PC110.

Accepted implementation:

- S3 adds the address-neutral VL82C420 protocol owner in X86 Core.
- S5 binds the proven SCAMP and EC/ED physical windows in `app-mypc110`.
- S6 binds an exact-length TFT BIOS image into the existing immutable Core ROM
  provider at `C0000h..FFFFFh`.
- S7 binds the exact Block2 gate through that same sole chip owner and removes
  a redundant App-local protocol state.

Each admitted implementation passed its direct x64/x86 Ninja/ccache tests;
S7 additionally reran the existing VL82C420 Core regression on both widths.
No PC110 executable, raw firmware, media or customer configuration was
created or committed.

The [S8 closure audit](../etc/evidence/t565-s8-board-closure-audit.md)
explicitly transfers all remaining board composition, endpoints and guest
qualification work.  In particular, the existing all-in-one X86 NXVM board
constructor cannot be repurposed as PC110 merely because several standard port
families are present: it also supplies AT-specific display, KBC, A20, reset and
profile behavior.  The new [base-composition candidate](../proposals/m5-pc110-base-composition.md)
owns the required safe extraction and construction proof before later PC110
input, display, storage, audio and firmware-boot tasks attach devices.

The archived [original proposal](M5-T565-pc110-board-proposal.md) remains the
chronology and original scope; this record and the S8 audit are the authoritative
closure disposition.

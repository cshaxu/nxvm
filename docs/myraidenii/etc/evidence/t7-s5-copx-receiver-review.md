# M1 T7 S5 — COPX Receiver Review

## Current ownership audit

The selected-set inventory names `copx-d2.u0313` as a 262144-byte runtime
image.  It is not a hidden omission in a later adapter: the current source
has exactly twelve `myraidenii_machine_image` roles, twelve Product INI keys,
twelve Machine identity entries, and a Board configuration with no COPX
member.  The selected-set wrapper deliberately stages the same twelve roles.
The Board COP model has no COPX pointer, copied storage, address range or
lookup callback.  Consequently, adding an INI key or Machine image enum today
would create an inert, misleading second input route rather than implement a
hardware behavior.

## External reference review

The inspected external reference was the MAME source published at
`src/mame/seibu/raiden2.cpp` and `src/mame/seibu/seibucop.cpp`, accessed on
2026-10-20.  `raiden2.cpp` declares a separate 0x40000 COPX region for this
set, matching the inventory size and identity.  Its hardware note characterizes
the COPX-D2/D3 purpose as *probable* lookup/math usage.  The corresponding
COP device source contains no COPX-region handle or explicit address/read
contract.  Its own comments identify incomplete documentation and remaining
play-testing work.

These are L2 facts only: they establish that the external model recognises the
separate image and that the available reference does not yield an implementable
lookup contract.  They are not L3 because no inspected primary manual PDF
provides an applicable COPX register, address, formula or table rule.  No MAME
code, tables, ROM bytes or algorithm was copied or transliterated.

## Disposition

COPX remains a required selected-set input identity but has no admitted Board
consumer.  This packet deliberately makes **no** source/API/configuration
change.  Any later implementation must first establish a finite Board-owned
contract: the requesting register/command, address or index calculation,
result width/order, failure behavior and synthetic non-ROM oracle.  A local
guess would be L1 and is not silently substituted for that receiver.

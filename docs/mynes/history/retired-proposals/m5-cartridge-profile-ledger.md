# M5 Cartridge Format And Profile Ledger

## Outcome

Replace M4's game-size-specific cartridge admission with a bounded descriptor and profile ledger. It accepts exactly the declared Mapper 0/1/2/3/4 iNES profiles, retains legacy title-suffix normalization, raises the media size limit only to the declared maximum, and rejects every unlisted format or variant before machine publication.

## Acceptance

Header/length fixtures cover all six supplied-profile shapes, malformed headers, trainer/battery/RAM flags, size overflow, suffix normalization and every profile rejection. Core receives an immutable descriptor, not raw header policy. Mapper 2/3/4 packages can consume its profile fields without reopening format parsing.

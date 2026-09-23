# M5 Mapper 2 UxROM

## Outcome

Implement the declared Mapper 2 UxROM profile, including switchable/fixed PRG windows, CHR RAM, fixed mirroring and the chosen legacy/submapper bus-conflict policy. Jackal becomes a bounded six-input acceptance receiver.

## Acceptance

Production cartridge/MMIO fixtures prove bank wrap, reset state, fixed final bank, CHR writes, mirroring, bus-conflict behavior and malformed-profile rejection. The owner-local Jackal scenario passes both presenters and lifecycle boundaries on x64/x86 without a Core trap.

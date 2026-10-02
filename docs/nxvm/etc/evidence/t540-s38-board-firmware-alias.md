# M5 T540 S38 Board Firmware/Reset Alias Ownership

## Actual source diff

The existing CPU-profile high-reset-alias calculation and F0000h source
selection moved from mixed `machine.c` to `machine_board.c`. The sole public
`core_machine_bind_firmware_provider` implementation moved from the generic
firmware-operation file to that board owner. It still validates the same
configuration boundary, invokes the provider's one configure callback,
derives the same alias only after source ROM exists, and publishes the same
provider on success. Configure and alias failure now meet one common rollback
block: restore the prior immutable-ROM mapping count, revoke provider/context
and clear the firmware operation context. This deletes the duplicate cleanup
branch without adding an alternate bind or mapping path.

`machine_firmware.c` retains only neutral operation guarding and bounded
firmware reads/writes; `rom_mapping_interface.c` retains the one copied-image,
alias, route-priority and rollback owner. The 8086/8088/80186 no-high-alias,
80286/80386 high-alias, already-present alias, short/unrelated F0000h source
and partial-registration cases use the same conditions and arithmetic as
before. The separate firmware-less high-reset RAM fallback remains assigned
to S39, not silently changed here.

Four NXVM source files changed: **116 lines added, 129 removed, net -13**.
No public API, second ROM table, new callback or profile-name switch was
introduced. Shared/MyNES and owner INIs are unchanged.

## Verification

- Final-source repository-only units: x64 **469/469**, x86 **469/469**.
- Specialized NXVM gate target passed, including direct strict compilation.
- Existing firmware bind/rollback, immutable-ROM, reset-alias and four-profile
  tests are included in the complete unit runs.
- External boot checkpoint passed once for Default, XT, 5170 and Model 40
  per width: **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` verifies four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` section.
  SHA-256 by profile, x64 then x86:
  - Model 40: `FCED1B945F82958FB32DDD2A5F3B8B416B6C682E1B389028965D7122143C65B3`, `EF448B79B7BBCB49DD3ED5A50BC5601881EDF4FFF4A410342B917A35C926CF5F`.
  - Default: `FA0724EDC44C86237F7C1A12A298A2FA26ADE86B7048055C5396EBE9A3E3556F`, `6A170975BE33B2F84B4074BBFCD4F03A379D42A31210230C3DAA447D2A877E83`.
  - XT: `19C06D8AAB497BF7B6F26E16B8A875DDF5DC7526837B82ED2CC812B667CB67E2`, `73DE1E6137CF4444DC79868E53A06BDFAE5B7D0C629683731F26B9FA173BE775`.
  - 5170: `0AB6CC30C72BD42E39EB990E7EA5D8035D8AB1ABE6551F2A380E4E0C3DF123D4`, `B577D0D146D01C4970CEDB80BA732E5448132860BFE8B9EF00C06E3EECED4203`.

No protected firmware/media, time formula or timing grade changed. T540
remains open for S39-S43 and subsequent shared board extraction.

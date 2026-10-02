# M5 T540 S35 Teardown Ownership

## Actual ownership diff

`core_machine_destroy` remains the one public route for both completed
machines and post-construction failures. It first revokes the firmware
provider/context, invokes one private board-device finalization stage, then
releases CPU/FPU, port, immutable ROM routes/images, memory, trace, bus and
the machine allocation. The board stage preserves the prior order: system
and auxiliary PIT, HDC, FDC, DMA, RTC, exclusive XT keyboard/PPI or AT KBC,
PIC, VADP. It does not release neutral Core objects or add a second
destructor. Pre-construction allocation/FPU/CPU failures retain their
original local cleanup because no complete machine has been published.

Final destruction now calls the existing
`core_machine_rollback_immutable_rom_mappings(machine, 0u)` before memory
finalization. That route unregisters each mapping and releases bytes only
when `owns_image` is true; aliases do not own or release another mapping's
image. This deletes the separate final-destroy image loop and keeps one ROM
ownership rule for rollback and teardown. The original null/partial-machine
and success-only publication behavior remains intact.

The production diff changes three NXVM files: **21 lines added, 21 removed,
net zero**. No new public API, board framework, asset path or media lease
was introduced. Entry/ROM/trace qualification remains S36 work.

## Verification

- Final-source repository-only units: x64 **469/469**, x86 **469/469**.
- Current specialized gate target passed, including the direct strict-
  compilation matrix.
- Existing ROM owner/alias, route transaction, reset alias, plan rollback,
  partial-construction and four-profile tests are included in the full runs.
- External boot checkpoint passed once for Default, XT, 5170 and Model 40
  per width: **8/8**.
- Eight optimized 0540 products rebuilt. `objdump -f` verifies four
  `pei-x86-64` and four `pei-i386`; `objdump -h` finds no `.debug` section.
  SHA-256 by profile, x64 then x86:
  - Model 40: `810C5837243379958A64730483B68F2526D33B2DD67124B75AEEE340FA6CD65C`, `C6ECD24D61448FCD48042A404199D904182AF62F56E8A229BE7EB6BCB5399176`.
  - Default: `E73E09BE9871952964A325C5BA7C8A43DA3A8655173A085FC8A8EE939B7DF4DF`, `2D3ACAE1E82AAF238DF86A1AA7B46FB59645239175D9F946EBBF59B0061CCE0D`.
  - XT: `81F06AFA57263D0B21ADC2C34BDADCE6C225536E9BCCC2B0E799FD9437BBCDAD`, `F5A79529456E8DD9751426DF6D3274917BB66D674F5B514CD9203FD7F7989A74`.
  - 5170: `0A269FBD59F6F6A157064375294484419F0491A5F250BB609D146758BAC42EEF`, `4A7975C97E3DF2A867500A43141B67396C070E7A8B8A2310BCBBDA03D9AF1675`.

No Shared/MyNES, owner INI, firmware input, protected asset or timing grade
was modified. T540 remains open for S36 qualification and physical moves.

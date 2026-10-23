# T568 S1 — PC110 F65535 and Font-ROM Admission

## Scope and result

This is an admission record only. It audits the existing MyPC110 base and
the available PC110 display/font evidence; it neither adds a display device
nor embeds a ROM. The result is deliberately split into a bounded font-ROM
receiver and a separately source-qualified F65535 receiver.

## Current code route

`app-mypc110/pc110_machine.c` currently composes a 486SX, neutral Board
controllers, the TFT firmware map and the VL82C420 configuration route. It
selects no display topology. `x86-nxvm/chips/video` plus Board-Base `vadp`
is the existing PC/AT CGA/EGA/VGA adapter and snapshot owner; it is not a
generic F65535 implementation. Its public register vocabulary, memory
regions and personality setup encode those existing adapters.

Core Machine already supplies the needed ordinary device-provider mechanism:
an App can install an immutable memory reader and a port route before the
execution providers are frozen. No Lib, Emulator or frame contract is
needed merely to expose the PC110 font window.

## Evidence ledger

| Row | Finding | Grade | Owner and disposition |
| --- | --- | --- | --- |
| Controller | The PC110 uses a C&T F65535 revision 1 display controller. | L3 identity; register behavior requires the cited C&T Revision 2.0 register manual. | A distinct `x86-nxvm/chips/f65535` mechanism candidate, never a VADP personality. |
| Panel | The original machine's internal panel is 640x480 indexed-color DSTN; TFT conversion uses a different BIOS pair. | L2 for pairing/model until exact selected-ROM observations are frozen. | App chooses the build-fixed identity; no host scaling policy. |
| Frame capacity | Existing copied-frame storage can represent the expected indexed raster dimensions, but VADP cannot truthfully emit F65535 register/memory semantics. | Direct code audit. | Re-evaluate only after an F65535 capture contract exists; no Shared frame change is admitted. |
| Font ROM identity | U36 is the 1 MiB IBM 84G7940/OKI MSM538032E DOS/V font ROM, distinct from BIOS flash. | L3 from the service/manual and live discovery record. | App-local immutable firmware binding. |
| Font decode | Live capture records bank select at `1160h`, a 7-bit bank, an 8 KiB window at `DE000h`, 128 banks, and bank-zero `55 AA`/`FONT`. | L3 live-dump observation. | App-local Bowman font-window receiver. |
| Auxiliary font ports | `1161h` scratch, `1162h` segment and `1163h` enable/reset behavior have incomplete independent behavioral proof. | L2 reference only. | Do not infer QEMU's reset/enable shortcuts; audit exact observed behavior before modelling them. |

The primary research anchors are Open-Source-PC110 `Discovery/Live-Dump`
sections 5.3 and its port table, `Discovery/Chipset` section 13e, and
`Discovery/Bowman` sections 6--7. PC110-QEMU is a read-only L2 comparison,
not imported implementation.

## Rejected shortcuts

- Do not configure the existing VADP as a made-up F65535 personality.
- Do not publish a second frame or renderer, normalize the panel in Lib, or
  make Console consume a graphical text surrogate.
- Do not load raw ROMs at runtime, commit their bytes, or derive an original
  DSTN/TFT pair from an unqualified patch.
- Do not treat QEMU's permissive font-enable/reset behavior as board fact.

## Next bounded receivers

1. **Font window:** add an App-local immutable font-ROM binding and the
   directly observed bank/window route. Its tests prove endpoints, bank
   selection, read-only behavior and absence when no firmware image is bound.
   Any unobserved auxiliary-port behavior remains unsupported rather than
   invented.
2. **F65535:** first establish the exact cited register manual, selected VRAM
   capacity, reset/unlock ledger and a standalone X86 NXVM chip contract.
   Then the App may own physical port/memory wiring and panel selection.
3. **Capture/product:** only after (2) can the existing Machine capture path
   be exercised. A Shared-frame change requires a separate demonstrated
   capacity/representation gap and approval.

No guest boot, physical panel validation, display output, embedded ROM or
product executable is claimed by this record.

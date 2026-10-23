# T568 S2 — PC110 Font-ROM Window

## Delivered boundary

`app-mypc110/pc110_font.*` owns one immutable, caller-supplied 1 MiB font
image and only the live-observed CPU path:

- write-only bank selection at `1160h`;
- the low seven bank bits select one of 128 8 KiB banks;
- a replacement memory route exposes that bank at `DE000h`;
- reads copy immutable bytes and writes are accepted without changing them.

The replacement route is necessary because the selected TFT BIOS mapping spans
`C0000h-DFFFFh`. The font aperture is the concrete active device at `DE000h`,
while the lower mapping remains the owner everywhere else. Core continues to
own port dispatch, memory routing and their lifetimes.

The route borrows the `font` member in the final output composition, not a
temporary construction object. This is material: the first implementation
borrowed the temporary member, which made the route dangling after successful
return (x64 failed and x86 faulted). Construction now initializes the output
composition in place, preserving one coherent rollback/teardown owner.

## Explicit omissions

The source records `1161h` as unused/`FF`, `1162h` as segment high and `1163h`
as enabled in one live capture, but it does not yet establish their CPU write,
reset or disabled-window behavior. They are intentionally unregistered.
There is no raw-ROM loader, no external path, no byte commit, no F65535
register implementation, no panel capture and no product executable.

## Verification

`mypc110.unit/font-rom` proves invalid binding rejection, initial bank-zero
data, `82h -> 02h` aliasing, last-byte access, immutable writes, ordinary
non-window routing and absent invented bank-register readback.

`mypc110.unit/base-composition` proves the same path through the actual PC110
composition and confirms that it wins only at the overlapping aperture.

Both stable cached Ninja widths passed all MyPC110 units:

| Width | Tests |
| --- | --- |
| x64 | 4/4 passed |
| x86 | 4/4 passed |

This is mechanism qualification, not a PC110 BIOS, DOS/V, panel or physical
machine validation.

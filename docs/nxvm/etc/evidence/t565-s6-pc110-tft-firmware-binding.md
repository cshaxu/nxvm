# M5 T565 S6 — PC110 TFT firmware binding

## Implemented boundary

`app-mypc110` now owns a construction-only TFT BIOS binding.  It accepts
exactly one non-null 256 KiB byte image and supplies it through the existing
Core firmware-provider contract.  Core copies those bytes into one immutable
mapping at `000C0000h` through `000FFFFFh`; neither the App nor a later guest
run retains a caller-owned firmware pointer.

The mapping is an L2 configuration fact.  `PC110-EMU`'s BIOS inspection names
the low BIOS base as `000C0000h`; `PC110-QEMU` models the full 256 KiB BIOS
window.  This does not assert their later ROM shadow, cache, panel, or reset
behavior is hardware-exact.

## Deliberate exclusions

- The binding does not open an asset path, parse a manifest, embed a ROM, or
  publish an executable.  A later product-build receiver must validate the
  owner-managed `profiles-nxvm/ibm-pc110-tft` manifest before producing an
  artifact.
- The DSTN pairing remains absent; there is no TFT fallback.
- No alias, RAM shadow, reset default, display/font-ROM, storage, input or
  power endpoint is inferred.
- A neutral Core machine plus a firmware mapping is not a PC110 board plan.
  Construction publication remains blocked on a source-qualified composition
  of those missing endpoint owners; an empty X86 NXVM construction is not
  emitted merely to satisfy a lifecycle shape.

## Direct proof

`mypc110.unit/firmware-binding` rejects null and non-exact inputs, binds the
exact TFT image, freezes and resets the normal Core route, then proves only the
complete `C0000h..FFFFFh` mapping exists (not the preceding byte).  It passed
on both established Ninja/ccache graphs:

- `mingw-gcc-x64-release-ccache`: config-route and firmware-binding;
- `mingw-gcc-x86-release-ccache`: config-route and firmware-binding.

This is a construction/mapping proof, not POST, panel, desktop, RDP or guest
boot qualification.

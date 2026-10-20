# T4 S31 — Graphics Image Ingress

## Basis And Scope

The existing owner-supplied inventory records seven immutable graphics roles:
one 128 KiB character image, two 2 MiB background images and four 2 MiB sprite
images. This S admits only their Core creation-time ownership and lifetime.
It neither reads the external archive nor derives a graphics layout, palette
format, decryption key, sprite format, priority rule or frame from it.

The model is a neutral input-lifetime contract, not an L2 emulation claim. The
roles and byte counts come from the existing metadata inventory; their use by a
future decoder remains separately qualified.

## Contract

`myraidenii_core_board_config` now accepts one optional
`myraidenii_core_board_graphics_images` bundle. A null bundle explicitly keeps
the prior headless Core diagnostic configuration. A non-null bundle must name
all seven roles. Board rejects incomplete input before allocation or attachment.

Video is the only storage owner. It copies all fixed-size images to one private
contiguous allocation during Board creation. Board releases it on both creation
rollback and normal destruction. Reset does not erase immutable source images.
No public getter, generic asset registry, host file operation, second cache or
frame path is introduced.

## Verification

- `myraidenii.core/board-bus-smoke` passed in both `gcc-x64-release` and
  `gcc-x86-release`. The existing smoke now checks incomplete-bundle rejection,
  each role's copied lifetime, repeat-load refusal and release, plus successful
  Board creation with a complete synthetic bundle.
- The existing selected-set startup probe passed in both widths, with its
  pre-existing bounded result: `reason=1`, `executed=2000000`, `fault-code=0`.
  It still supplies program images only, proving the former headless path
  remains explicit and no control routing regressed.
- `git diff --check` and MyRaidenII documentation governance passed.

This is not a graphics-decode, presentation, desktop/RDP, sound or playable
Raiden II qualification.

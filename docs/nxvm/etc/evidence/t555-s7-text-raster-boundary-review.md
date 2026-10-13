# T555 S7 Text Pixel Presentation Boundary

## Owner Decision

The owner rejects a Lib text frame containing an optional nested raster.
The approved replacement keeps all six Shared trees unchanged: Core resolves
pixel-exact guest text, and the NXVM Machine adapter publishes it through the
existing KVM graphics frame. Console accepts only genuine character frames.
The owner's instruction to continue admits this NXVM-only implementation in S7.
The earlier optional Shared frame extension was never implemented.

## One Production Boundary

- Chip VRAM/register/scanout remains the only guest video owner. Guest hardware
  can remain in alphanumeric mode while its copied output requires pixels.
- Append `X86_VIDEO_KIND_TEXT_PIXELS` to the existing Core snapshot kind enum.
  Keep existing enum values and use the snapshot's one existing pixel member;
  no nested image, second framebuffer, renderer or persistent discriminator.
- The fixed copied pixel capacity covers 720x512 (nine dots by 80 columns,
  nine-bit vertical extent). Name that storage as pixel capacity, not a default.
  Existing graphics-mode dimensions, text/font layout and source limits remain.
- Core also copies a foreground-only mask (7 or 15; zero preserves the
  existing four-bit default) so the existing Machine cell conversion can
  preserve IBM/CECG font/intensity differences without changing background
  intensity, palette entries or any Shared contract.
- Plain, representable eight-dot whole-cell text retains the original copied
  character/font route. Nine-dot, fine-pan/preset, partial-row and active split
  output uses the one chip pixel sampler, also consumed by diagnostics.
- Machine recognizes that copied output kind, validates the actual source
  capacity before mutation, and converts it to existing `window.graphics=true`.
  Emulator's existing graphics routing therefore selects Window even for a
  Console-configured App. This approved presentation behavior does not modify
  guest mode registers or introduce a second target policy.
- Lib/Emulator/x86 sources, APIs, tests and manifests remain byte-identical to
  their current baseline. No Shared or MyNES change or rebuild is admitted.

## Source And Quality Boundary

Original IBM/CECG sources supply ninth-dot, font-map, address/stride, row-scan,
panning, split and cursor semantics. Exact output is not a physical clock grade.
Unknown Restore/oscillator truths require their own source/model disposition,
not a transport workaround. Core output failure preserves destination and
acknowledgement; Machine validation precedes all destination mutation.

Reuse existing address/font helpers and the pixel array. Keep one sampler for
visible output and diagnostic data, no platform API, host-time input, runtime
manager or App-name branch. Review every automatic snapshot allocation and
measure both widths; increased copied capacity must not silently overflow x86
stack. Ordinary text must not initialize or copy the inactive large pixel tail.

## Files And Estimated Diff

NXVM production: existing `core/chips/video/video.c` and snapshot values,
`core/machine/frame.c`; receiving display/publication changes only when a real
caller requires them. Corresponding existing chip/Board/Machine/App tests,
Core manifests and NXVM evidence own verification.

Estimate complete advanced-text production +120..240/-25..80, existing tests
+160..300/-15..50, excluding independent clock/Restore repairs. No Shared file
is modified. These are estimates, not actual diffs or closure evidence.

## Required Acceptance

1. Actual plane-0/1/2 and original register facts produce both personalities'
   ninth dot, fine scroll, partial rows and split, with exact font/cursor data.
2. Ordinary character delivery remains unchanged; pixel text publishes only
   the existing graphics representation and uses the existing Window route.
3. Direct sampler and diagnostic observations agree without read-count state.
4. Invalid geometry/contents preserve copied output and publication generation;
   first/reset/no-change transitions retain the existing single owner.
5. Source/test manifests, Types/dependencies and documentation checks pass;
   all six Shared trees and owner INI/media/snapshots remain unchanged.
6. Full dual-width units, four-PC integrations and current affected eight PC
   EXEs pass once before complete S7 P delivery. Preserve the exact MyNES pair.
7. Actual-diff review exhausts the full S7 residual ledger, not just this
   representation. T555 remains open for S8 until its own exits are proven.

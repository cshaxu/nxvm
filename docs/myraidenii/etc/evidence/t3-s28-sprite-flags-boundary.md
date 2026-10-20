# T3 S28: Sprite-Flags Receiver Boundary

## Scope

S28 is an evidence and ownership decision only. It changes no production or
test source, public interface, manifest, asset, ROM, or executable.

## Evidence

The existing pinned MAME research reference identifies `0x6ce--0x6cf` as a
write receiver for sprite-protection flags. The two-width selected-set probe
after S27 reaches that address through the existing Board write path. Together,
those facts establish a named write receiver and its local Video/Sprite owner;
they do not establish a complete state layout, bit meaning, reset effect,
decryption transition, readback contract, or visible-frame consequence.

MAME is LGPL research material under the product source policy. Its handler
implementation is not copied, transliterated, linked, or treated as a complete
hardware oracle. The selected-set stop is intentionally retained only as the
neutral address/direction fact already recorded by S27; no protected guest
payload, registers, program counter, instruction bytes, or trace is kept.

## Local Ownership Review

`myraidenii_core_video` owns only the qualified private raster phase. It has no
sprite flags, sprite memory, decryption state, frame state, or public output
contract. `myraidenii_core_board` owns map routing and the one raster-derived
interrupt latch; `myraidenii_core_cop` owns only COP state and existing safe
point delivery. Putting an opaque flags word in Board or COP would create a
second Video state owner and would not make the state observable or correct.

The adjacent map remains deliberately distinct:

- `0x6c0--0x6c7` has prior named sprite-protection configuration receivers;
- `0x6c8--0x6c9` is S27's exact selected write-only no-op span;
- `0x6ca--0x6cb` and `0x6cc--0x6cd` retain their established separate owners;
- `0x6ce--0x6cf` transfers to Video/Sprite, not Board fallback.

## Disposition

No code is admitted. A future Video/Sprite receiver must first establish a
bounded state contract from independent hardware documentation or a finite,
reproducible selected-set observation that distinguishes configuration,
reset/decryption and rendering consequences. It must then keep the flags in
the one Video owner, preserve old accepted state on failure, and test the
actual Board route. A no-op, RAM mapping, source-derived bit algorithm, or
Board/COP mirror is expressly rejected.

S28 therefore closes its research objective and transfers implementation to the
queued Video task. T3 remains open: the selected-set startup has advanced, but
is not qualified and no gameplay, rendering, sound, desktop, RDP or external
integration claim follows.

# T352 S3: Selected Digital CGA Path Reconciliation

## Mechanism Result

The selected CGA mechanism is fully owned by VADP.  Text capture uses the
selected CRTC word start/cursor state and B8000 wrap.  The two selected
graphics decoders use the CGA's distinct interleaved raster layout: even and
odd scanlines are separated by `0x2000`, with 80 bytes per row; `320x200x4`
packs four two-bit pixels per byte and mode `06h` `640x200x2` packs eight
one-bit pixels per byte.  These are real layout differences, not duplicate
implementations to be mechanically merged.

## Writer/Reader/Publication Sweep

| Boundary | Writer/preparation | Reader/publication | Disposition |
| --- | --- | --- | --- |
| Mode selection | VADP `3D8h` callback validates the selected high-resolution form before publishing; QDCGA reset writes mode `03h` through the firmware port capability | VADP dispatch selects text, 320, or 640 snapshot decoder | Selected modes are one VADP state path; unsupported high-resolution forms retain the previous mode. |
| Palette/video enable | `3D9h` updates the one color-select image and dirty state | selected decoder maps RGBl palette; video-disable zeros visible palette entries | No second palette owner or host interpretation. |
| B8000 and raster layout | checked core RAM is the sole video-memory owner; VADP copies the selected aperture before decoding | snapshot pixels use the selected packed/interleaved layout | Existing port tests distinguish first and odd scanline bytes. |
| Text CRTC page/cursor | QDCGA and guest ports write the guarded CRTC bank | text snapshot applies word start, wrap, cursor shape/address/visibility | Retained T266 proof; this is not silently applied to graphics. |
| Dirty/copy/reset | mode/color/memory comparison update VADP capture state; reset clears transient capture state while retaining selected configuration | VM captures then copies the value snapshot to the presentation mailbox | No snapshot or platform consumer aliases VADP/RAM. |

## Direct Proof

| Owner | Required selected behavior |
| --- | --- |
| `core-machine-cga-graphics-port-smoke` | `320x200x4` packed pixels, odd-line `0x2000` selection, palette-select change, mode transition, and text fallback. |
| `core-machine-cga-640-port-smoke` | mode `06h` 640x200 packed pixels, odd-line selection, palette/video state, and rejection without publication of an unsupported high-resolution mode. |
| `core-machine-vadp-text-status-smoke` and `core-machine-vadp-text-smoke` | selected text CRTC page/cursor/start wrap, mode/color readback, capture dirty state, and reset/raster lifecycle. |
| VM CGA graphics/high-resolution system and DOS tests | real profile/ROM port writers reach copied selected frames without a host display mutation path. |
| copied-display/session tests | VM uses a copied snapshot/mailbox boundary; platform consumption does not mutate core VADP state. |

## Non-Merge And Transfer Decision

CRTC display-start scrolling in text is selected and proven.  Extending it to
the graphics raster would require a separately selected IBM CGA address-mode
contract: the 6845 start word, bank/interlace address formation, valid page
range, wrap, and interaction with each graphics mode must be specified as one
matrix.  Current selected digital graphics evidence does not admit that
behavior, so this is not an S3 defect or a safe local `start_byte` patch.

It remains in `TODO(Medium) Complete digital CGA` with other unselected CGA
geometry/CRTC breadth.  Composite/NTSC stays its own TODO.  S4 retains EGA
planar semantics; S5 retains cross-mode reset/copy composition.

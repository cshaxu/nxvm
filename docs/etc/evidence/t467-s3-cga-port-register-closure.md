# T467 S3 IBM CGA Port/Register Closure

## Source-backed repair

Rendered IBM *Technical Reference: Options and Adapters, Volume 2*, PDF p. 58
(manual p. 17) makes the selected 6845 contract finite: 3D4h is write-only
and carries five index bits; R0--R13 are write-only, R14--R15 read/write, and
R16--R17 read-only light-pen address registers. Rendered PDF p. 62 (manual
p. 21) makes 3D8h/3D9h six-bit write-only registers, 3DAh four-bit read-only,
and 3DBh/3DCh address-activated light-pen controls. The same page requires
video disabled while CRTC state is programmed, then re-enabled through the
mode/color route.

`vadp.c` remains the only mutable CRTC/mode/colour/status owner. The repair:

- accepts all selected CRTC indices R0--R17, masks 3D4h to five bits, and
  separates supported, readable and writable access classes;
- makes only R14/R15 readable and prevents guest writes to R16/R17;
- masks both six-bit output-only registers, deleting the obsolete `1Ah`
  single-value high-resolution admission; and
- registers the two IBM light-pen control addresses without creating a false
  light-pen edge, switch or address from a guest write.

The last item is the exact retained L2 boundary: IBM defines the controls but
the selected profile has no physical light-pen input/cancellation lifecycle.
The existing Compaq latch is not reused. A later profile may add that complete
input contract only through VADP; it may not mirror video state or synthesize
an event from 3DCh.

## Proof and minimality

`core-machine-cga-graphics-port-smoke` now covers the write-only CRTC data
class, R14 readback/mask, R16 write rejection/readback, five-bit index masking,
both light-pen control decodes and a valid high-resolution mode combination
previously rejected by the special case. The focused CTest group
`core-machine-(cga-graphics-port|cga-640-port|vadp-text|vadp-text-status)-smoke`
passes 4/4 after a rebuild in `build/mingw-gcc-x64`.

Production paths: one `vadp.c` CRTC route, one mode/colour route and one
copied snapshot route remain. No header, profile, VM, renderer, time source
or second light-pen state was added. The obsolete high-resolution special case
was deleted; the source/test delta is recorded at coordinator review.

# T521 S4 External Video-Asset Closure

`M5:T521:S4:EXTERNAL-VIDEO-ASSET-CLOSURE:OK`

## Finite YAML Matrix

The 22 tracked session YAML files were resolved relative to their own
directories. Every `firmware.font` path resolves, and all select exactly one
asset:

`../../../nxvm-assets/profiles/ibm-mda/firmware/ibm-mda-cp437-character-generator.rom`

| YAML group | Rows | `firmware.video` | Font disposition |
| --- | ---: | --- | --- |
| `default-pc-at` four CPU variants, four floppy formats, and two HDD variants | 18 | explicit `null` | The external MDA generator is the sole Other-L2 raw 8x16 glyph input. It does not claim a source-qualified CGA/EGA adapter configuration. |
| `ibm-5160-model-268-360k` | 1 | explicit `null` | Same sole Other-L2 glyph input; no selected option ROM. |
| `ibm-5170-model-339` 360K and 1.2M | 2 | explicit `null` | Same sole Other-L2 glyph input; no arbitrary C0000h ROM is enabled and no CGA/MDA/EGA firmware ownership is implied. |
| `compaq-deskpro-386-model-40-1200k` | 1 | existing Compaq EGA ROM route | Same sole Other-L2 glyph input. The already distinct Model-40 video-ROM mapping remains outside IBM 5170 configuration selection. |

The one non-null video entry is the pre-existing Model-40 route. No YAML selects
`profiles/ibm-ega/firmware/ibm-ega-6277356-u44-video.rom`; its archive manifest
is an identity record only. Therefore it cannot silently affect any session.

## External Manifest Audit

| Asset role | Manifest profile/path | Bytes | SHA-256 | Disposition |
| --- | --- | ---: | --- | --- |
| Character generator | `ibm-mda` / `firmware/ibm-mda-cp437-character-generator.rom` | 8192 | `37527F661580E5A09710051CEC67422ABFADF31C61B841537A91E7F27BE19304` | Owner-provided, read-only, Other-L2 raw two-plane 8x16 normalization; selected by every YAML row above. |
| IBM EGA U44 option ROM | `ibm-ega` / `firmware/ibm-ega-6277356-u44-video.rom` | 16384 | `BF1583DD387D6E078AB3F5039BCB3F7020A66A63D5F0E57039883C5081DBBF9C` | Owner-provided, read-only, archive-only; no YAML selection and never interpreted as a font. |

The manifests' role, relative path, size, SHA-256, read-only access and
owner-provided provenance match their on-disk files. Neither protected binary
is stored in NXVM Git.

## Owner And Unsupported Dispositions

The retained production direction is:

`YAML external glyph asset -> VM construction-time normalized copy -> VADP sole
glyph state -> copied snapshot/frame -> derived renderer cache`.

VADP is the only runtime glyph owner. The MDA byte layout is Other-L2 evidence,
not a claim that any default-PC/AT, XT, 5170 CGA, or Model-40 EGA board has
thereby gained a Manual-L3 adapter ROM/font configuration. IBM 5170 MDA, CGA,
IBM EGA and later V7 VGA firmware configurations remain unsupported exactly as
List 1/List 2 state, pending their own source-qualified board/firmware admission.

## Verification

- Stripped Release artifact target `vm-0-5-0521` built for both architectures.
  `nxvm_0_5_0521_x64.exe`: SHA-256
  `DBB6171E9EEC31656FA07E8653DD812F247BA3F212B78008467A08AD3A59C6BB`;
  `nxvm_0_5_0521_x86.exe`: SHA-256
  `3FADB0945361B2D4C7774FAD65C422ED97AADC7F4331DC6359EA3FAFC0B8BB3F`.
  PE checks report x64/x86 respectively; both PE debug directories are zero.
- Complete repository-only unit: `304/304` passed.
- Complete external-YAML integration: `44/44` passed, including the two IBM
  5170 boot rows and the Model-40 boot row.
- Production/test/CMake/YAML sweep found no `w32adisp_load_font`, renderer
  `font_glyphs`, dead `VIDEO_INT10` declaration, or deleted CP437 filename.

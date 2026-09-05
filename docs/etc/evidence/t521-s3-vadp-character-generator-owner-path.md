# T521 S3 VADP Character-Generator Owner Path

`M5:T521:S3:VADP-CHARACTER-GENERATOR-OWNER-PATH:OK`

## Selected Assets

The owner authorized two local, owner-provided binaries for the external
`nxvm-assets` archive. They are not copied into this repository.

| Role | Archive path | Bytes | SHA-256 | Evidence level and use |
| --- | --- | ---: | --- | --- |
| IBM MDA character generator | `profiles/ibm-mda/firmware/ibm-mda-cp437-character-generator.rom` | 8192 | `37527F661580E5A09710051CEC67422ABFADF31C61B841537A91E7F27BE19304` | Other-L2 raw-layout source; VM normalizes its two 2-KiB row planes into an 8x16 copied VADP text generator. |
| IBM EGA option ROM U44 6277356 | `profiles/ibm-ega/firmware/ibm-ega-6277356-u44-video.rom` | 16384 | `BF1583DD387D6E078AB3F5039BCB3F7020A66A63D5F0E57039883C5081DBBF9C` | Option-ROM identity only. It is not treated as a character generator or an IBM 5170 configuration selection. |

The MDA raw ordering follows the recorded 86Box parser model: characters 0--255
rows 0--7 occupy the first 2048 bytes; rows 8--15 occupy bytes 2048--4095.
That reference permits an Other-L2 presentation mapping, not a fabricated
adapter-specific L3 claim for EGA or Model 40.

## Sole Production Route

`firmware.font YAML path -> VM file asset -> VM one-time normalized copy ->
VADP copied runtime glyph state -> copied Core snapshot -> copied platform
frame -> Win32 derived GDI cache`

VADP has the only guest/presentation glyph state. The renderer receives no
path, opens no asset and owns no independent source table; its cache is
invalidated from the copied frame when the glyph bytes differ. Reset preserves
the immutable construction state. The old unallocated repository CP437 file
is deleted.

The IBM EGA option ROM remains an external archive record only. Selecting it
in a session or claiming its font semantics needs a separate source-qualified
adapter configuration task.

## Tests

- Core VADP text smoke provides code-defined glyph bytes and verifies they
  survive reset and appear in its copied snapshot.
- Win32 palette smoke provides code-defined copied-frame glyph bytes. It opens
  no YAML or external file.
- Release build: `ninja -C build/mingw-gcc-x64-release -j 8` completed
  `435/435` targets.
- Complete repository-only unit suite: `304/304` passed.
- External-YAML integration: the EGA INT 10h, CMOS-seed, and IBM 5170 1.2-MiB
  boot-matrix rows passed `3/3`; each receives its firmware/font configuration
  through `assets/sessions` and its relative `nxvm-assets` path.
- Documentation governance passed. `git diff --check` passed. A tracked-source
  sweep found no renderer font-file loader, retained presentation `font_path`,
  old `font_glyphs` source table, or reference to the deleted CP437 file.

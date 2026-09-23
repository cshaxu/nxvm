# T521 S2 VADP, Firmware And Font Gap Map (List 2)

`M5:T521:S2:VADP-FIRMWARE-FONT-GAP-MAP:OK`

## Scope And Method

This maps every current product route identified by T521 List 1 to its actual
owner, mutation direction and reachability.  It is a source audit only: no
asset, YAML or production behavior changes in S2.  Queries covered tracked
production source, unit tests, session YAML, CMake registrations and external
asset manifests for `firmware.video`, `firmware.font`, `font_path`, VADP
capture, option-ROM mapping and firmware software-interrupt registration.

The result distinguishes a Core/VADP owner gap from a ROM-admission gap.  It
does not turn the source-unallocated 4-KiB CP437 bitmap into a CGA, MDA or EGA
asset merely to preserve presentation output.

## Current Product Route Map

| ID | Current input and path | Actual owner and mutation direction | List-1 relation / reachability | S3 disposition |
| --- | --- | --- | --- | --- |
| G1 | All 20 checked `assets/sessions/*.yaml` rows name `firmware.font: ../default-cp437-8x16.bin`.  `session_factory.c` copies it to `vm_session_config.font_path`; `session.c` retains the path; `lifecycle.c` passes it to `vm_platform_run_context_create`. | `w32adisp.c` opens the path itself, owns `font_glyphs[256][16]`, and caches host GDI glyph/color bitmaps.  VADP owns text cells/VRAM only; its snapshot contains no character-generator bytes.  Thus the active direction is `YAML file -> renderer private state -> host frame`, not the required VADP route. | This is V6 from List 1: source-unallocated, 4096 bytes, not a CGA/MDA 8-KiB ROS or an IBM EGA generator.  It is reachable for every current product session that opens a Win32 display. | Remove the renderer file loader, renderer guest-font truth and retained presentation-only path.  An admitted character-generator asset must instead be copied into VADP during construction and travel only in the copied display snapshot. |
| G2 | `core_machine_display_config` configures VADP text timing, CGA VRAM, EGA/VGA controllers and ports.  `core_machine_vadp_capture_snapshot` returns cells or indexed pixels; `session/display.c` copies them to `core_platform_display_frame`. | VADP is already the sole guest port/VRAM/mode/frame owner, but lacks character-generator state and does not publish it.  Presentation has a single copied-frame mailbox, but text presentation currently requires G1's second state. | Applies to V2--V5.  It is the correct receiver for a selected adapter's non-programmable character-generator asset; it must not know the adapter name or an asset path. | Extend the construction-only generic display plan with copied, validated character-generator data and describe it in the copied text snapshot.  Keep paths/provenance in VM; VADP owns the copied runtime state.  The renderer may cache only a derivative of the snapshot and must invalidate it when the snapshot font changes. |
| G3 | `firmware.video` parses to `config.video_path`; `vm_session_file_assets_load` reads an optional external byte image.  The generic PC/AT provider maps valid bytes at C0000h. | `vm_session_external_pc_at_rom_provider` copies selected system/option ROM bytes into Core immutable mappings.  It has no software-interrupt provider. | This is a valid generic mechanism for a provenance-admitted EGA/VGA option ROM, but no current default-PC/AT YAML selects one. | Retain as the one generic option-ROM mapping mechanism.  S4 may select it only for a source-qualified EGA/VGA row with a manifest-recorded asset; it is not a font substitute. |
| G4 | 5170 session parsing rejects a non-null `firmware.video`; its two YAML rows explicitly say `video: null`. | 5170 uses the generic external system-ROM provider for its two BIOS chips.  Its fixed CGA VADP topology has B8000h memory and no mapped option ROM. | This exactly preserves the present V3 limitation: CGA hardware is described, but neither a source-qualified CGA character generator nor a source-backed system-BIOS video-service disposition is admitted. | Retain the rejection until an evidence-backed 5170 configuration is selected.  Do not loosen it to accept arbitrary C0000h ROMs or invent an adapter service. |
| G5 | Model 40 requires `firmware.video`; its external provider maps the fixed-size option ROM at C0000h (and the documented compatibility alias). | The Model-40 provider owns immutable ROM registration; VADP remains video state owner.  Its YAML nevertheless still reaches the G1 renderer font path. | This is a distinct Compaq EGA route, not IBM 5170 evidence. | Do not alter its ROM mapping in this task.  S3 must remove its shared renderer-font dependency through the same generic VADP owner path, never by adding a Model-40-only renderer branch. |
| G6 | XT accepts an optional video image in its external-ROM provider, but its current YAML selects `video: null`; the unit BYOB loader currently passes no video image. | XT maps a validated option ROM at C0000h only when explicitly supplied.  It too reaches G1 when presented in Win32. | Outside the 5170 candidate corpus; included by the required global duplicate-route sweep. | Preserve the generic option-ROM mechanism; move only the shared font owner boundary, with no XT special path. |
| G7 | `VM_PROFILE_DEFAULT_PC_AT_FIRMWARE_VIDEO_INT10` appears in Default-PC/AT and 5170 descriptor arrays. | No production caller consumes `firmware_services`; no provider materializes these declarations.  The external PC/AT, XT and Model-40 firmware providers all declare `software_interrupt = NULL`; therefore Core invokes no in-code INT 10h implementation through this field. | It has no source-backed configuration ownership and no product reachability.  Calling it an active in-code video BIOS was inaccurate. | Delete this dead descriptor category/arrays during S3, together with any validation solely keeping it alive.  Do not replace it with a callback or compatibility path. |
| G8 | `test/vm/platform/vm_w32_text_palette_smoke.c` writes a temporary code-defined 4096-byte file solely to exercise the current renderer loader.  Other unit helpers call `vm_session_create_from_assets` with in-memory ROM bytes. | These are repository-only unit fixtures and do not load YAML or external assets.  The W32 smoke mirrors G1's obsolete API shape; the other helpers correctly test composition without product asset I/O. | Not product hardware and not a source-admission path. | Replace the W32 test with a copied-frame glyph test after S3.  Retain in-memory ROM fixtures where they test unit construction; do not turn unit tests into YAML/asset tests. |

## Finite Configuration Matrix

| Matrix row | System/adapter identity | Current asset declaration | Current support disposition |
| --- | --- | --- | --- |
| M1 | IBM 5170 + fixed CGA topology | External paired system ROM and CMOS; no video option ROM; unqualified CP437 renderer bitmap. | **Not admitted as a source-qualified display configuration.**  CGA hardware topology exists, but the required CGA character generator and video-service ownership are absent. |
| M2 | IBM 5170 + MDA | No session/configuration row. | **Unsupported.**  No source-qualified character generator or firmware ownership. |
| M3 | IBM 5170 + IBM EGA | No session/configuration row; 5170 parser rightly rejects an arbitrary option ROM. | **Unsupported.**  No provenance-admitted IBM EGA ROM/font asset and no selected board configuration. |
| M4 | IBM 5170 + later V7/VGA | No session/configuration row. | **Unsupported.**  Later vendor material is not IBM 5170 evidence and is not admitted as an external asset. |
| M5 | Default PC/AT generic EGA-capable topology | External system ROM and CMOS; current YAML video is null; unqualified CP437 renderer bitmap. | **No source-qualified text-font configuration admitted.**  Generic option-ROM mapping exists but is unselected. |
| M6 | Compaq DeskPro 386 Model 40 EGA | External system/video ROM and CMOS; unqualified CP437 renderer bitmap. | **ROM mapping is an existing separate product route; character-generator presentation is not admitted through the required sole VADP path.** |
| M7 | IBM 5160 XT | External system ROM; no selected video ROM; unqualified CP437 renderer bitmap. | **No source-qualified text-font configuration admitted.**  Generic optional-ROM mechanism remains separate. |

## S3 Boundary

S3 has one coherent deletion/replacement batch:

1. remove the session-to-platform `font_path` lifetime, Win32 direct file
   loader, private glyph array and the dead profile `VIDEO_INT10` descriptor;
2. add one construction-only, copied, profile-neutral character-generator
   input to Core display configuration; VADP becomes the unique runtime owner;
3. publish the selected glyph descriptor/data only in the copied VADP text
   snapshot/frame, so every presentation adapter consumes the same frame;
4. replace the renderer-file unit test with code-defined copied-frame data;
   retain no product CP437 fallback; and
5. add no YAML/manifest row until an owner-managed, source-qualified adapter
   asset exists.

This does **not** authorize downloading, copying or registering any vendor
ROM.  The current CP437 file must remain temporarily untouched until S3 has a
valid replacement path; deleting it first would leave the current product
with neither an admitted asset nor a rendered text route.

## Audit Result

The only active duplicate guest-font path is G1.  G7 is dead configuration,
not an executing video service.  External option-ROM registration is already
one Core-owned immutable-ROM mechanism and must be retained rather than
duplicated.  No existing candidate can honestly be marked as a source-qualified
IBM 5170 video configuration until a lawful, manifest-ready adapter asset and
firmware ownership are supplied.

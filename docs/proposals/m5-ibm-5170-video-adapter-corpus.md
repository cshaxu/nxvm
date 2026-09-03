# M5 IBM 5170 Video-Adapter Corpus And External-ROM Admission

## Goal

Establish the evidence-backed video configurations that NXVM may call an IBM
5170, and make each selected configuration load its declared external assets
through the sole session-YAML path.  The outcome distinguishes the original
IBM PC/AT system board from the optional CGA, EGA and later VGA adapters; it
does not call an arbitrary VGA ROM an original 5170 configuration.

## Scope

The task creates two durable ledgers.

1. **List 1 — source configuration corpus.** For original 5170 + MDA/CGA,
   IBM EGA, and a later ISA VGA expansion, record board/adaptor identity,
   ports, aperture, option-ROM expectation, BIOS/INT 10 ownership, font
   source, manual identity and whether the original PDF is text/OCR/ambiguous.
   IBM system-board and adapter manuals are archived only in
   `../nxvm-assets/manuals`; the ledger records their hashes and locators.
2. **List 2 — current product gap map.** Map every List-1 row to VADP,
   firmware mapping, session YAML, manifest and integration-test owners. Each
   row is Manual-L3 only where the manual supplies a usable value/formula;
   source-supported reference behavior is labelled Other-L2; an unsupported
   feature remains explicit rather than receiving a synthetic ROM or font.

The task then selects only configurations supported by both evidence and the
VADP owner path.  Their session YAML explicitly names `firmware.video` when an
adapter option ROM is required and `firmware.font` when the selected adapter
uses a character-generator ROM; both are mapped through the generic external
read-only asset provider and tested with the external-asset integration corpus.
A no-option-ROM CGA/MDA configuration remains an explicit configuration: the
system BIOS and VADP do not silently gain an invented video BIOS.  Its
character-generator ROM is a distinct adapter asset, not an option ROM.

## Existing Path Audit And Required Disposition

Current session YAML supplies `firmware.font`, which the Win32 presentation
adapter reads directly as a fixed CP437 8x16 bitmap.  Separately, the
Default-PC/AT and 5170 profile descriptors register an in-code video INT 10h
firmware hook.  Neither mechanism may remain an unnamed substitute for an
adapter ROM or guest-programmable video font.  In particular, a source-qualified
CGA/MDA character-generator ROM must remain one external reusable adapter
asset: it is not copied into VADP C data, made 5170-specific, or read by the
renderer.

S2 records the exact caller, mutation and presentation route for both shapes,
then assigns every admitted configuration one of these explicit dispositions:

1. a system-BIOS-owned CGA/MDA service, where the source documents that
   ownership;
2. an external option-ROM-owned EGA/VGA service and its real font-loading
   route; or
3. unsupported, with no implicit service or fixed-font claim.

The renderer is never a guest-font owner. It may consume only the copied VADP
presentation snapshot. A supported non-programmable text adapter without a
source-qualified external character-generator asset is unsupported; it does
not receive a host-presentation fallback. S3 removes every duplicate route
identified by S2; it does not paper over it by adding another YAML field,
renderer cache or firmware callback.

## Character-Generator Asset Decision

S3 removes the production glyph table that previously lived in the Win32
renderer. It does not move that table into VADP. A CGA/MDA character-generator
ROM, when source-qualified, is a manifest-recorded external adapter asset and
is selected by session YAML through `firmware.font`; VADP is its sole runtime
consumer and state owner. This role is reusable by every compatible profile,
including but not limited to IBM 5170. Repository-only unit fixtures may use
minimal code-defined glyph bytes, but they never provide a product fallback.
S1 records the source status and the adapter-specific CGA/MDA, EGA and VGA
disposition before any row receives a Manual-L3 label.

The admitted production route has one owner path:

`session YAML external character-generator asset or guest-programmed font
state -> VADP sole state -> copied VADP presentation snapshot -> renderer`.

The renderer-owned glyph array/file loader is removed. VADP consumes the
declared external asset and owns the resulting guest-visible font state; it
does not retain a second built-in production table. The same adapter asset may
be selected by any compatible profile. An EGA/VGA configuration without a
source-qualified option-ROM or character-generator route is unsupported; it
does not silently use a generic VADP seed or claim option-ROM initialization,
programmable-font support, or an IBM adapter-specific L3 state.

## SoftPC Research Boundary

`O:\repos.hobby\softpc\firmware\roms\v7vga.rom` is a V7 VGA ROM, not evidence
of an IBM 5170 original adapter.  S1 may inspect its size, hash, provenance and
licensing record as a read-only research lead. It may become an external,
owner-managed later-VGA input only if the owner confirms its provenance and the
source-policy conditions; it must not be copied into NXVM, imported as source,
or used to claim IBM CGA/EGA behavior.

## Font Rule

An EGA/VGA option ROM may contain glyph data, but ROM presence alone does not
establish a usable VADP font. List 1 identifies whether the selected BIOS loads
its font into guest-visible planes, uses private ROM storage, or requires a
separate character-generator asset; List 2 then verifies the one VADP
snapshot/font-owner route. The renderer must not add a parallel font truth or
assume a fixed ROM offset.

## Planned Subtasks

1. **S1 — manual and firmware provenance corpus.** Acquire/index qualifying
   original IBM 5170, CGA and EGA documentation; audit the available external
   assets and the SoftPC V7 candidate without importing bytes; produce List 1.
2. **S2 — VADP/firmware/font gap audit.** Cross-check List 1 against NXVM
   VADP, generic external option-ROM mapping, the in-code INT 10h hook,
   session grammar and the renderer glyph loader; produce List 2 and one finite
   supported-configuration matrix with an explicit disposition for every one
   of those paths.
3. **S3 — selected configuration implementation.** Repair the sole
   Core/VM/VADP owner path needed by every admitted matrix row, removing any
   duplicate or implicit ROM/font route; make a source-qualified
   character-generator ROM an external, manifest-recorded, reusable adapter
   asset consumed only by VADP; prove that the presentation adapter consumes a
   VADP snapshot rather than a guest-font substitute; add repository-only unit
   tests.
4. **S4 — YAML integration and closure.** Add only source-qualified external
   YAML/manifest rows, run the complete integration corpus and prove the
   selected ROM/font/adapter disposition. Unsupported rows are transferred
   explicitly, not hidden behind `null` or an in-code fallback.

## Non-goals

- Do not distribute, commit, download or fabricate vendor firmware.
- Do not change the existing 5170 system BIOS, CMOS seed or media merely to
  mask a video-adapter issue.
- Do not make Core know an IBM, CGA, EGA, VGA, SoftPC or asset path name.
- Do not treat VGA as an original 1984/85 IBM 5170 configuration.

## Completion Standard

Every candidate video configuration has a source, ownership and support
disposition; every admitted YAML asset is manifest-verified and consumed by the
one external session route; each existing in-code INT 10h and renderer-owned
font route has been removed or assigned one source-backed configuration owner;
selected VADP output and font behavior are proven through their one owner path;
the renderer has no independent guest-font state or glyph asset; an admitted
character-generator ROM is one reusable external adapter asset, never an
embedded VADP production table; full unit and external integration gates pass.

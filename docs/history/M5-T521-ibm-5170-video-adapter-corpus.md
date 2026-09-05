# M5 T521: IBM 5170 Video-Adapter Corpus And External-ROM Admission

T521 establishes the evidence-backed video configurations that NXVM may call
an IBM 5170 and converges each admitted configuration on the sole session-YAML
external-asset to VADP path. The retained proposal defines the bounded S1--S4
plan. No arbitrary VGA ROM is an original IBM 5170 configuration, and no host
renderer owns a second guest font or video state.

## S1 Admission

S1 creates List 1 only: the original-source and local-provenance corpus for
the IBM 5170 system board with MDA/CGA, IBM EGA and later ISA VGA candidates.
It does not import firmware, alter YAML, or change production code. Its
evidence identifies source quality and distinguishes direct manual facts from
external-reference observations before any implementation path is selected.

## S1 Result

The source corpus proves that a 5170 system BIOS Option-ROM scan, a CGA/MDA
character generator, and an EGA adapter BIOS/character generator are separate
hardware roles. The repository-local 4-KiB CP437 bitmap has no source-qualified
adapter identity and is not admitted as any of them. No configuration is
admitted merely from the presence of a local SoftPC or emulator ROM candidate.

The visually checked source corpus is retained in
[T521 S1 List 1](../etc/evidence/t521-s1-ibm5170-video-adapter-source-corpus.md).
Complete repository-only unit passed `304/304` in 19.77 seconds; documentation
governance and `git diff --check` passed. S2 owns the actual VADP, firmware,
YAML and renderer caller audit.

## S2 Result

S2's [List 2](../etc/evidence/t521-s2-vadp-firmware-font-gap-map.md) found
one active duplicate guest-font route: every session YAML names the
source-unallocated CP437 file, which the Win32 renderer opens and retains as
private glyph state.  VADP is already the sole mode/port/VRAM/frame owner, but
does not yet own a character generator or publish one in its copied snapshot.

The profile `VIDEO_INT10` entries are not a live in-code firmware service:
they are unconsumed descriptor data, while all external-ROM providers expose
no software-interrupt callback.  S3 therefore deletes that dead data rather
than assigning it a fictional BIOS owner.  Existing option-ROM registration is
one retained Core immutable-mapping mechanism; current 5170 YAML correctly
rejects an arbitrary option ROM for fixed CGA.  No source-qualified 5170 video
configuration is admitted before an owner-managed adapter asset exists.

The complete repository-only unit suite passed `304/304`; documentation
governance and `git diff --check` passed.  The coordinator actual-diff review
accepted the evidence-only S2 change.  Its deletion/replacement receiver is
source-gated rather than being allowed to invent a font fallback.

## S3 Result

S3 accepts only the owner-approved IBM MDA character-generator ROM as the
external text-glyph input and keeps the IBM EGA U44 option ROM as an
archive-only identity record.  The one live route is now `firmware.font` YAML
asset to a VM construction-time copy, VADP-owned text-glyph state, copied Core
snapshot, copied platform frame, and a derived Win32 GDI cache.  The renderer
opens no file and carries no source glyph table; the old unallocated CP437
repository binary and the dead default-PC/AT `VIDEO_INT10` descriptor are
removed.

The implementation changes 19 production and 6 repository-only test paths:
production source is `+105/-63` lines and tests are `+22/-17` lines.  This is
a cohesive ownership replacement; the 4-KiB binary source table is deleted.
Core/VADP and Win32 tests use in-process bytes, so unit tests remain
independent of YAML and external assets.  Release build completed 435/435 targets, unit passed
304/304, and the three selected external-YAML integration rows passed 3/3.
Documentation governance, `git diff --check`, and the duplicate-route sweep
passed.  The complete source/asset disposition is in
[S3 evidence](../etc/evidence/t521-s3-vadp-character-generator-owner-path.md).

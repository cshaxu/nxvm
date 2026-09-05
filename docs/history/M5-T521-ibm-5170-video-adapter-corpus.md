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

# M5 Profile/CPU/Floppy Integration Matrix

## Purpose

Make the supported DOS floppy-boot matrix an actual external-asset integration
gate rather than a collection of profile-topology unit smokes and ad hoc BYOB
probes.  Each covered row reaches a declared usable terminal: DOS date/input
state, a DOS prompt, or a documented installer entry screen.  An installer
screen is a valid successful boot terminal; it must not be replaced by a
synthetic prompt requirement.

## Frozen Coverage Rule

The matrix is the product of only source- and profile-supported choices:

`frozen profile CPU identity x supported FDD kind/geometry x owner-provided DOS image`.

It does not manufacture every CPU for every machine.  IBM 5160 retains its
8088 and 360K path; IBM 5170 retains its 80286 with 360K/1.2M compatibility;
DeskPro 386 Model 40 retains its frozen 80386DX/1.2M path; and `default-pc-at`
uses each CPU/FDD combination the current profile contract explicitly admits.
S1 freezes the exact rows from the live profile descriptors and media parser
before a test is added.  Unsupported combinations receive an explicit
rejection test, not an attempted boot.

## Boundary And Design

One table-driven `test/integration/` matrix runner owns scenario expansion,
timeouts, terminal recognition and concise row diagnostics.  It invokes the
existing session/profile and BYOB composition route; it must not reimplement a
profile, CPU selector, FDC geometry, BIOS path or DOS parser.  CMake registers
the frozen table as `integration` rows using cache-configured, owner-provided
asset roots.  ROMs, DOS images, machine-local paths, hashes and Microsoft
bytes remain outside the repository and release artifacts.

The runner records the selected profile, CPU identity, declared FDD kind,
logical external input label and reached terminal.  It uses independently
owned per-row scratch locations when persistence is needed, so rows may run in
parallel unless a concrete shared host resource requires serialization.

## S Decomposition

1. **S1 - matrix admission.** Audit current profile CPU/FDD contracts and
   accepted image geometries; freeze every valid row and every expected
   rejection, plus source/provenance boundaries for external firmware/media.
2. **S2 - runner consolidation.** Refactor the current BYOB DOS boot probe
   into one reusable table-driven integration owner without duplicating profile
   construction or terminal detection.
3. **S3 - CTest registration and isolation.** Register every frozen external
   row under the `integration` label, with cache-only asset inputs, independent
   scratch ownership and accurate timeouts; remove superseded ad hoc
   registration if one exists.
4. **S4 - matrix execution and repair.** Run every available row against the
   owner-provided ROM/media corpus, repair any discovered Core/VM/controller
   defect at its sole owner, and rerun the whole matrix.  A missing external
   asset is reported as an unavailable row, never converted to a synthetic
   pass.
5. **S5 - closure.** Audit the registered matrix against S1, run complete
   unit plus integration gates, and build the task's stripped Release artifact
   without modifying `build/output` YAML.

## Exit Criteria

Every supported profile/CPU/FDD/image row is registered and exercised from
external owner-provided assets, each reaches its declared terminal or has a
named unavailable-input record, and every profile/FDD rejection is explicit.
There is one integration scenario owner, no duplicated profile path, no
committed protected material or machine-local path, complete unit and
integration gates pass, and the task builds its stripped Release artifact.

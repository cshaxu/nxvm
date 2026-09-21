# T533 S3 Profile-Owned Machine Plan

S3 replaces Machine's four constructor branches, firmware-kind selector and
parallel resolved-board buffers with one opaque, frozen `vm_profile_machine_plan`.
Profiles construct and own that plan.  It contains the selected Core
configuration, topology, board materialization, media topology, copied BYOB
firmware/CMOS/font data and the firmware provider/context pair.  The plan is
the one lifetime owner of those copied board inputs.

Machine now has one route for both code-owned and file-backed inputs:

`Profile plan create -> Machine prepare -> Core plan -> Common driver`.

It consumes neutral plan getters, binds the supplied firmware provider once,
and keeps only generic runtime/media/display/control ownership.  The only
descriptor projection retained in Machine is read-only diagnostic metadata;
it is never used to select or configure a board.  Profile construction owns
the old default-PC/AT, IBM 5170, XT and Model-40 decisions.  Model-40's D4,
FDC/HDC wiring and topology now reside in `core/profiles/model40/`.

The profile planner is also the sole file-backed asset-size selector, so direct
unit-byte inputs and BYOB file inputs converge before Machine construction.
No external asset was read, added or altered for this S.

## Verification

- Fresh configure and build of `build/t533-s1-x64` pass.
- Focused retained-profile tests pass: default PC/AT, IBM 5170 Model 339, IBM
  5160 XT, DeskPro Model 40, profile memory and atomic-construction coverage.
- The full repository-only unit target passes after the refactor.
- Live `core/machine` source has no `firmware_kind`, `model40_private`,
  `xt_private` or profile-selection branch.  `profile_kind` remains only the
  public request/diagnostic value; Profiles interpret it.
- `git diff --check` and documentation governance are required before P1.

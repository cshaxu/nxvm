# Redistributability

The default ntvdm64 runtime is independently buildable and distributable. It
does not require Microsoft DOS, BIOS media, NTVDM guest files, WineVDM, system
patching, global injection, or registry modification to run its supported DOS
feature set.

- Project-owned files use the root MIT License.
- NXVM-derived code retains applicable LGPL-3.0-or-later notices and release
  obligations; source provenance and `THIRD_PARTY_NOTICES.md` must be updated.
- Microsoft guest mode is BYOB: users supply files from a lawful source into a
  private local directory. Releases neither package nor download those files.
- Invasive integration is excluded from the default build and release.
- Every shipped third-party component requires a license, notice, source
  provenance, and packaging entry.

Before the first release, the build produces a machine-readable dependency and
notice manifest. A release review verifies that protected inputs and local
paths are absent.

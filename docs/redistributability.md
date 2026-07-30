# Redistributability

The default ntvdm64 runtime is independently buildable and distributable. It
does not require Microsoft DOS, BIOS media, NTVDM guest files, WineVDM, system
patching, global injection, or registry modification to run its supported DOS
feature set.

- Project-owned files use the root MIT License.
- NXVM-derived code retains applicable LGPL-3.0-or-later notices and release
  obligations; source provenance and `THIRD_PARTY_NOTICES.md` must be updated.
- Microsoft BYOB is a future M6 research possibility. Releases neither package
  nor download Microsoft files, and no `ntvdm import` workflow is promised.
- Invasive integration is excluded from the default build and release.
- Every shipped third-party component requires a license, notice, source
  provenance, and packaging entry.

Before the first release, the build produces a machine-readable dependency and
notice manifest. A release review verifies that protected inputs and local
paths are absent.

Research documents and tools are excluded from the default installation package.

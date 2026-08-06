# Redistributability

The default products are independently buildable and distributable. `nxvm.exe`
may require owner-provided guest media to boot an external OS image, but the
repository and release must not package protected media. `nxvdm.exe` does not
require Microsoft DOS, BIOS media, NTVDM guest files, WineVDM, system patching,
global injection, or registry modification to run its supported DOS feature set.

- Project-owned files use the root MIT License.
- NXVM-derived code imported under the copyright-holder authorization uses the
  root MIT License; source provenance and `THIRD_PARTY_NOTICES.md` must record
  its origin and copyright attribution. Independently licensed third-party
  material remains subject to separate review.
- Microsoft BYOB is a future M11 T2 research possibility. Releases neither package
  nor download Microsoft files, and no `ntvdm import` workflow is promised.
- Invasive integration is excluded from the default build and release.
- Every shipped third-party component requires a license, notice, source
  provenance, and packaging entry.

Before the first release, the build produces a machine-readable dependency and
notice manifest. A release review verifies that protected inputs and local
paths are absent.

Research documents and tools are excluded from the default installation package.

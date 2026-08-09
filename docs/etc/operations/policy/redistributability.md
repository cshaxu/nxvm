# Redistributability

## Current Status

The repository is not release-ready. `nxvm.exe` is the only current product
candidate; it may require owner-provided guest media to boot an external OS
image, but a repository or release must not package protected media. `nxvdm.exe`
and the `core.dll`, `mantle.dll`, and `dos.dll` targets remain future products
and must not be described as independently buildable or distributable until
their implementation and release records exist.

- Project-owned files use the root MIT License.
- NXVM-derived code imported under the copyright-holder authorization uses the
  root MIT License; source provenance and `THIRD_PARTY_NOTICES.md` must record
  its origin and copyright attribution. Independently licensed third-party
  material remains subject to separate review.
- Microsoft BYOB is a future research possibility. Releases neither package
  nor download Microsoft files, and no `ntvdm import` workflow is promised.
- Invasive integration is excluded from the default build and release.
- Every shipped third-party component requires a license, notice, source
  provenance, and packaging entry.

Before every release, the build produces a completed machine-readable
dependency and notice manifest using
[the release-record template](../release/manifest-template.json). The owner then
completes [the release gate](../release/README.md); that review verifies that
protected inputs and local paths are absent.

Research documents and tools are excluded from the default installation package.

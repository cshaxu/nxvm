# Release Record

This directory contains the release-only evidence required before publishing a
binary or source package. It does not declare that the repository currently has
a release.

## Required Record

For each release, copy `manifest-template.json` to an ignored build/release
workspace, replace every placeholder, and retain the generated manifest with
the release package. The manifest must list:

- product identity, version, source commit, and SHA-256;
- every included component and its license, notice source, provenance, and
  packaging role;
- every required but unbundled external component and its acquisition boundary;
- excluded protected inputs; and
- the notice files delivered with the package.

The source package must contain `LICENSE` and `THIRD_PARTY_NOTICES.md`. A
binary package must contain those files plus its completed manifest.

## Release Gate

The release owner verifies and records all of the following before publication:

1. The product exists in the configured build graph and its output name matches
   the manifest.
2. The build occurs from the declared commit in a clean owned build tree.
3. The manifest SHA-256 matches the packaged executable and every included
   file is listed.
4. No protected guest media, Microsoft binary, third-party firmware, absolute
   path, local trace, developer artifact, or unreviewed third-party material is
   packaged.
5. Every included or required external component has an approved license,
   notice, provenance, and distribution decision.
6. The package contents are compared with the manifest and the release owner
   records a pass/fail verdict with any exception.

`nxvm.exe` is the only current product candidate. `nxvdm.exe`, `core.dll`,
`mantle.dll`, and `dos.dll` require their future implementation and their own
completed manifest before they may be described as release products.

# Asset Policy

Protected guest media is excluded by default. This includes BIOS/ROM dumps,
DOS distributions, disk images, commercial applications, Microsoft guest
components, and third-party executables whose redistribution status is unclear.

- Store local inputs outside the repository or under ignored `local-assets/`.
- A filename is only an operator-local alias; a media identity is its logical
  role, size, SHA-256, and declared use.
- CMake media cache inputs are operator-provided and must not embed an absolute
  local path in tracked source, presets, or documentation.
- A changed hash or guest purpose requires a new approved media record; it does
  not rewrite a historical baseline.
- Commit only logical identifiers, expected SHA-256 values, size, role, and
  provenance note.
- Prefer generated COM/MZ probes and redistributable fixtures in tests.
- Never commit absolute paths, credentials, registry exports, or a captured
  user's application data.
- A release must separately list included and required external components in
  the completed manifest and gate defined by `docs/etc/release/`.
- Any future Microsoft research input remains local and never enters the
  repository. A BYOB/profile proposal requires a separately approved research
  decision.

## M1 Local Fixtures

M1 may read owner-provided disk images and guest probes only to reproduce the
legacy NXVM baseline. They remain read-only, local, unclassified protected
inputs: they are not copied into this repository, build output, test fixture
directory, or release package. The repository records only a logical name, size,
SHA-256, role, and the fact that the owner supplied the file. See
`docs/etc/evidence/fixtures/m1-local-images.md`. Absolute paths and image contents are never
recorded.

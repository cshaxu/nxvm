# Asset Policy

Protected guest media is excluded by default. This includes BIOS/ROM dumps,
DOS distributions, disk images, commercial applications, Microsoft guest
components, and third-party executables whose redistribution status is unclear.

- Store local inputs under ignored `local-assets/`.
- Commit only logical identifiers, expected SHA-256 values, size, role, and
  provenance note.
- Prefer generated COM/MZ probes and redistributable fixtures in tests.
- Never commit absolute paths, credentials, registry exports, or a captured
  user's application data.
- A release must separately list included and required external components.
- Any future Microsoft research input remains local and never enters the
  repository. A BYOB/profile proposal requires an M6 Go decision.

## M1 Local Image Fixtures

M1 may read owner-provided `*.img` files only to reproduce the legacy NXVM
baseline. They remain read-only, local, unclassified protected media: they are
not copied into this repository, build output, test fixture directory, or
release package. The repository records only a logical name, size, SHA-256,
role, and the fact that the owner supplied the file. See
`docs/fixtures/m1-local-images.md`. Absolute paths and image contents are never
recorded.

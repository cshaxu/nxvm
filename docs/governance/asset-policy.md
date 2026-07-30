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
- Microsoft guest-mode components are BYOB local inputs and require version,
  size, and SHA-256 profile validation. They never enter the repository.

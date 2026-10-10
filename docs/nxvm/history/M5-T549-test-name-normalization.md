# M5 T549 — Test-Name Normalization

## Delivered Scope

T549 replaces task-shaped live identities in NXVM test sources, CMake
registrations, static verifiers and test tooling with stable component and
behavior names. Historical evidence filenames and evidence-marker assertions
remain intact as provenance.

## Commits

- `781900494` — inventory and mapping;
- `c1a4d385f` — Shared sweep;
- `969eb41a1` — Core helpers;
- `e6cc846c1`, `616a87aea` — PC App unit identities;
- `b055eeb49` — integration identities;
- `1bd6f2d9a` — static verifier identities;
- this closure commit — final live-output sweep and documentation closure.

## Verification

- x64 repository-only Unit: 515/515 passed.
- x86 repository-only Unit: 515/515 passed.
- x64/x86 specialized static verifier selections passed. The known
  `verify-dependency-dag` allowlist disagreement is unrelated to naming and
  intentionally remains for its own governed receiver.
- The currently registered 8086 decoder-ledger output uses behavior-only
  markers; its x64/x86 receiver and Core manifest both passed.
- The Emulator negative-manifest probe was already corrected and pushed in
  `273ba3296`; it retains the canonical `sha256-manifest-v1` header and is not
  duplicated here.

No production code, public API, assets, firmware/media inputs or deployed
executables changed. No desktop or external-media integration run is claimed.

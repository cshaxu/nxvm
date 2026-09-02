# M5 External Asset-Bundle Governance

## Purpose

Define one owner-governed, external `nxvm-assets` bundle for all inputs that
must remain outside this repository: machine and option ROMs, CMOS/default
configuration records, and integration-test disk images.  The task makes the
asset boundary reproducible and legible without copying, cataloguing for
download, or making protected firmware and guest media a product dependency.

It follows the five-CPU re-audit.  The resulting bundle is consumed only by
explicit BYOB/integration contracts; it does not change Core, VM, profile
selection, test membership, or a release's included files.

## Owner design decision

Before inventory or migration, the owner selects the external bundle root and
approves its retention policy.  The proposed logical shape is deliberately
small:

```
nxvm/assets/sessions/              # copyable YAML templates
nxvm-assets/
  profiles/<profile>/              # ROM, CMOS and profile manifest
  media/                           # FDD/HDD inputs and manifest
  manuals/                         # original manuals only
```

The root itself is owner-managed and is never written into tracked source,
presets, documentation, evidence, or release metadata.  A file name is only
a local alias.  A manifest identifies an input by logical role, size,
SHA-256, intended machine/test purpose, and an owner provenance statement;
it contains no redistributable bytes, credentials, downloaded-source recipe,
or absolute path.  ROM slots describe only the mapping role required by a
machine; they never create a firmware catalogue or default runtime dependency.

## Session YAML boundary

Each copyable session file is the sole request authority.  Its accepted asset
surface is intentionally only:

```yaml
firmware:
  bios:
    - path: ../nxvm-assets/profiles/<profile>/firmware/<rom>
  video: null
  cmos: ../nxvm-assets/profiles/<profile>/cmos/<seed>
  font: default-cp437-8x16.bin
media:
  floppy:
    - path: ../nxvm-assets/media/<image>
  fixed_disk:
    - path: ../nxvm-assets/media/<image>
```

Paths are YAML-relative unless absolute.  Array order is physical attachment
order, never guest drive-letter or partition meaning.  YAML neither discovers
assets nor embeds size/hash validation.  The VM validates only whether the
frozen profile has the declared physical slots; Core owns live devices and
media state.

## Frozen scope

The inventory covers exactly these classes:

- system BIOS and required option/controller ROM logical slots for each
  supported machine/profile;
- CMOS/default board-configuration records needed to reproduce a selected
  firmware/profile observation; and
- external ROM, DOS boot/installer, diagnostic and HDD image inputs used by
  integration scenarios, including their semantic terminal and mutability
  rule.

Repository-owned generated probes and redistributable fixtures remain under
the existing `test/` ownership and are explicitly excluded from this bundle.
No task stage may search for, acquire, copy, redistribute, or commit protected
assets.

## S decomposition

1. **Owner root and policy decision.** Approve the external root, directory
   shape, access/mutability rules, and retirement policy; reconcile them with
   source and asset policy before moving any local input.
2. **Firmware and configuration inventory.** For each selected profile,
   record the required logical ROM slots and CMOS/default-config role, mapping
   purpose, size/hash identity and owner provenance in local manifests; remove
   duplicate aliases or ambiguous ownership from the bundle.
3. **Integration-media inventory.** Classify every external integration input
   by logical test purpose, read-only/copy-on-run rule, expected terminal and
   manifest identity; ensure no unit test or repository fixture depends on it.
4. **One consumption boundary and closure.** Make every supported external
   input resolve through the existing approved BYOB/integration mechanism,
   with one local manifest lookup route and clear missing/mismatch diagnostics.
   Delete any ad-hoc asset path, copied configuration or second lookup path;
   verify unit and owner-provided integration routes without retaining asset
   paths, bytes, hashes or traces in repository evidence.

## Exit criteria

- The owner-approved external `nxvm-assets` shape has exactly one location
  for firmware, configuration, media and manifests, with no ambiguous copy or
  parallel lookup path.
- Every selected profile and integration scenario has a local logical-input
  record or an explicit unavailable disposition; no protected input silently
  becomes a default product dependency.
- ROM slots, CMOS defaults and media identities are distinguishable by role
  and manifest identity, not a hard-coded host path or filename.
- Tests preserve the repository-only unit boundary; external assets are read
  only through the opt-in integration/BYOB route with safe copy-on-run where
  mutability is required.
- Source, asset, documentation, unit and applicable integration gates pass;
  no protected byte, vendor download catalogue, absolute path, credential or
  captured user data is committed.

# M1 T5 S17: Machine Image Materialization

## Scope

S17 establishes the one Machine-owned external-image transaction. Product
maps the twelve named INI roles to the Machine request; Machine resolves every
path, validates the selected-set length and SHA-256 identity, owns a complete
candidate bundle, and supplies the existing Core Board configuration. Core
does not see paths, files, archive names, or identity rules.

This is an asset-identity contract, not a timing, chip, or CPU evidence claim.
The project-wide L3/L2/L1 ladder therefore does not promote these hashes or
sizes to a hardware-model grade.

## Implementation

- `myraidenii_machine_images_load()` accepts only the complete twelve-role
  request and publishes an opaque bundle only after every image validates.
- Any path, open, size, or digest failure destroys the candidate and returns
  no bundle.
- The selected-set probe now uses that production Machine transaction. Its
  prior test-only per-image reader and direct Core configuration path were
  removed.
- The probe tool remains the sole owner of archive staging: it verifies the
  owner-supplied archive metadata, extracts the required files to a unique
  temporary directory, invokes the probe with ordinary file paths, and removes
  the directory. Machine itself has no ZIP reader or archive fallback.

## Verification

| Check | x64 | x86 |
| --- | --- | --- |
| Machine configuration failure-path unit | pass | pass |
| Complete repository-only MyRaidenII unit selection | 11/11 | 11/11 |
| Owner-supplied selected-set startup probe through Machine transaction | pass | pass |
| Documentation governance and whitespace checks | pass | pass |

The external run used the owner-supplied `O:\assets\raidenii\raiden2.zip`
without retaining any ROM bytes in the repository. Both widths reached the
same bounded instruction budget and emitted the same aggregate, non-payload
frame/PCM observation. This proves the selected set passes through the one
Machine materialization boundary; it does not claim product composition,
desktop presentation, audible sound, input, gameplay, or release
qualification.

## Cleanup

The task-owned `build\myraidenii-s17` staging root was empty after both runs
and removed. No protected external input, generated trace, or ROM payload was
committed.

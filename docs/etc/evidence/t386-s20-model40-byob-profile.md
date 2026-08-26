# T386 S20: Model 40 BYOB Backbone Profile

`M5:T386:S20:MODEL40-BYOB-MANIFEST:OK`

`M5:T386:S20:MODEL40-BYOB-VALIDATION:OK`

`M5:T386:S20:MODEL40-PUBLIC-COMPOSITION:OK`

## Scope And Ownership

This S makes exactly one already-selected machine backbone public through the
startup-frozen YAML catalog: `compaq-deskpro-386-model-40`. It is not a generic
clone constructor. VM product owns discovery, parsing, relative-path resolution
and the frozen entry; VM composition owns option application, session
construction and rollback; the Model 40 profile owns its two-chip local loader,
exact-size SHA-256 validation, copied chip lifetime and immutable-ROM mapping.
Core is unchanged: it still receives only the existing immutable mapping
provider and no Compaq name, path, hash, file or profile policy.

The Model 40 entry fixes the accepted 80386/no-FPU/1 MiB/D4/CECG/DMA/HDC
composition and selects the reusable 5.25-inch 1.2 MB FDD device profile. It
accepts no CPU, FPU, memory or free-form hardware override.

## Frozen YAML Contract

```yaml
schema: nxvm-session
profile: compaq-deskpro-386-model-40
display: console
boot: rom
media:
  floppy: null
  hard_disk: null
firmware:
  provenance: owner-supplied local firmware
  rom_even:
    slot: system-rom-even
    path: local-even.bin
    bytes: 16384
    sha256: 64 hexadecimal characters
    map: read-only
  rom_odd:
    slot: system-rom-odd
    path: local-odd.bin
    bytes: 16384
    sha256: 64 hexadecimal characters
    map: read-only
```

The catalog retains no source file after startup changes are made; opening a
selected entry validates the two owner-supplied files before any session is
published. Each must contain exactly 16 KiB and match its manifest digest. The
session copies both validated chips into its owned storage, then the existing
Model 40 provider interleaves/maps immutable bytes. Paths, manifest digests and
provenance are not retained in the session and no real firmware, local path,
vendor digest, catalogue or download mechanism is committed.

## Proof

- `vm-product-session-catalog-smoke` accepts a complete synthetic two-slot
  Model 40 manifest, retains its profile/provenance fields, and rejects an
  otherwise identical malformed digest.
- `vm-model40-console-s20-smoke` drives the retained Console startup chooser through a complete synthetic YAML manifest and confirms the selected session reports the fixed Model-40 profile, 80386 and no FPU.
- `vm-model40-byob-s20-smoke` writes project-owned zero/one 16-KiB synthetic
  chip files, creates the public fixed Model 40 session, proves 80386/no-FPU/
  1 MiB/15-sector FDD selection and copied carrier bytes, mutates the source
  file after creation without changing those live bytes, then proves a digest
  mismatch publishes no session.
- The serial `current-gate` passed **275/275** controls. The specialized gate
  passed with zero VM-to-Core migration edges, 275 registered current targets,
  and T345's updated 222-row ownership matrix (162 owner-test, seven embedded,
  one type, two safe-production, 50 mixed/inherited and 57 residual rows).
- The rebuilt developer artifact is `vm-0-5-0389`,
  `build/output/nxvm_0_5_0389.exe`, SHA-256
  `B0B70FA0C0A304BA9D8E1F2ADECC0E32A547DABA3FE274513B5C9DFEBA8C9483`.

## Transfer

This is not evidence that a vendor ROM starts, that Compaq BIOS services work,
or that firmware has copied bytes into D4 compatibility RAM. It does not close
D4 parity/reset arbitration, remaining CECG behavior, physical fixed-media
semantics, selected-device completeness, board timing or Model-40 L3. Those
remain the T386 functional and board/L3 receivers. Public backbone variants
for additional machines require their own constrained profile contracts.
## P2 Corrective Acceptance Input

P1 was not accepted because its catalog and construction proofs were separate. P2 adds the missing retained Console-to-catalog-to-provider production-route regression; it changes no machine contract or artifact bytes. The focused route, 275/275 serial current gate, specialized gates and documentation gate pass from the corrected graph.

## Coordinator Acceptance

P1 14b3abe was reviewed against the active packet and rejected solely because it lacked a durable Console-to-catalog-to-provider path regression. P2 8f3f8ec adds that exact synthetic regression. The coordinator reviewed P1--P2 actual source, CMake, tests, evidence and owner-provided Queue/Roadmap/proposal changes: Core remains free of Model-40/file policy; VM retains the constrained manifest, copied carrier and fixed backbone; no protected asset is tracked. The final source passes the 275/275 serial current gate, specialized gates and documentation governance. S20 is accepted as bounded functional composition progress, not firmware execution, selected-device completion, board timing or L3 closure.

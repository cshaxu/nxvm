# M5 T30 Final Closure Audit

## Decision

T30 closes. S5 resolved the sole S4 finding, and the final source/test/evidence
review finds no remaining T30-only gap. The next receiver is T31 Mapper 2;
T30's explicit `LIB_STATUS_UNSUPPORTED` disposition ensures it begins from a
safe state rather than a false compatibility claim.

| Exit requirement | Direct evidence | Result |
| --- | --- | --- |
| Finite iNES profile ownership | `core_cartridge_descriptor_create` and `core_cartridge_profile_select` are the only profile selector; media no longer parses mapper behavior. | Pass |
| Payload and suffix bounds | The same Core module publishes the maximum input bound and exact/127/128 suffix normalizer; negative surplus proof exists. | Pass |
| Admitted profile matrix | NROM, MMC1, UxROM, CNROM and MMC3 have finite header shapes and tested dispositions. | Pass |
| MMC3 capacity range | S5 directly proves 2/0, 2/32, 32/0 and 32/32 profile corners. | Pass |
| Deferred runtime correctness | UxROM, CNROM and MMC3 shapes outside the existing M4 receiver return `LIB_STATUS_UNSUPPORTED`; no format-only profile can run. | Pass |
| Media transaction | Storage failures and an admitted UxROM replacement retain the previously accepted machine. | Pass |
| Dual-architecture baseline | S3 recorded x64 112/112 and x86 112/112 full CTest success; S5 rebuilt and passed its changed focused test on both architectures. | Pass |
| Governance and release boundary | Documentation all/self-test gates passed; artifacts are tracked, ROMs ignored, working tree is clean and no remote is configured. | Pass |

T30 changed 316 additions and 86 deletions across its S1--S3 implementation
range, then added the S4--S5 evidence correction. Its delivery artifacts are
the current tracked x64/x86 pair from S3; no artifact changed in the
test-only S5 correction.

# M5 T589 S4: Am386SXLV system management

Coordinator accepted Shared 4d9a248a3 and receiver publications e41d30480,
520fa7370 and 1cf01a0ab. Actual-change review reconciled the separate bus
routes, AMD and retained Intel entry and restore, data-space override,
fault behavior, test metadata follow-up and all fourteen PE/hash checks.
The exact final unit inventories passed 716/716 per host width across
the original run and scoped follow-up. AMD save-image fields and timing
retain the qualification limits in the evidence. No original-ROM
full boot or hardware panel capture occurred in S4. The S4 scratch
directory was cleared after its background jobs exited.

[Implementation evidence](../etc/evidence/t589-s4-am386sxlv-management.md)
includes the display and chipset leads for the following tasks. In order
the x64 aggregate, x86 aggregate, x64 follow-up summary, and x86 follow-up
summary SHA-256 digests were:

- `c22a65f7f8bfa2d60e31435c9cb8316f5e329ac2734b48d9d3df6301d2fe7d5e`
- `986cdc9a0e82d6e8377119ada86d22d4e1a5ec821224472ed58850f3f045f599`
- `80038daae35722477f714057f3c35ba75fde96ad14ac95bf9f891bd1c6fb9e47`
- `594900d373b7ab27be701c972ecb4996dd41fdad710d942841715154942e9fed`

## Archived packet

# M5 T589 S4 - Delivered: Am386SXLV system management

[S3 is closed](../history/M5-T589-S3-sxlv-word-bus.md).
The [S4 firmware handoff](../etc/evidence/t589-s3-am386sxlv-word-bus.md#s4-firmware-handoff)
records the verified RES3 consumer and separates silicon contracts from
HP decode facts that still belong to the board receiver.
The [S4 implementation record](../etc/evidence/t589-s4-am386sxlv-management.md)
records the CPU-to-Machine route proof, 716/716 unit outcomes on both
host widths, and all fourteen verified receiver artifacts.

| Field | Required record |
| --- | --- |
| Identifier Mode | Continuation M5 T589 S4. S3 accepted and closed. |
| Admission And Approval | Owner authorized independent sequential admission and basic hardware implementation in single-session dual-role mode on 2026-11-19. |
| Objective | Implement Am386SXLV SMI entry, distinct management memory, RES3, UMOV and software SMI on the existing CPU/Machine execution path. |
| Non-goals | No HP board port or address decode, ROM boot claim, independent CPU executor, per-product SMM branching, or precise undocumented silicon timing. |
| Reference Baseline | Shared 01d60a30e and receiver baseline 420529dc9; AMD 21020 Rev A and 16305; ROM evidence in the S3 handoff. |
| Candidate Proposal | proposals/m5-ob300-hardware.md S4. |
| Files And ABI Surface | Shared src/test x86-core CPU, Machine and manifests. Specify a minimal distinct memory-space contract at the existing bus/provider boundary. Update two x86-nxvm test fixtures that assert the Core opcode and profile metadata inventory, with their test manifest. NXVM evidence and required NXVM, MyNES and MyRaidenII receiver publication. |
| Applicable Rules | NXVM guide, Contributing, Roadmap, architecture/layout, source policy, and shared Execution/Documentation/Architecture/Coding. |
| Verification | Extend existing CPU SMM and Machine bus fixtures to prove entry priority, 16-bit save cycles, same-address separation, I/O and UMOV routing, guest-edited restore, reset/failure and retained 486 behavior on x86/x64. Apply full Shared receiver build, test and publication requirements before delivery. |
| Expected Markers | One execution and transaction path. CPU owns architectural state and space selection. Board attachment owns backing and decode. Verified spans and modeled fields are distinguished. |
| Asset Needs | Stable Ninja trees, synthetic SMM fixtures and read-only hash-verified System ROM analysis. S4 owns build/t589-s4 up to 64 MiB with deletion after exit. Publication allows 40 minutes; unit aggregates retain 300-second deadlines. |
| Reporting Requirements | Integration boundary, per-field evidence status, delta, test and artifact results, remaining HP board decode obligations. |
| Stop Conditions | Do not publish vendor ROM bytes, silently route AMD management accesses to ordinary RAM, or represent a modeled save-area offset as a primary-source fact. Resolve documented and ROM-evidenced requirements before delivery. |
| Exit Criteria | SMM entry/restore, memory space and intended I/O paths qualified with real-opcode synthetic fixtures, retained profiles pass, required artifacts published, scoped delivery reviewed and pushed. |
| Original Owner Request | Implement MyOB300 in hardware, wiring, machine and product stages. |
| Similar-Issue Sweep | CPU fetch/preview, data, ordinary I/O, reset/fault paths, and management-memory routing across the existing bus contract. |

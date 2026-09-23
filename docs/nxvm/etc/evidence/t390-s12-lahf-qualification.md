# T390 S12: LAHF Qualification

`M5:T357:S3:INSTRUCTION-TIMING-LEDGER:OK`

Intel's [80386 Programmer's Reference Manual](https://datasheets.chipdb.org/Intel/x86/386/manuals/386intel.pdf), `LAHF` table (printed page 324), gives opcode `9F` an exact two-clock row. The shared private 80386 source-timing owner now qualifies only the observed no-prefix form at two Core ticks. Prefix-bearing LAHF remains source-unallocated and physical-contract-rejected; no decoder, public interface, profile or board-time path changes.

The focused timing-ledger smoke proves normal and physical two-tick admission, plus prefixed rejection with no publication. The similar-form sweep retains SAHF as its separate existing three-tick owner; all broader flag-transfer or prefix work remains nonphysical.

A rebuilt owner-managed redacted Model-40 replay advances beyond the former LAHF terminal to its bounded 250,000-successful-retirement budget with no unallocated successful retirement. This does not qualify the full CPU, physical profile, board timing or L3. No external asset identity, path, hash, byte record or trace enters Git.

Full 283/283 current gates, documentation governance and diff hygiene pass. The S12 developer artifact is `vm-0-5-0390`, SHA-256 `9810495E3258E8395050C55452E4DEC20FD361239F2D9F143076DB2CBC7BA227`. Coordinator review remains required before acceptance.

## Coordinator Acceptance

Coordinator actual-diff review confirms one private 80386 source owner, exact Intel two-tick qualification and explicit prefixed rejection. The focused regression independently passes; the full gate, documentation governance and replay evidence meet the packet. S12 is accepted; no L3 claim is made.
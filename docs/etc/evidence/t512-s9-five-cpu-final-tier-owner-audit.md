# T512 S9: five-CPU final tier and owner audit

## Effective result universe

This audit uses the final generated-result contracts, rather than historical
`missing-*` planning snapshots.  All successful rows are conforming and no
CPU instruction result is L1:

| profile | final rows | L3 | L2 | L1 | lower-tier receiver |
| --- | ---: | ---: | ---: | ---: | --- |
| 8086 | 1,053 | 989 | 64 | 0 | Group-3 external model in the sole Core timing selector |
| 8088 | 1,053 derived rows | 989 | 64 | 0 | same Group-3 selector plus Table-2-21 transfer plan |
| 80186 | 616 | 580 | 36 | 0 | sole Core dynamic arithmetic/primary selector midpoint choices |
| 80286 | 771 | 771 | 0 | 0 | none |
| 80386DX | 1,413 | 1,411 | 2 | 0 | Core FPU completion/deadline owner for `ESC` and `WAIT` |

The two 80386 L2 rows are the corrected source facts: `WAIT` is a manual
minimum with external BUSY deassertion, and `ESC` delegates completion to the
coprocessor.  For 8086/8088, the `WAIT` base issue formula remains the
manual CPU row; when a coprocessor is BUSY, the added remaining completion
interval is the shared External-L2 FPU owner established in S8.  Thus no
profile has a hidden one-tick CPU fallback.

## Owner and fallback sweep

The Core selector records `source_timing_unallocated` rather than publishing
its defensive one-tick sentinel.  Every final result contract rejects that
condition.  The remaining scheduler L1-compatibility policy is a separately
admitted controller-progress escape, not a CPU-instruction timing result; it
does not enter any of the five CPU ledgers and this S neither broadens nor
relabels it.

All lower-tier CPU rows retain one Core owner: no VM/profile timing branch,
copied CPU timing state or parallel selector was found.  The same five-profile
sweep found no new L1 instance and no further tier downgrade.

## Acceptance evidence

The five manifest/result runners, their result-contract verifiers, complete
repository-only unit suite and documentation governance gate are the required
proof.  Their final command results are recorded in the S9 closure packet.

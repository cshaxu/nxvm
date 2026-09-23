# T410 S1 CPU External-Cycle Observation

`M5:T410:S1:CPU-EXTERNAL-CYCLE:OK`

Core now publishes an observable candidate external-cycle lifecycle around each
non-preview CPU physical-memory access: begin, existing CPU transaction, then
commit or cancel. It carries the T409 provenance classes, including prefetch,
consumed fetch, data, and page-table read/writeback. The boundary is
observation-only: serial logical accesses remain serial and do not claim an
80386 prefetch overlap, D4 page hit, wait scalar, or Model-L3 result.

Focused transaction, paging, HOLD/HLDA lifecycle and full current-gate pass.
The current developer artifact is `vm-0-5-0410` /
`build/output/nxvm_0_5_0410.exe`, 3,201,306 bytes, SHA-256
`9E3EB86B62AD1CDF6A3CBEE18B9A16FBA7FF9F3E56F63E4FE060AF21B3B9E98B`.
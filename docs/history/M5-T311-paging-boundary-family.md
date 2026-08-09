# M5 T311: Paging Boundary Family

T311 closes the admitted 80386 non-PAE paging boundary while retaining one
core CPU executor, one checked page walk, and existing physical-memory access.
It adds CPL3 U/S and R/W outcomes, CPL0 `CR0.WP`, exact producer-side #PF/CR2
diagnostics, and atomic exactly-one-4-KiB-boundary access publication.

S4 validates both page translations before publishing page-table A/D bits or
guest data. Its private candidate is per access only. S5 found no persistent
translation cache, stale mapping consumer, flush operation, or `INVLPG` entry;
the later-CPU `0F 01 /7` form remains `#UD`.

- Artifact: `build/output/nxvm_0_5_0311.exe`.
- SHA-256: `3A8104DA10F09AC0D632123C2257786D3EC81ADB97CC8B5978C995D7F18A922C`.
- Focused markers: retained T258 paging, `M5:T311:S3:PAGING-PERMISSIONS:OK`,
  and `M5:T311:S4:CROSS-PAGE:OK`.
- Closure verification: 51/51 static and governance targets and 143/143 CTest
  current-gate tests passed. One initial debugger-pause smoke failure passed on
  immediate verbose re-run and the complete subsequent gate run.
- Deferred: exact decoded-length instruction-observation prefetch, #PF IDT
  delivery, PAE/large pages, and TLB/`INVLPG` only after a concrete consumer.

S6 is closure preparation only. T311 remains active pending coordinator
acceptance; this record does not close the task or advance the queue.

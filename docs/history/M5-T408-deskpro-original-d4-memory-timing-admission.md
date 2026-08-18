# M5 T408: DeskPro Original D4 CPU-Memory Timing Admission

## Task Record

T408 examined the first available primary D4 memory timing material for the
1986 DeskPro 386/16 after the tiered generic receiver work.

## S1 Source Boundary And Closure

The Compaq D3PE Processor Descriptions pages 60--63 provide original
initial/row-miss two-wait and row-hit zero-wait CPU memory-read behavior. They
do not provide a safe address-to-DRAM-row decoder in the available material,
and NXVM has no external CPU bus-cycle/pre-fetch boundary. The current logical
CPU-memory transaction path also includes fetch and page-walk reads. Applying
the source scalar there would be false timing.

T408 closes with a documented original-evidence upgrade and an explicit
transfer, not a runtime implementation or an L3 claim. The next productive
work resumes the already owner-approved CPU completeness audit; the physical
D4 receiver remains a named prerequisite for later DeskPro timing acceptance.
# M5 T493 IBM 5160 CGA VADP Phase Contract

T493 is the independent IBM 5160 CGA/VADP unit.  It validates primary sources,
freezes a complete selected function/timing ledger and code-gap ledger, then
implements one owner-local batch at VADP.  Guest video ports, memory mapping,
mode state and copied display snapshots retain one VADP owner; renderer and VM
remain consumers only.

| Subtask | Result |
| --- | --- |
| S1 | Accepted: IBM 5160 and Color/Graphics Monitor Adapter primary-source ledger records visual/OCR quality and selected-CGA authority boundary. |
| S2 | Accepted at `3b497bee`: the complete 33-row List 1 records every selected port, 6845 register, aperture, text/graphics format, status, reset and timing disposition. The owner-approved S4 rule requires immediate complete repair of any in-scope List 2 gap. |
| S3 | Accepted at `3351d2ea`: all 33 rows map once to VADP/Core/VM. The sole S4 batch is generic CGA light-pen latch/status plus binary 640x200 palette; physical timing remains a source-defined boundary, not guessed work. |
| S4 | Accepted at `825dda37`: VADP now owns standard-CGA `3DBh`/`3DCh` latch to `3DAh` bit 1 and the selected high-resolution black/white palette. The focused path proves reset and removes the obsolete high-resolution `3D9h` frame dependency. |
| S5 | Closure audit: all C1--C33 rows reconcile to retained VADP behavior, S4 correction or explicit non-estimated boundary. Serial CTest passes 300/300 and stripped Release 0493 is recorded in the [closure audit](../etc/evidence/t493-s5-cga-closure-audit.md). |

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

# T391 S4: C1 Semantic Observability Inventory

`M5:T391:S4:C1-OBSERVABILITY:OK`

## Existing-state candidate inventory

| Candidate | Existing copied fact | Reset and failure boundary | Disposition |
| --- | --- | --- | --- |
| Second protected-mode entry after C0 | `protected_mode` plus the capture's prior-mode state already define the edge; the synthetic capture proves that state sequence. | Cold reset clears the test capture. The retained two-million-retirement replay did not observe the edge; a budget is containment, not a checkpoint. | Ineligible as the C1 endpoint: observable but absent within containment and not itself a boot-transfer semantic fact. |
| First post-C0 immediate port-61h read | Existing opcode bytes yield immediate I/O direction and port; existing D4 owns port 61h. S2/S3 record its complete finite three-unit predecessor stage. | Test-only capture stops after the read; DX-port I/O deliberately has no resolved port and does not become a substitute. | Already consumed as M40-C0A's endpoint, not a later C1 endpoint. |
| Boot transfer at `0:7C00` | `core_machine_retirement_observation.point.linear_pc` is already a copied execution-point field. A test-local comparison can retain only a boolean reached flag. | Cold reset clears the flag. Fault, unallocated, form-capacity and containment terminals remain distinct. The comparison neither stores PC nor keeps a trace or firmware byte. | Eligible for a separately admitted test-only C1 capture. It is a named transfer endpoint, but no retained replay has reached it. |

## Owner and boundary decision

The retirement observation remains a generic Core-owned copied snapshot; the
Model-40 capture is its existing test-only consumer. Evaluating an existing
linear-PC field to set a local boolean neither expands that observer ABI nor
creates a VM callback, a port-provider route, or a Core/VM bridge. The C1
capture must aggregate only after the C0A boundary and stop on the first
observed `0:7C00` transfer. It must retain only normalized form/context and
its boolean terminal state; it must not print or persist the PC, instruction
bytes, assets, media or raw trace.

This inventory makes a bounded test-only C1-transfer capture admissible. It
does not establish that the endpoint is reached, qualify physical retirement,
or provide board, device, firmware or L3 timing.

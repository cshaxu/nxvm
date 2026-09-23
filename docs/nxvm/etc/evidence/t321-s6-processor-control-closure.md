# M5 T321 S6: Processor-Control Composition And Closure Audit

## Purpose And Scope

This record closes the bounded P0 Queue candidate allocated to T321.  It
does not claim an overall 80386 implementation or a complete processor-control
family.  It reconciles the exact exception, event, return, table, CR0/MSW, and
external-coprocessor CPU-interface work admitted by T321 S1 with the retained
owner evidence, then assigns every remaining breadth to the next ordered Queue
candidate, an existing TODO, or the declared external-coprocessor boundary.

## T321 Crosswalk

| T321 objective | Route and focused evidence | Closure or transfer |
| --- | --- | --- |
| Exact active synchronous producer delivery | `ExecFinal` selects vectors 0, 14, and 16 for `#DE`, `#PF`, and `#MF`; [S2 evidence](t321-s2-exception-delivery.md) and `core-machine-exception-delivery-s2-smoke` prove producer frames and delivery. | Complete only for the named producer masks. Paging policy remains the paging/translation Queue package; x87 execution remains external. |
| External-event ordering and gates | `ExecInt` and `_e_intr_n`; [S3 evidence](t321-s3-hardware-delivery.md) and `core-machine-hardware-delivery-s3-smoke` prove NMI priority over IRQ0 and TF, including VM86 entry. | Complete only for the admitted ordering and frame boundary. NMI devices, reset/shutdown, and broader trap policy are transferred. |
| Software entry and return composition | `_e_int3`, `_e_int_n`, `_e_into`, `_e_iret`; [S4 evidence](t321-s4-interrupt-return-composition.md) and `core-machine-interrupt-return-composition-s4-smoke` prove INT-to-IRET-to-pending-IRQ composition. | Complete only for the shared no-error-code composition. Task, VME/PVI, and broader debug/trap behavior are transferred. |
| VM86 table-load privilege consumer | `INS_0F_01` `/2,/3`; [S5 evidence](t321-s5-vm86-lgdt-lidt.md) and `core-machine-vm86-lgdt-lidt-s5-smoke` prove `#GP(0)` before ModRM/source access. | Complete only for VM86 `LGDT`/`LIDT` rejection. Non-VM86 load behavior remains T319 evidence. |
| Bounded control/table forms | T316 S61--S63 prove `0F 00`, `CLTS`, and `SMSW`/`LMSW`; T318 proves `SGDT`/`SIDT`; T319 proves non-VM86 `LGDT`/`LIDT`. | Their declared form matrices are complete. CR0/CR2/CR3 consumer policy and translation remain the paging/translation Queue package. |
| CPU-side ESC/WAIT interface | T316 S65 proves ESC/WAIT profile and CR0 `MP`/`EM`/`TS` producer boundaries; S2 supplies the consumed `#MF` delivery mapping. | Complete as an external-coprocessor CPU interface only. Numerical x87 implementation is deferred by the x87 TODO. |

## Explicit Transfers

| Remaining breadth | Destination and admission boundary |
| --- | --- |
| Non-PAE translation, page permissions, CR0/CR2/CR3 execution policy, invalidation, and diagnostics | **80386DX paging and translation closure** Queue package; it consumes the delivered `#PF` foundation without reopening its vector mapping. |
| Descriptor, segmentation, gates, and privilege transitions | **80386DX protection and privilege-transfer closure** Queue package. |
| Tasks, LDT breadth, remaining VM86 forms, debug/breakpoint/test registers, and task gates | **80386DX task, local-descriptor, virtual-8086, and debug/test-register closure** Queue package. |
| 80186/80286 LOCK legality | **80186/80286 legacy LOCK-prefix legality matrix** Queue package and its existing TODO; it is a shared prefix policy, not a T321 exception fix. |
| VME/PVI | Existing VME/PVI TODO; it is a post-80386 extension and needs its own virtual-interrupt contract. |
| 80287/80387 and broad x87 execution | Existing broaden-present-x87 TODO; it requires a corpus-driven provider, state, format, exception, and regression matrix. |
| Windows compatibility | Windows 3.x readiness map candidate; it is system validation rather than a CPU closure claim. |

## Artifact And Verification Record

The configured current artifact is `vm-0-5-0321`, which supplies
`PRODUCT_BUILD_VERSION` `0.5.0321` to the bootable `nxvm` product.  The local
developer output is `build/output/nxvm_0_5_0321.exe`.

- SHA-256: `1F3718072E0AAA05AA12510299DF191CB9F56FD7F746DC2C939D7BB3DCEE9374`.
- Runtime identity: `Neko's x86 Virtual Machine`; the ordinary console binary
  has no non-interactive version switch, so the retained product name and
  CMake-supplied build version are the identity record.
- `verify-current-artifact-target` selects only `vm-0-5-0321`; its GCC preset
  names that same target.

The final closure record reruns fresh configuration, documentation governance,
`git diff --check`, and the complete `current-gate` CTest label.  This evidence
is supporting detail only; current task state and Queue order remain owned by
Status and Queue.

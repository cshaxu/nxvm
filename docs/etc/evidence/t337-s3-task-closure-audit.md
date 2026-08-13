# T337 S3: Shared Delivery Task-Closure Audit

## Original proposal-row reconciliation

| Original completion row | Current owner and proof | Closure disposition |
| --- | --- | --- |
| Real/protected/VM86 exception delivery and restart | `ExecFinal` selects synchronous rollback before `_e_except_n`; T326/T331 and T337 S1 prove real-mode vector-6 delivery plus invalid-IVT rollback. T337 S2 retains each mode's distinct serializer/frame boundary. | Closed for common delivery construction; vector-form fault matrices remain with their instruction owners. |
| IRQ/NMI delivery and priority | `ExecInt` -> `_e_intr_n`; `core-machine-hardware-delivery-s3-smoke` and T321 S3 prove NMI, IF-qualified PIC IRQ, then TF priority and the selected frame format. | Closed. |
| 16/32-bit gate and frame selection | Real/protected serializers own architectural width/layout selection; T321/T326 and the T337 S2 mechanism table retain their saved-IP/IF/TF distinctions. | Closed; Intel-required layouts are intentionally not unified. |
| Privilege transitions and TSS stack switching | T259/T320 retained gate, outer-IRET, and TSS-stack evidence; task transition entry is T329 evidence. | Proven common boundary; broader descriptor/task form work remains profile-owned. |
| Fault atomicity and restart | `ExecFinal` rollback precedes synchronous serialization; failed IVT retains diagnostic/rollback; T331 and T337 S1/S2 provide focused proof. | Closed. |
| Instruction-created interrupt inhibition | Existing instruction owners prove MOV/POP SS and STI inhibition; `ExecInt` consumes their common post-instruction state. | Closed as shared composition; opcode behavior remains its form owner. |
| Task debug-trap outcome | T329 S7 proves post-commit incoming-TSS debug trap through `_e_except_n(1)`. | Closed as a distinct post-commit path. |

## Exact transfers and debt

| Residual | Sole owner |
| --- | --- |
| DR6/DR7 matching, breakpoint causes, and ordinary vector-1 interaction | [80386DX extended instruction and system-state closure](../../proposals/m5-80386dx-extended-state-closure.md) |
| Reset, shutdown, and triple-fault policy | `TODO(Medium)` in [states/TODO.md](../../states/TODO.md): architectural reset, shutdown, and triple-fault policy |
| VME/PVI and later debug extensions | 80386DX proposal's explicit outside-80386 boundary |

## Closure checks

- The current production and CMake inventory retain one authoritative real-mode
  `#UD` owner disposition for every current-gate source in scope; T337 S2's
  configure-time classification prevents a stale implicit all-zero-IVT test.
- The T337 artifact identity is `vm-0-5-0337` / `0.5.0337`; its retained
  SHA-256 is recorded in the T337 history and Current baseline.
- No source mechanism, CMake behavior, or test runtime behavior changes in S3.
- The previous complete verification is 217/217 current-gate tests and all 50
  specialized gates in T337 S2. S3 reruns only the artifact, documentation,
  and diff checks required for this documentation closure.

T337 therefore transfers no unclassified shared delivery row to the next
profile package. The Queue may advance to the 8086/80186 profile closure.

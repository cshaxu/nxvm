# T331 S1: Real Exception Final-Delivery Matrix

## Scope

T331 S1 audits the real-mode final-delivery construction in `ExecFinal`. It
does not change the real serializer or invent a producer for an otherwise
unproven exception class.

| Producer class | Vector | Evidence owner | Disposition |
| --- | ---: | --- | --- |
| Divide error (`#DE`) | 0 | `core_machine_inc_dec_smoke` | Retained real delivery. |
| x87 pending fault (`#MF`) | 16 | `core_machine_fpu_interface_s65_smoke` | Retained real delivery. |
| Bound range exceeded (`#BR`) | 5 | `core_machine_bound_s54_smoke` | Retained real delivery. |
| Device not available (`#NM`) | 7 | `fpu_escape_smoke`; `core_machine_fpu_interface_s65_smoke` | Retained real delivery. |
| General protection (`#GP`) | 13 | `core_machine_real_exception_final_s1_smoke` | New success and failed-IVT-delivery proof. |
| Page fault (`#PF`) | 14 | `core_machine_80386_paging_smoke` | Protected-mode evidence only; transfer real-mode producer work. |

## Construction Contract

`_e_final_deliver_real_exception` is the sole private final-delivery plan for
the admitted real branches. It restores the fault snapshot, attempts the
architectural IVT delivery, rolls the snapshot and instruction exception state
back when delivery cannot complete, and records a delivered diagnostic only
after successful transfer. The caller retains vector selection and all
Intel-defined serializer/frame semantics.

The `#GP` branch now uses the same plan before terminal fault recording. The
new owner smoke proves both outcomes: vector 13 enters a real-mode handler
with a restart IP/CS/FLAGS frame, while an IDTR limit that prevents both the
producer and final vector leaves the original terminal `#GP` snapshot intact.

## Similar-Issue Sweep

The production query `rg -n "_e_except_n\\(|_SetExcept_(DE|PF|MF|BR|NM|GP)|ExecFinal" src`
classified the direct final-delivery paths. `#DE`, `#MF`, `#BR`, `#NM`, and
`#GP` use the plan; protected delivery remains in the protected branch;
real-mode `#PF` has no current producer proof and transfers to a later paging
or exception-delivery package. No serializer, memory, descriptor, or stack
helper changed.

## Mechanical Closure

`verify-t331-real-exception-final-construction` checks that `ExecFinal` has
exactly the admitted four helper invocations (`#DE/#PF/#MF` shared branch,
`#BR`, `#NM`, and `#GP`) and no former direct `#BR` or `#NM` final tail.

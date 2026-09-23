# M5 T321 S2: 80386 Exception Vector And Frame Delivery

## Scope And Authority

This record closes only the active `#DE`, `#PF`, and `#MF` producer-to-vector
gap identified by [the T321 S1 audit](t321-s1-processor-control-closure-audit.md).
The Intel 80386 Programmer's Reference Manual exception and interrupt chapters
are the form authority. It does not expand paging policy, x87 execution,
debug/trap/NMI policy, PIC design, task switching, VME/PVI, or reset policy.

## Dispatcher And Producer Sweep

The sweep used `rg -n "_SetExcept_(DE|PF|MF)|VCPUINS_EXCEPT_(DE|PF|MF)" src tests`;
it followed each exact mask through `ExecFinal`, `_e_except_n`, and the
real/protected entry planners.

| Producer class | Exact disposition and evidence |
| --- | --- |
| `#DE` | `ExecFinal` maps the exact mask to vector 0. `core_machine_inc_dec_smoke` installs a real IVT handler for divide, IDIV, and AAM zero. It verifies restart IP, CS, FLAGS, 16-bit frames for default forms, 32-bit frames for `66h`, handler transfer, and preserved producer registers. |
| `#PF` | `ExecFinal` maps the exact mask to vector 14. `core_machine_80386_paging_smoke` installs a protected 32-bit interrupt gate and verifies a not-present read's CR2, error code 0, restart EIP, CS/EFLAGS/error-code frame, handler transfer, and preserved producer GPR state. Its retained no-handler paging cases remain terminal `#PF`; an attempted invalid IDT vector fetch can set the code-page PDE accessed bit, which the retained expectations now record explicitly. |
| `#MF` | `ExecFinal` maps the exact mask to vector 16. `core_machine_fpu_interface_s65_smoke` verifies real WAIT pending-error delivery with a vector-16 frame. `core_machine_fpu_8087_smoke` verifies the unmasked 8087 FWAIT producer's same real vector/frame route while retaining its FPU state assertion. |

`#DE` and `#MF` use no-error-code frames. `#PF` supplies its producer error
code below the saved EIP/CS/EFLAGS frame. The implementation keeps `CR2` from
the producing page walk when restoring the producer snapshot for delivery.

## Implementation Boundary

`src/core/machine/cpu_instructions.c` changes only exact-mask selection and
delivery admission in `ExecFinal`: `#DE -> 0`, `#PF -> 14`, and `#MF -> 16`, including
the required vector-zero boolean distinction. Real-mode delivery is admitted
only for these exact new masks. No page-walk, FPU-provider, memory helper, or
public ABI changed.

The owner target `core-machine-exception-delivery-s2-smoke` composes the
direct retained owner regressions and emits
`M5:T321:S2:EXCEPTION-DELIVERY:OK`. It is registered once in current-gate and
receives target-local GCC/Clang `-Wall -Wextra -Wpedantic -Werror`.

## Verification

- Fresh `mingw-gcc-x64` configuration and focused owner build/run passed.
- The focused INC/DEC, paging, FPU-interface, and 8087 producer regressions
  passed after their equivalent installed-vector assertions were updated.
- `ctest --test-dir build/mingw-gcc-x64 -L current-gate --output-on-failure -j 4`
  passed 199/199 tests.
- Documentation governance and `git diff --check` are required before the
  implementation commit and again for the governance acceptance commit.

## Retained Boundaries

An invalid or unusable delivery remains contained at the original terminal
producer boundary. The separate vector-entry, interrupt composition, trap,
VM86 table-load, and processor-control tasks remain the next T321 slices.

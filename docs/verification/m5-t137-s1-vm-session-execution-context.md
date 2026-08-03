# M5 T137 S1: VM Session Execution Context

`core_product_execution_context` has become
`vm_session_execution_context` and moved from `core/product/runtime` to
`vm/composition`. Its implementation is compiled directly into
`nxvm-vm-composition`; the obsolete `nxvm-runtime` target is gone.

The object remains an embedded member of `vm_session_control_state`, retains
non-owning machine bindings and callbacks, and does not change session,
machine, Console, debugger, or VDM behavior.

The Windows GCC dependency-DAG configure and focused execution-context,
CPU-probe, Console lifecycle, two-session isolation, and FDD DOS-prompt gates
passed. The task artifact is `build/output/nxvm_0_5_0137.exe`
(`64E7A53D3D9F86C0459E81E8B2E107367B76D07BEC918A97A99397E96D608A66`).

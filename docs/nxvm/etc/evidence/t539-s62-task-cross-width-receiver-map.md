# M5 T539 S62 — cross-width TSS receiver map

| Original contexts | Sole receiver | Preserved observations |
| --- | --- | --- |
| 16-bit current TSS to 32-bit target TSS: direct, nested far-CALL, task-gate and nested IRET return | `machine_task_switch_cross_width_smoke.c` | target TSS32 image, current TR/busy descriptors, backlink/NT state, outgoing TSS16 image and return to the TSS16 source |
| 32-bit current TSS to 16-bit target TSS: direct, nested far-CALL, task-gate and nested IRET return | `machine_task_switch_cross_width_smoke.c` | target TSS16 image, 32-bit outgoing image, busy descriptors, backlink/NT state and return to the TSS32 source |

The receiver is the renamed final residual of the former mixed task-switch
test: after the earlier S55-S61 receivers moved their respective contexts, its
`main()` executes only these eight cross-width cases. No duplicate fixture or
second state-image path was added. The two timing manifest runners include the
same receiver source under their private `main` rename, so their existing
timing recipes continue to exercise the identical construction.

This is test/CMake/documentation-only work. No production/API, Shared,
firmware, asset, INI or executable input changes; an EXE rebuild is not
required.

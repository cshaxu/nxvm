# T457 S2: Optimized Debug Artifact and Trace Audit

## Publication route

`current-gcc` and its optional ccache variant now configure isolated
`RelWithDebInfo` build trees.  This is the one route that publishes the current
artifact.  The existing `mingw-gcc-x64` Debug tree remains the current-gate
route, so trace-provider and diagnostic tests retain their ordinary debug
coverage without being able to replace `build/output/nxvm_0_5_0457.exe`.

The current-artifact target has one pre-link guard.  It rejects every build
type except `RelWithDebInfo` before its copy command.  A direct attempt to
build `vm-0-5-0457` from the Debug tree fails with that guard; it does not link
or copy a Debug executable.  The accepted optimized tree records
`CMAKE_BUILD_TYPE:STRING=RelWithDebInfo` (`-O2 -g -DNDEBUG` under the admitted
MinGW route).

The rebuilt artifact is `build/output/nxvm_0_5_0457.exe`, version `0.5.0457`,
SHA-256 `D2351D8940209DBB9BAB82FDA0AB33155223F00EDB7B1C74B59B210C855B5E36`.

## Trace and diagnostic disposition

| Mechanism | Normal product consumer | Disposition |
| --- | --- | --- |
| `TYPE_TRACE_ENABLED` / `TYPE_TRACE_DEBUG` | None; both are `0`; the only guarded uses are debugger assembler/disassembler diagnostics. | Retained, compiled out already. |
| Core machine trace provider and 26 `core_machine_trace_record` sites | No production installation of `core_machine_set_trace_provider`; Core trace smokes are its consumers. | Retained as the debug/test contract. In `RelWithDebInfo`, call sites are compile-time no-ops that do not evaluate their arguments, and `core_machine_trace_*` is absent from the published EXE. |
| Retirement observation provider | No production installation; timing-manifest and observation tests install it. | Retained unchanged. Its pending observation also feeds timing selection before publication, so an attempted no-provider early return was rejected by the timing regressions. |
| CPU fault diagnostic | `vm_session_fault_capture` copies it for product fault reporting. | Retained unchanged. |
| Interactive debugger trace, pause, step, break/watch and recording | The Core debugger commands and VM debug target. | Retained unchanged as a production path. Optimized-target `core-product-debug-target`, `vm-debug-target`, and `vm-debugger-recording-lifecycle` smokes pass; the final EXE exports the corresponding `core_machine_debug_*`, `core_product_debug_*`, and `vm_machine_debug_*` symbols, including `set_trace`. |

Thus the repair removes only unattended internal observation work from the
optimized runtime.  It does not remove a debugger command, instruction trace
requested by a debugger user, a recorder, or fault diagnostics.

The Debug full-gate also exposed one stale S1 test expectation: after F9 the
test still expected a queued guest key event, despite the accepted host-control
behavior being that it returns before key submission.  The test now proves the
opposite without changing the product path.

## Verification

- `cmake --build --preset current-gcc --target vm-0-5-0457` passed and emitted
  the pre-link optimized-artifact check.
- The direct Debug-tree artifact build failed as expected with `Current NXVM
  artifacts may be published to build/output only from RelWithDebInfo.`
- `ctest --test-dir build/mingw-gcc-x64-release --output-on-failure -R
  "(vm-debug-target|core-product-debug-target|vm-debugger-recording-lifecycle)-smoke"`
  passed 3/3.
- The DOS-prompt smoke ran from the optimized tree in 0.062 seconds.  This is
  a focused health check, not a performance benchmark; the earlier 4.61-second
  Debug versus 1.52-second isolated-release baseline remains the comparable
  configuration comparison.
- Symbol inspection of the published executable found no `core_machine_trace_`
  symbol and retained the debugger symbol families named above.

## Minimalism review

The change adds two isolated build presets, one five-line publication guard,
one build-type definition, and two narrow hot-path gates.  It adds no second
artifact target, logger framework, runtime switch, mirrored state, or debugger
path.  The trace-provider API and Debug test route remain one owner and one
path; the optimized route subtracts inactive work at that shared boundary.

# M5 T446 S1 VM Debugger State And Recording Lifecycle

## Owner And Mechanism

`vm_session` retains its own `core_product_debug_context`; the cursor fields
remain context fields, and the T446 static gate rejects a file-static cursor.
The VM machine-debug object is the sole recorder owner.  Its one private close
operation clears `recordFile` regardless of close success, records the outcome,
and is used by replacement start, explicit stop, write failure, and finalization.

The retained console commands and trace text are unchanged.  Start, write, and
stop report at the existing VM debug boundary; `vm_machine_debug_record_status`
also exposes the last lifecycle outcome to its owner without changing the
console-provider command ABI.

## Focused Proof

`vm-two-session-isolation-smoke` now mutates one actual session's debugger
cursor and trace state, then proves its peer remains unchanged.
`vm-debugger-recording-lifecycle-smoke` uses test-local C-runtime substitution
for the one VM debug source. It proves failed open, first write, explicit stop
close, and finalizer close. Every failure clears the active file pointer and
records a non-success status. Its marker is
`M5:T446:S1:VM-DEBUGGER-RECORDING-LIFECYCLE:OK`.

The direct source build is a deliberately narrow T345 embedded-production test
exception: it compiles only `src/vm/machine/debug.c` with three substituted
C-runtime file calls. It adds no production route; its source is listed in the
exact residual ledger alongside the existing equivalent input-failure tests.

The static gate marker
`M5:T446:S1:VM-DEBUGGER-RECORDING-LIFECYCLE-STATIC:OK` verifies the sole close
and write-failure helpers, status boundary, session-owned context, and absence
of file-static cursor state.

## Similar-Issue Sweep

The completed sweep was:

```powershell
rg -n "recordFile|vm_machine_debug_record_(start|stop|status)|vm_machine_debug_finalize|STD_FOPEN\(|STD_FCLOSE\(|STD_FPRINTF\(" src tests CMakeLists.txt cmake docs/states docs/history docs/proposals docs/etc/evidence
```

Production dispositions:

| Hit | Disposition |
| --- | --- |
| `src/vm/machine/debug.[ch]` | Fixed: one recorder stream owner, close route, outcome state, and failure publication. |
| `src/vm/composition/session/control.c` | Sole finalizer caller; it reaches the owner-local close route. |
| `src/vm/composition/session/console_machine_adapter.c` | Sole console start/stop adapter; return values are intentionally consumed at the existing VM debug reporting boundary. |
| `src/core/product/debug/debug.c` | Separate debugger command-file handling; no instruction recorder or shared cursor state. |
| FDD/HDD/media/profile/catalog file calls | Separate media/configuration lifecycles, outside this recorder mechanism and already owned by their respective modules. |

## Verification And Artifact

- Focused smoke, static gate, and artifact build passed on Windows/MinGW.
- `current-fast-smokes-gcc` passed with no failure marker.
- `current-gates-gcc` passed all runnable smoke coverage and 71 specialized
  gates after the T345 ledger and active-packet corrections; its prior failures
  were governance-record omissions, not runtime failures.
- Fresh `build/t446-clean` compiled all 103 target steps from the current source
  and produced `build/output/nxvm_0_5_0446.exe`, SHA-256
  `2265CC4B0182EF66063327FC55118BAA44C95F270B7B0680131E8813EA412CBF`.

## Minimalism Accounting

`git diff --numstat` records 158 added and 21 removed lines across four
tracked source/test paths (`debug.c`, `debug.h`, the focused smoke, and the
existing two-session smoke), for a net +137. The added state is one owner-local
status needed to distinguish successful and failed recording; the focused test
is 76 lines of failure injection and the retained two-session smoke adds seven
direct isolation assertions. No recorder facade, callback, compatibility path,
second stream owner, or command interpreter was added; the former ignored
close/write outcomes were replaced rather than preserved.

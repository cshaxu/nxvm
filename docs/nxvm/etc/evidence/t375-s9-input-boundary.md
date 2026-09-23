# T375 S9: Host And Attached-Device Input Boundary

## Decision

There is no test-only guest-input API.  Input has two production meanings and
therefore two explicitly named boundaries:

| Producer meaning | Production operation | Owner and resulting path |
| --- | --- | --- |
| A host platform reports a key or relative-mouse event. | `vm_session_submit_host_input()` | VM composition copies the event through its `core_platform_input_source` and request transport; the existing VM mapper then emits the selected native device stream. |
| A keyboard or pointing device attached to the emulated machine emits its native stream/report. | `core_machine_keyboard_receive_native_byte(s)` and `core_machine_mouse_receive_relative()` | Core machine passes the event to the owning 8042/KBC mechanism, which retains FIFO, translation, IRQ and guest-port ownership. |

`vm_platform_run_context` now carries only a copied, bounded VM host-input
sink.  Win32 and Linux adapters invoke that sink; they no longer receive a
core input-source object.  This preserves composition as the only host-to-
guest integration point.

## Caller Classification

| Caller class | Disposition |
| --- | --- |
| Win32 and Linux platform adapters | Use the VM host-input sink. |
| End-to-end host-keyboard and host-mouse tests | Use `vm_session_submit_host_input()`; no test reaches `session->input_source` to create a host event. |
| DOS, FDC, display and Windows fixture command streams encoded as Set-2 bytes | Use the renamed core native-device receive operation.  These fixtures model the physical keyboard side of the selected 8042 rather than a host UI action. |
| VDM minimal machine | Uses the same native-device receive operation; it does not recreate a Set-1 or guest-FIFO bypass. |
| Core input-source smoke | Retains the generic platform-source lifecycle contract, not a VM guest-input shortcut. |

The former `submit_native`/`submit_relative` names are absent.  The new names
make it clear that native data is *received from an attached device*, not
injected directly into guest state.

## Focused Proof

The fresh local MinGW build rebuilt the boundary and all affected native-stream
fixtures.  The focused replays passed:

```text
M5:T249:S2:CORE-PLATFORM-INPUT:OK
M5:T226:S2:HOST-INGRESS:OK
M5:T227:S3:KBC-CONTROLLER:OK
M5:T267:S1:AUX:PORT:OK
M5:T229:S3:AUX:GUEST:OK
M5:T366:S5:MODEL339-COMPOSITION:OK
```

`HOST-INGRESS` exercises both the public session operation and the Win32
adapter's platform sink, and confirms that each event remains queued before
the execution-boundary consumer.  The affected DOS/Windows fixture executables
also rebuilt.  They require their separately declared external guest-media
preconditions for execution and are not pass evidence here.

## Retained Boundary

This is an ownership and API-meaning correction only.  It changes no scan-set
mapping, 8042 command, FIFO, IRQ, auxiliary-device protocol, keyboard serial
duration, host pacing, or Model-339 timing claim.  The 8042/keyboard timing
rows remain open in T375.

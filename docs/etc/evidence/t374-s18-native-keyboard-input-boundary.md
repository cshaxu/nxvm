# T374 S18: Native Keyboard Input And 8042 Translation Boundary

## Scope And Owner

This S implements the S17-selected Model-339 keyboard mechanism.  The core
operation `core_machine_keyboard_submit_native_byte(s)` represents a byte
emitted by the attached physical keyboard to the 8042.  It is a production
machine capability, not a guest-FIFO shortcut or a test-only injector.

The VM host adapter accepts the platform's Set-1-style key notification and
emits the selected keyboard's Set-2 make/break stream.  The KBC alone owns the
guest-visible FIFO, command-byte translation, `60h` dequeue, and IRQ1
publication.  The non-runnable VDM skeleton names the same native-byte
capability; it has no Set-1 bypass.

## Functional Boundary

| Native keyboard state | Translation disabled | Translation enabled |
| --- | --- | --- |
| Default/reset | Scan set 2; native bytes are published unchanged. | Command byte controls translation separately; reset does not invent a timing value. |
| `F0h 00h` | Replies with selected set. | Same keyboard-command behavior. |
| `F0h 01h` / `F0h 02h` | Selects Set 1 / Set 2 respectively. | Same selection; translation operates only on Set 2. |
| Set-2 ordinary make/break | Raw Set-2 bytes, including `F0h`. | Set-1 make or high-bit break byte. |
| Set-2 `E0h` extended make/break | Raw Set-2 prefix and bytes. | Set-1 `E0h` plus translated make/break byte. |
| Set-2 Pause | Exact standard eight-byte stream is raw. | Exact stream becomes the six-byte Set-1 Pause stream. |

The lookup covers the 101-key Set-1/Set-2 ordinary and extended keys reached
by the retained host adapter.  Set 3 and arbitrary/nonstandard `E1h` streams
remain unsupported.  AUX/IRQ12, serial transfer duration, command duration,
typematic duration, board timing and L3 are not claimed.

## Similar-Issue And Consumer Sweep

The sweep found every former generic `submit_scan_code(s)` call in core, VM,
VDM and machine tests.  The old interface was removed.  VM product input now
maps host Set 1 into native Set 2 before the named core operation.  Firmware
and DOS-facing paths remain consumers of the KBC's Set-1-visible output when
firmware enables command-byte translation.  DOS fixture command streams that
directly model the physical keyboard were converted to native Set 2.

## Focused Proof

The local MinGW replay, with no ROM or media input, passes:

```text
M5:T227:S3:KBC-CONTROLLER:OK
M5:T374:S18:HOST-SET1-TO-NATIVE-SET2:OK
M5:T353:S2:PCAT-TOPOLOGY:OK
M5:T366:S5:MODEL339-COMPOSITION:OK
```

The KBC proof covers F0 query/selection for sets 1 and 2, native versus
translated normal and extended make/break streams, exact Pause translation,
FIFO order, IRQ1 acknowledgement, reset/defaults and Set-2 typematic break
cancellation.  The DOS keyboard-path targets rebuild with their converted
native fixtures.  They cannot presently be used as pass evidence because the
local invocation lacks their external DOS/ROM media preconditions and stops
before keyboard-path observation.

A full configured build remains blocked by pre-existing, unrelated fixture
interfaces: `vm_cpu_probe` uses an incomplete `vm_session` and stale storage
declarations, while `vm_request_bridge_smoke` uses the removed
`VM_PLATFORM_REQUEST_KEY_PRESS` request shape. The focused native-input, KBC,
Model-339 and VDM targets above build and replay successfully.

No firmware, ROM, media, binary or third-party source was imported.

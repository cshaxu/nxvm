# M5 T229: 8042 AUX Mouse And IRQ12

## Scope

T229 admits a bounded PS/2 AUX mouse through the existing Intel 8042 owner.
It starts with one standard three-byte relative packet, the controller command
path needed to reach the device, and IRQ12 through the existing PIC source
contract. It does not implement a DOS mouse API, BIOS mouse service, GUI
pointer, or direct host-to-guest-memory path.

Reference baseline: T216 PIC source lifecycle, T226 guest keyboard protocol,
and T227 bounded KBC command/timing protocol. Bochs 2.6 keyboard/mouse code is
an observed state-model reference only; no code, global device management, or
GUI coupling is imported.

## S1: Contract And Fixture Design

**Status:** Complete. The reviewed contract fixes the sole KBC owner and the
bounded fixture before the S2 implementation.

### Ownership And Route

```text
host mouse event
  -> VM platform ingress
  -> session command boundary
  -> default-profile relative-mouse mapper
  -> core KBC AUX report ingress
  -> one tagged KBC output sequence / port 60h
  -> PIC IRQ12 (slave IRQ4) / CPU IVT 74h
  -> guest-installed driver or fixture handler
```

- Platform transports normalized host-relative movement and button transitions
  only. It never writes guest RAM, BDA, DOS APIs, KBC fields, or PIC state.
- The default VM profile owns host-coordinate convention, sensitivity, and
  conversion to bounded relative X/Y plus left/right/middle button state. It
  owns no guest queue and cannot create guest packet bytes itself.
- `core/machine/kbc` owns AUX enable/reporting state, device-command state,
  output bytes, current-byte origin, status bits, packet construction, and both
  IRQ source lifecycles. S2 will add one bounded core ingress equivalent to
  `core_machine_kbc_submit_aux_report(kbc, dx, dy, buttons)`; no platform API
  may submit raw guest bytes.
- The default ROM has no mouse consumer in T229. IRQ12 reaches the ordinary
  guest IVT vector `74h`; a guest driver or controlled fixture owns any guest
  memory/BDA/DOS-visible result. `INT 15h` mouse services and `INT 33h` are
  explicitly not admitted.

### Shared Output And IRQ Contract

- KBC has one ordered output sequence whose entries are tagged `controller`,
  `keyboard`, or `aux`. A device response or a complete AUX report is an atomic
  sequence: keyboard bytes cannot appear between its three packet bytes.
- Port `60h` reads exactly one current entry. `OBF` reflects whether an entry is
  present. The status AUX-data bit is set only when that current entry has AUX
  origin; it is not a sticky mouse-present flag.
- KBC owns two PIC source handles: IRQ1 and IRQ12. It asserts only the source
  matching the current output origin, only when the corresponding command-byte
  IRQ-enable bit and interface-enable state permit delivery. It deasserts that
  source after the `60h` acknowledgement, then promotes the next sequence and
  creates its fresh edge through the existing PIC source contract.
- Controller-command responses, including `20h` command-byte reads and `A9h`
  AUX interface tests, have controller origin: AUX-data is clear and they do
  not manufacture keyboard or mouse IRQ delivery. Keyboard-device replies have
  keyboard origin; AUX-device replies have AUX origin.
- IRQ12 is global IRQ12, therefore slave PIC IRQ4 and master cascade IRQ2.
  Neither KBC nor a mouse mapper writes PIC IRR/ISR or CPU interrupt state.
- Controller `A7h` disables new host report admission and deasserts the live
  IRQ12 source; `A8h` re-enables it and may present already queued AUX output.
  A previously latched edge remains PIC-owned until the guest services it and
  sends EOI, so `A7h` never rewrites PIC IRR/ISR. Existing device replies are
  never discarded. `A9h` reports the controller-origin AUX interface-test
  result `00h`. Command-byte bit 1 enables IRQ12 and bit 5 disables AUX.

### Admitted Device Protocol

- Controller command `D4h` marks the next `60h` write as an AUX device command.
  The KBC owns the pending-command state; keyboard command processing remains
  unchanged.
- The admitted standard PS/2 device commands are `F4h` enable stream reporting,
  `F5h` disable reporting, `F6h` defaults, `F2h` identify (`FAh, 00h`), and
  `FFh` reset (`FAh, AAh, 00h`). Acknowledgements and identify/reset results are
  AUX-origin output sequences.
- In enabled stream mode, an admitted mapped movement or button transition
  creates one atomic three-byte relative packet. Byte 1 always sets bit 3;
  bits 0--2 are left/right/middle buttons; bits 4--5 are X/Y signs; bits 6--7
  report saturated X/Y overflow. Bytes 2--3 are signed X/Y deltas. One report
  is bounded to the PS/2 range; this task does not coalesce, split, or retain
  host motion outside the existing bounded session ingress.
- Unsupported in T229: wheel/five-button IDs, sample-rate/resolution/scaling,
  remote mode/read-data/status commands, resend/error timing extensions,
  controller timing calibration, set-2/3 keyboard conversion, BIOS mouse
  services, DOS `INT 33h`, absolute pointing, and host pointer capture.
  Unsupported AUX commands reply `FEh` through the AUX origin rather than
  silently mutating state.

### Required S2 Probes And S3 Fixtures

| Evidence | Required assertion |
| --- | --- |
| `core-machine-kbc-aux-port-smoke` | `20h`/`60h` command-byte IRQ enables, `A7h`/`A8h`/`A9h`, `D4h` device command route, AUX status bit, ordered `FAh`/identify/reset replies, `F4h`/`F5h`, three-byte packet order, and no IRQ1 cross-delivery. |
| IRQ12 lifecycle branch | Slave IRQ4 plus master IRQ2 cascade asserts for an AUX current byte, deasserts on `60h` read, and reasserts only for a promoted successor after correct EOI handling. Keyboard IRQ1 and AUX IRQ12 remain independent. |
| `vm-kbc-aux-guest-smoke` | One normalized host event crosses session ingress and the profile mapper to the KBC. It proves platform/profile never alter guest RAM or BDA and do not retain a host-side guest queue. |
| Controlled guest fixture | A generated or temporary-clone fixture installs an ordinary IVT `74h` handler, enables AUX reporting, reads three `60h` bytes, and records them in fixture-owned guest memory. It proves PIC/CPU/IVT delivery without claiming a DOS mouse driver or `INT 33h`. |
| Retained matrix | KBC keyboard probes, DOS prompt/EDIT.COM, Console/debugger, FDD/HDD boot, two-session, and current GCC/CTest gates remain green. |

### Stop Conditions And S1 Evidence

Stop before S2 if AUX requires a second KBC, mouse FIFO, PIC path, CPU delivery
path, host queue, direct BDA/RAM/DOS mutation, profile-owned guest bytes, or a
default-ROM mouse shortcut. Stop if packet ordering cannot be defined through
one tagged KBC output owner.

S1 evidence is this contract and the design sweep:

```text
rg -n -C 3 "AUX|IRQ12|mouse|KBC" docs src tests CMakeLists.txt
rg -n -C 3 "mouse" D:\home\repos.hobby\bochs-2.6-compat\bochs-2.6\iodev\keyboard.h D:\home\repos.hobby\bochs-2.6-compat\bochs-2.6\iodev\keyboard.cc
```

The sweep found no existing production AUX owner or host-to-DOS shortcut. The
current KBC has one keyboard FIFO and IRQ1 source; S2 is therefore a deliberate
extension of that owner, not an adapter around a second controller. This is a
new capability-design task, so the defect similar-issue sweep is not applicable.

## S2: Owned Implementation

**Status:** Complete.

- `core/machine/kbc` is the sole owner of the tagged controller/keyboard/AUX
  output FIFO, `OBF`/AUX status observation, `D4h` command routing, AUX command
  state, standard packet construction, and IRQ1/IRQ12 source lifecycles.
- VM platform only normalizes relative Win32 input; the default-profile mapper
  applies the host-Y convention and button mask; the existing session ingress
  is the only asynchronous boundary. Its consumer invokes one core-machine
  relative-report ingress at an execution boundary.
- No BIOS service, DOS API, BDA mutation, guest-RAM write, host mouse queue,
  extra KBC, or second IRQ path was introduced. The default ROM remains mouse
  neutral; a guest handler/driver alone consumes vector `74h`.

## S3: Verification And Closure

**Status:** Complete.

| Evidence | Result |
| --- | --- |
| `core-machine-kbc-aux-port-smoke` | Emits `M5:T229:S3:KBC-AUX:PORT:OK`; verifies controller/AUX origin, `20h`/`A7h`/`A8h`/`A9h`/`D4h`, command replies, packet order, IRQ12 cascade, and retained IRQ1 isolation. |
| `vm-kbc-aux-guest-smoke` | Emits `M5:T229:S3:AUX:GUEST:OK`; generated boot media enables reporting with `D4h`/`F4h`, installs ordinary IVT `74h`, consumes the AUX ACK and mapped `(+5,+3,left)` packet through `60h`, and confirms session ingress has no effect before its execution boundary. |
| `verify-aux-mouse-boundary` | Emits `M5:T229:S3:AUX-MOUSE-BOUNDARY:OK`; rejects Win32/profile guest shortcuts and requires platform transport plus session-owned core ingress consumption. |
| `current-gates-gcc` | Passed with 64/64 current CTest smoke cases, including retained DOS prompt, `EDIT.COM`, Console/debugger, keyboard, FDD, and HDD regressions. Linux source contract passed; no WSL was used. |

The developer artifact is `build/output/nxvm_0_5_0229.exe` with SHA-256
`62E7AE972C0D4B433A4842A4756E4D88B36D8F1AEAED2D5F22B883E3297B4ADA`.
Its source implementation commit is `a60b57a`. Its runtime identity is
`Neko's x86 Virtual Machine [0.5.0229]
Copyright (c) 2012-2026 Neko.` The capability admission sweep found no existing
production AUX owner or host-to-DOS shortcut; the new boundary closure gate
guards the recurring forbidden shortcuts. Wheel/advanced AUX protocol and every
guest mouse API remain deferred work, not implied support.

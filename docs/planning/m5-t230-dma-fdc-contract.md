# M5 T230: 8237 DMA And FDC Channel 2 Contract

## Scope

T230 makes the existing dual-8237 storage and scheduler behavior an explicit
core-machine contract, then gives the VM FDC one bounded channel-2 binding. It
does not admit the full 8272A command/result, media, motor, format, non-DMA, or
transfer-timing work; those remain T231.

The purpose is to remove the historical arrangement in which the VM FDC borrows
raw `t_dma` and latch objects and directly raises a numeric DRQ. The core owns
all controller state and guest-memory access. A VM-only device may provide or
consume one byte only through a frozen core channel binding.

## S1: Contract And Probe Design

**Status:** Complete.

### Ownership And Route

```text
guest DMA ports
  -> core 8237 primary/secondary state
  -> core scheduler selects an unmasked request
  -> core reads/writes guest RAM through core memory contract
  -> frozen device byte provider/consumer
  -> terminal-count completion callback

VM FDC command phase
  -> its frozen channel-2 request binding
  -> core DMA request assertion
  -> same core-owned route
```

- `core/machine/dma` owns both controller register files, flip-flops, page,
  base/current address and count, command/mask/mode/request/status state,
  arbitration, terminal count, and all RAM access.
- `core_machine` owns DMA reset/advance/finalize order. VM composition never
  advances DMA and does not retain an alternate transfer loop.
- A device binding is a non-owning, frozen reference to one valid channel. It
  supplies a byte provider/consumer and a terminal-count notification. It may
  assert or deassert its own DREQ only; it cannot inspect or edit DMA registers,
  select another channel, or access guest RAM.
- Default PC/AT describes FDC as IRQ6/DMA2. Composition binds that declaration
  once while core configuration is open. The binding is valid only through
  core-controlled finalization.
- FDC remains the owner of its command state and FDD backend. T230 permits it
  to request one bounded transfer through DMA2; it does not make BIOS or FDC
  directly copy RAM, and it does not claim a complete FDC controller.

### Admitted 8237A Slice

- Primary channel address/count ports `00h`--`07h`, command/status/request/
  mask/mode/flip-flop/master-clear/mask-all ports `08h`--`0Fh`, channel page
  ports including `81h` for DMA2, and the existing secondary-controller port
  family remain core-owned.
- Contracted behavior: low/high flip-flop, base/current reload, byte-width DMA
  channel 2, page + current-address physical addressing, masked-request
  suppression, demand/single/block transfer selection, terminal count/status,
  auto-initialize reload, address increment/decrement, and explicit request
  deassertion. Unsupported controller modes remain observable but must not be
  claimed as device-compatible beyond focused probes.
- DMA moves at most the existing core scheduler cadence; neither FDC nor
  platform wall-clock code may create a second transfer clock.

### Required Evidence

| Evidence | Required assertion |
| --- | --- |
| `core-machine-dma-channel-smoke` | Port-programmed DMA2 transfers a provider byte to guest RAM; validates flip-flop, page/address/count, mask, DREQ assert/deassert, terminal count, auto-init, and no transfer while masked. |
| `vm-fdc-dma-channel-smoke` | Default profile binds exactly FDC -> DMA2; the FDC has a source/provider binding but no raw DMA/latch reference and cannot touch guest RAM directly. |
| Static boundary gate | Reject raw `t_dma`/`t_latch` fields and legacy numeric DRQ calls from `vm/machine/fdc`; require the FDC channel-2 binding. |
| Retained matrix | Current GCC/CTest, FDD/HDD boot, DOS prompt, `EDIT.COM`, Console/debugger, and existing FDC authority probes remain green. |

### Stop Conditions

Stop before S2 if implementation needs a second DMA state copy, VM-side memory
copy, VM-side transfer loop, host timer, direct PIC/CPU mutation, or a hidden
FDC command/result rewrite. If a necessary 8237 behavior exceeds the admitted
slice, record it for T231 or a later DMA extension instead of silently claiming
support.

## S2: Owned Implementation

**Status:** Complete.

- `core/machine/dma` retains the only two-controller register/state storage and
  scheduler advance. It now exports a one-channel provider binding plus
  assert/deassert request handle instead of the legacy raw function-address
  registration and public numeric DRQ operation.
- The secondary-controller `D0h`--`DEh` control-port decode is corrected to
  its interleaved 8237A mapping. DMA2 can therefore traverse the mandatory
  secondary channel-0 cascade only after both controller masks permit it.
- VM composition binds the profile-declared FDC DMA2 route exactly once while
  core configuration is open. `vm/machine/fdc` retains only the resulting
  request binding and byte callbacks; it has no raw DMA/latch field and no
  core-memory access.
- FDC reset/finalize deasserts its request before its own IRQ source is
  released. Existing FDC command/result behavior remains deliberately intact
  for T231 rather than being folded into this ownership task.

## S3: Verification And Closure

**Status:** Complete.

| Evidence | Result |
| --- | --- |
| `core-machine-dma-channel-smoke` | Emits `M5:T230:S3:DMA-CHANNEL:OK`; port-programs DMA2 through the primary and secondary cascade, checks flip-flop/page/address/count, masked and deasserted requests, terminal count, auto-initialize, and address decrement while only core DMA touches RAM. |
| `vm-fdc-authority-smoke` | Emits `M5:T230:S3:FDC-DMA-BINDING:OK`; checks that the default FDC has exactly a DMA2 request binding, its FDD/IRQ/port dependencies, and no raw DMA storage fields. |
| `verify-dma-fdc-boundary` | Emits `M5:T230:S3:DMA-FDC-BOUNDARY:OK`; rejects FDC raw DMA/latch, numeric DRQ, and core-memory access while requiring the composition binding and core-only RAM transfer path. |
| `current-gates-gcc` | Passed with 65/65 current CTest smoke cases, including FDD/HDD boot, DOS prompt, `EDIT.COM`, Console/debugger, keyboard, and display regressions. Linux static contract passed; no WSL was used. |

The developer artifact is `build/output/nxvm_0_5_0230.exe` with SHA-256
`512358DD2053FDA21B3B1A4B5BB6E8A29724E9098CCDAFC340423D99917777D0`.
Its source implementation commit is `c30331e`. Its runtime identity is
`Neko's x86 Virtual Machine [0.5.0230]
Copyright (c) 2012-2026 Neko.` The T230 sweep found that the legacy FDC was the
only production raw-DMA/DRQ consumer; it is now covered by the closure gate.
T231 remains the only task allowed to broaden FDC device behavior.

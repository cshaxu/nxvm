# T386 S8: Model 40 Integrated Selected-Device Replay

`M5:T386:S8:MODEL40-INTEGRATION:OK`

`M5:T386:S8:MODEL40-CONTROLS:OK`

## Delivered Boundary

S8 uses S7's private, VM-only Model 40 composition and a project-owned 128 KiB
synthetic ROM span. It neither creates a public profile, a catalog or Console
route, nor reads a vendor ROM, guest medium, host path, or external binary.
The integration smoke proves the selected 80386/1 MiB/reset-vector mapping,
keyboard-only 8042 behavior, D4 platform state, second PIT/FDC/Compaq HDC/RTC
wiring, Compaq EGA identity state, controller reset, and error/IRQ consumers.

The replay exposed one shared neutral-FDC defect: releasing DOR reset had no
controller-completion IRQ and no reset completion state for Sense Interrupt
Status. The repair is therefore in `core/machine/fdc`, not the Model 40
composition. On the NRS 0-to-1 transition it resets the generic controller,
records the existing four controller slots as reset-completion states, and
asserts the already configured IRQ source when DOR enables it. Sense Interrupt
Status consumes `C0h` through `C3h`, each with cylinder zero, deasserts the
one IRQ source, and then returns the established no-interrupt `80h` response.
A repeated write with NRS already set does not create another completion.
No board-time delay is claimed or added.

## Behavior Cross-Check And Provenance

No external source, firmware, media, text, local path, hash, or binary is
copied or committed. Under the source policy, local reference checkouts were
consulted transiently as behavior cross-checks only:

- the local PCjs FDC reference documents the Model 5170 BIOS reset path's four
  successive Sense Interrupt Status results (`C0h` through `C3h`) and its
  DOR enable-transition interrupt observation;
- the local 86Box FDC reference independently uses the DOR reset-release edge,
  issues an FDC interrupt, and retains four reset statuses for Sense Interrupt
  Status.

Those observations establish the bounded generic queue/IRQ requirement. They
are not imported code or a timing authority. Physical reset delay and media
mechanics remain outside S8.

## Focused Proof

- `core-machine-fdc-topology-port-smoke` proves one DOR reset-release IRQ,
  all four controller-slot reset Sense results, no IRQ after consumption, and
  no duplicate completion for an unchanged enabled DOR write.
- `vm-model40-integration-s8-smoke` uses only the private Model 40 composition
  and synthetic ROM bytes. It repeats the FDC reset/Sense sequence through
  IRQ6/DMA2 wiring, then proves existing keyboard-only KBC/AUX rejection,
  second-PIT, D4/NMI, Compaq HDC IRQ14/error acknowledgement and CECG/RTC
  selected topology checks.
- Existing default-PC/AT and Model-339 current-gate controls remain selected
  by the same source graph; neither receives Model 40 profile policy.

## Similar-Issue Sweep

The defect class is a DOR reset-release transition that omitted a generic
controller completion state. Searches over tracked source, tests, build
registration and T386 evidence use:

```text
rg -n -C 3 "VFDC_DOR_NRS|reset_sense_count|CMD_SENSE_INTERRUPT" src tests CMakeLists.txt docs
rg -n -C 3 "core_machine_fdc_write_dor|core_machine_fdc_reset_controller" src tests
```

The only production owner is `src/core/machine/fdc.c`; all machine profiles
reach it through configured topology. `core_machine_fdc_topology_port_smoke`
now contains the shared regression, and the Model 40 integration smoke proves
its selected composition route. No profile-local duplicate is introduced.

## Transfers

S8 does not claim external-ROM firmware execution, guest-media attachment,
physical 40 MB drive semantics, FDC rotational/index/flux/CRC behavior, CECG
board behavior, ISA waits/arbitration, exact reset duration, DeskPro board L3,
or Windows readiness. Those remain with the established functional/timing
receivers and TODO ledgers.

## Verification

Focused owner and Model 40 integration proofs pass. The serial
ctest --test-dir build/t386-s8 -L current-gate --parallel 1 --output-on-failure
run passes 258/258 in 79.20 seconds. The complete build remains intentionally
blocked by the separately recorded non-current m-request-bridge-smoke API
drift; it is outside S8 and no unrelated repair is folded into this delivery.`nDocumentation governance, VM provider composition, and T345 deferred ownership`nalso pass; the T345 inventory is 145 owner tests, 50 mixed/inherited entries,`nand 57 exact residual production entries.
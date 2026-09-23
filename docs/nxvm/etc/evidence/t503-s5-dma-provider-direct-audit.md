# T503 S5: 8237A To Provider Direct Audit

`T503-S5-DMA-PROVIDER-ROUTE`

## Sources and method

The direct source was visually reviewed from Intel 231466-005, *8237A High
Performance Programmable DMA Controller* (September 1993), printed pp. 2--11.
It is a scanned/OCR PDF; rendered pages govern the DREQ/DACK, EOP, active-cycle,
cascade, transfer-mode and request-register classifications.  IBM 5170
*Technical Reference* (March 1984), System Board pp. 1-8--1-14, was also
rendered: it fixes the two-8237A topology, controller-2 channel 4 cascade,
channel 2 diskette use, and the channel-1 refresh source.  IBM 5160's retained
8237A/Xebec source records remain the XT board basis.

Read-only external comparison inspected local 86Box `src/dma.c`, Bochs
`iodev/dma.cc`, and PCjs `machines/pcx86/modules/v2/chipset.js`.  86Box keeps
hardware request state separate from software request state; Bochs asserts the
master cascade request only after it finds an unmasked slave request; PCjs
documents the same XT/AT channel allocation but leaves several DMA command
forms as TODO.  MAME and QEMU are not present in the approved local reference
tree, so no row relies on either.  These observations corroborate the manual;
they do not become a source dependency.

## Complete selected route ledger

| Form | Direct fact and NXVM route | Result |
| --- | --- | --- |
| Primary/secondary register, mode, mask, request, count and reset forms | Intel defines current/base register pairs, first/last flip-flop, mask/request/status, master clear and selected modes. `dma.c` is the sole paired-controller/port owner; `core-machine-dma-channel` covers primary and secondary port layouts, page addressing, request, mask, mode, auto-init, count, reset and read-to-clear status. | **Manual-L3** for selected logical controller state. |
| Hardware DREQ and selection | Intel requires DREQ until DACK and masks it before priority selection. An immutable binding gives a selected provider the only request capability; `core_machine_dma_request_assert/deassert` updates its controller's request state. `dma_pending_requests()` is the sole mask-aware priority input. | **Manual-L3** logical relation. Raw DREQ polarity is not a second Core input: selected boards use the reset default, while unselected pin-voltage adaptation remains an external board contract. |
| Primary-to-secondary cascade | IBM assigns controller-2 channel 4 as the controller-1 cascade; Intel specifies that cascade is a priority path, not a transfer producer. `core_machine_dma_advance_one()` derives the secondary channel-0 candidate only from the current mask-aware primary candidate. | **Manual-L3.** The repair removes the stale raw-primary mirror from secondary state. A masked primary DRQ therefore cannot preempt a real secondary request. |
| Service and transfer | Intel's SI/S0/S1--S4 (and compressed S1/S2/S4) precede one transfer; read, write and verify have distinct memory/provider effects. `core_machine_dma_advance_transaction()` is the sole machine service entry; `machine_scheduler.c` alone applies the transaction/HOLD boundary. `dma.c` owns phase, provider call, checked memory route and current-address/count publication. | **Manual-L3** state ordering; bus grant publication is the existing Core deterministic contract. |
| EOP, TC, auto-init and provider completion | Intel gives EOP/TC one termination path, sets TC/mask except auto-init, and reloads base values only for auto-init. `dma_complete_transfer()` is that one owner; provider terminal callbacks are bounded completion notification only. `core-machine-dma-channel` covers ordinary and external-EOP completion, auto-init and M2M terminal ownership. | **Manual-L3** logical lifecycle; pin waveform is not claimed. |
| Refresh consumer | IBM 5170 assigns PIT channel 1 to the refresh request; `core_machine_configure_dma()` binds immutable channel 0 refresh and the PIT callback is its sole request producer. The S4 regression proves PIT1-to-DRQ0 edge state. | **Manual-L3** selected logical board relation; DRAM electrical refresh is outside Core. |
| FDC consumer | IBM assigns channel 2 to diskette. Configuration binds the FDC's one DMA provider once; FDC's byte gate asserts/deasserts that opaque request and terminal callback completes its command state. FDC DMA/FDC topology tests exercise normal, reset and media/error paths. | **Manual-L3** selected routing; FDC mechanical service time retains its own source-qualified boundary. |
| XT Xebec consumer | The selected IBM 5160 Xebec topology binds its one HDC provider to DMA3. Xebec alone requests/releases DMA and owns its response/IRQ state; `core-machine-xebec-wiring` covers byte direction, terminal and reset. | **Manual-L3** selected logical route. |
| ATA, Compaq and WD1003 personalities | Their selected HDC configurations are PIO/task-file routes, not DMA providers. They retain no DMA binding, DRQ mirror or terminal callback. | Not applicable to a DMA route; this is an intentional single-path distinction, not an omission. |
| Wait/deadline behaviour | An active DMA request blocks false Core fast-forward and normal arbitration advances the controller. No selected source defines a complete physical CPU/bus conversion for every machine. | **fallback to L2** for Core/board service cadence; no fabricated deadline or host pacing is added. |

## Repair and sweep

`core_machine_dma_set_drq()` previously mirrored every raw primary DREQ into
secondary channel-0 status.  Because channel 0 is highest priority, a masked
primary request could then select the apparent cascade path, find no
mask-eligible primary work, and prevent an unrelated unmasked secondary
channel from running.  That was a second mutable representation of cascade
state and contradicted both the Intel priority rule and the board connection.

The repair deletes the mirror and its later clear.  The existing
mask-aware `dma_pending_requests()` result is now the only cascade input used
by both pending-query and service arbitration.  The regression asserts a
masked primary channel-2 request and an unmasked secondary channel-5 request;
the latter completes its word transfer while the former remains pending and
masked.  No profile shim, polling loop, new interface or duplicate provider
state was added.

The downstream sweep found no additional selected route defect: refresh,
FDC/DMA2 and Xebec/DMA3 retain bounded immutable bindings; ATA, Compaq and
WD1003 are correctly not assigned a DMA route.  DREQ/DACK/HRQ/HLDA electrical
polarity and READY waveforms are not observable Core interfaces and remain
outside the selected logical controller path.

Tracked source/test result: `src/core/machine/dma.c` removes eight stale
mirror-management lines; `test/core/machine/core_machine_dma_channel_smoke.c`
adds 20 owner-local regression lines, for a tracked net of `+12` lines.  The
retained production path is one DMA state pair, one derived cascade candidate,
and one immutable provider binding per selected channel.

Focused command:

```text
ctest --test-dir build/mingw-gcc-x64 -R "(core-machine-(dma|fdc|xebec|hdc|controller-authority)|vm-(fdc-dma-boundary|fdc-port|hdc-port|model40-(fdc|dma|hdc)))" -j 8 --output-on-failure
```

Result: 15/15 passed.  Complete repository-only unit replay:

```text
ctest --test-dir build/mingw-gcc-x64 -L unit -j 8 --output-on-failure
```

Result: 312/312 passed in 13.80 seconds real time.

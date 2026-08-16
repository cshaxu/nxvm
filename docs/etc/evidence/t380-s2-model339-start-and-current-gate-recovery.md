# T380 S2: Model-339 Start And Current-Gate Recovery

`M5:T380:S2:MODEL339-512K-FDC-START:OK`

## Scope And Result

This corrective subtask repairs the user-reachable `ibm-5170-model-339`
start fault and the full-current-gate failures found while auditing that path.
It does not claim that the 5170 boots DOS or that its L3 audit is complete.

The strict 512 KB profile now starts through production session composition.
The final current gate passes all 250 tests, and the specialized governance
gate passes.

## Findings And Repairs

| Finding | Shared owner and repair | Proof |
| --- | --- | --- |
| The FDC firmware shared INT 40h helper used a hard-coded 9FC0h bounce page, outside 512 KB. The BDA also always advertised 639 KB. | The profile descriptor now owns the FDC bounce segment; Model-339 uses 7000h. BIOS BDA base memory is populated from the selected profile CMOS value. Descriptor validation rejects a bounce page outside installed memory. | `vm-fdc-dma-boundary-smoke` proves the generic 9FC0h path and Model-339 7000h path, BDA RAM size, sector load, and the 64 KB DMA boundary. `vm-session-initialization-atomicity-smoke` proves invalid descriptor rejection. |
| Optional address-mark metadata was incorrectly required for all FDC reads, rejecting conventional providers before any byte read. | Core FDC treats a provider without `ADDRESS_MARKS` as conventional Data-mark media, while providers that advertise the capability still query marks. | FDC topology and media-change port smokes pass; DOS FDC read-track and DMA-grant smokes pass. |
| T374 changed the default attached keyboard to Set 2 but left the reset 8042 command byte with translation off. DOS therefore decoded host input as the wrong scan set. | The reset command byte enables 8042 translation for the default Set 2 keyboard. The production VM mapper queries the selected native scan set and emits Set 1 or Set 2 correctly after a guest change. | `vm-dos-keyboard-smoke` starts EDIT successfully. CGA, EGA, ROM INT 10h, mouse, FDC and ATA DOS smokes pass. KBC controller, AUX and VDM presentation smokes prove the default input contract and isolation. |
| CPU preview test compared uninitialized padding in an observation structure. | Test observations are zero-initialized before semantic capture; no CPU behavior changed. | `core-machine-cpu-timing-preview-smoke` passes under the fresh GCC 16.2 build. |
| FDC port tests sent non-DMA bytes and SENSE INTERRUPT without advancing the controller's implemented byte/seek cadence. | Tests now advance to the published controller deadline and retain the same transfer and IRQ assertions. | `core-machine-fdc-topology-port-smoke` and `core-machine-fdc-media-change-port-smoke` pass. |
| With the fixed 512 KB Model-339 profile, DOS 5 `HIMEM.SYS` enabled A20 then probed physical `0x100003`. The generic checked-memory fallback reported `#CE(100003)` instead of exposing an unpopulated board range to the guest. | A profile-selected, 80286-bounded `0x00100000..0x00ffffff` absent-memory provider returns `FFh` and discards writes. It is selected only by Model-339 and does not allocate RAM; the generic profile retains checked-memory faults. | `vm-ibm-5170-model-339-composition-smoke` resets through production composition, enables A20, observes the provider at `0x100003`, verifies `FFh` before and after a discarded write, and reports `M5:T380:S2:MODEL339-NO-XMS-PROBE:OK`. `core-machine-checked-memory-smoke` remains green. An owner-managed external DOS 5 image replay confirmed that HIMEM no longer raised `#CE(100003)` and the test completed successfully. |

## Governance Audit

The audit reviewed every modified production owner and its callers:

- Profile descriptor to firmware materialization, BDA publication and startup
  range validation.
- Generic and Model-339 FDC paths, DMA boundary behavior, optional media
  metadata and timing-aware controller consumers.
- 8042 reset defaults, native scan-set selection, host request transport,
  KBC unit consumers and VDM native-input consumer.
- CPU preview observation comparison and test determinism.
- The Model-339-only absent-memory selection, its 80286 address bound, A20
  routing, provider-capacity accounting, generic checked-memory retention, and
  firmware-ROM/CGA provider coexistence.

No ROM, guest media or asset was imported or changed. No test assertion was
weakened. The output artifact is not evidence of guest OS or Model-339 L3
completion.

## Verification

Fresh MSYS2 UCRT64 GCC 16.2 build directory:

```text
cmake --build build/t380-s2-gcc --target run-current-smokes verify-current-specialized-gates
100% tests passed out of 250
```

The specialized governance targets, including documentation governance,
passed in the same final invocation.

## Developer Artifact

| Artifact | SHA-256 |
| --- | --- |
| `build/output/nxvm_0_5_0380.exe` | `E763F721CA3B3F823049DB4F6422826C6991FB53C079ED0A50E86B205D746512` |

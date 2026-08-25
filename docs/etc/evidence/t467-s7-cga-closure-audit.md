# T467 S7 IBM CGA Closure Audit

## Ledger Reconciliation

All seventeen frozen Checklist 1/Checklist 2 identifiers now have one final
disposition. `vadp.c` is the only mutable port, raster, palette, aperture and
snapshot owner; Core memory is the only B8000h router; profile/firmware emits
ordinary guest operations; VM presentation receives copied snapshots only.

| Rows | Final disposition |
| --- | --- |
| R1--R4 | Manual-L3 CRTC/mode/colour/status grammar is implemented and tested through the one VADP route. Exact elapsed phase follows T1/T2 rather than a port-local clock. |
| R5 | Manual-L3 control-address grammar is implemented as VADP-local no-op control addresses. The absent external pen input, cancellation and consumer are T3 L2, not borrowed Compaq state. |
| F1--F3, F5--F7 | Manual-L3 text, 320/640 interleaved B8000h, CRTC relation, mode-change/output-disable and palette effects are implemented through the sole copied-snapshot path. |
| F4 | 160x100 exists in the IBM source but lacks a complete programming tuple; it remains exact L2 with no invented snapshot kind. |
| T1--T2 | IBM logical raster/status relations are Manual L3. The copied construction receiver is Board-L3-capable; Model-339's qualified 86Box cadence is bounded Other L3. Unqualified default timing data and the ISA wait formula remain L2. |
| T3--T4 | Light-pen lifecycle and physical/composite/monitor/host-presentation behavior lack complete source-qualified input/consumer contracts and remain exact L2. |
| T5 | The architecture boundary is accepted: guest/firmware ports and memory -> VADP sole state -> copied snapshot -> VM presentation. |

## Similar-Issue Sweep

The tracked-source sweep used `rg` over `src/core/machine`,
`src/vm/profile/default_profile`, `src/vm`, `tests`, `CMakeLists.txt`,
`CMakePresets.json` and T467 records for `3D4`, `3D5`, `3D8`, `3D9`, `3DA`,
`B8000`, `cga`, `snapshot`, `0466` and `vm-0-5-0466`. It finds the retained
VADP callbacks, one B8000h provider, QDCGA/BIOS port writers, one copied
snapshot consumer, focused/system fixtures and the single current-artifact
declaration. EGA/CECG matches are separate VADP personalities, not CGA state
owners. No duplicate CGA mode, VRAM, palette, raster or frame state, stale
artifact target, forwarding wrapper or compatibility path remains.

## Verification And Artifact

After fresh Debug and Release configuration, the serial current gate completed
295/295 with zero failures. The focused Model-339 topology plus 320, 640 and
DOS CGA system group also passes 4/4. Documentation governance passes.

`cmake --build --preset current-gcc` built the sole optimized Release target
`vm-0-5-0467` and copied
`build/output/nxvm_0_5_0467.exe`. Its embedded version string is `0.5.0467`,
its SHA-256 is
`CD99B2E1A0F0AC0FB8982838BEC56237C29C1DA24BD9E5E3B608117B20EAF6C1`, and
`objdump -h` finds no `.debug` sections. The production change is only the
four-line current-artifact identity/preset cutover; all CGA behavior was
already supplied by S3/S4.

T467 therefore closes the selected IBM-CGA digital contract. The exact L2
transfers are F4 160x100 programming, T2 ISA wait/default timing data, T3
light-pen lifecycle and T4 physical/composite/monitor/presentation behavior.

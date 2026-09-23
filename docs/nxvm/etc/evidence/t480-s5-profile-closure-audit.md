# T480 S5 VGA profile-boundary and closure audit

## Profile/card sweep

The sweep used `rg -n -i -C 3 "ega_present|display_present|display.*config|configure_display|configure_vga|vga" src/vm/profile src/vm/composition src/core/machine/machine_display.c src/core/machine/vadp.c`.

| Surface | Disposition |
| --- | --- |
| Core display assembly | `core_machine_configure_display` configures the existing CGA/EGA VADP path only; it has no VGA configuration call. |
| IBM 5170 Model 339 | Its immutable descriptor selects CGA, not EGA/VGA. |
| DeskPro 386 Model 40 | Its composition selects Compaq Enhanced Color EGA, which is intentionally incompatible with the generic VGA configuration. |
| `default-at` | Its immutable descriptor selects generic EGA, but no source-qualified VGA card, option ROM, clock or firmware route. |
| Default firmware BDA names | Historical `VGA_*` symbol names describe BIOS data-area fields only; they neither bind a card nor configure VADP ports. |

No current profile/card meets the IBM source ledger's binding requirement. The Core VGA capability therefore remains unselected. This preserves one VADP state/port/memory/snapshot path and avoids claiming VGA hardware for IBM 5170, Model 40, or `default-at`.

## Gate reconciliation

The full-gate probe initially isolated four deterministic test-fixture failures.
`current.core-machine-port-assembly-smoke` still zero-initialized the now-explicit HDC protocol, while `current.vm-session-initialization-atomicity-smoke` requested a 286-family override for the frozen 80386 `default-at` board. T479 correctly rejects the former; the latter must remain an actual 80386 board and uses its valid 80387 companion. The fixtures now name their existing ATA intent with `CORE_MACHINE_HDC_PROTOCOL_ATA_PIO` and the supported `CORE_MACHINE_CPU_PROFILE_80386`/`CORE_MACHINE_FPU_PROFILE_80387` request; no production HDC, FPU, profile, or VADP behavior changed.

The same obsolete 286/8087 request appeared in `current.vm-ibm-5170-root-resolver-smoke`; its expected resolved values now name the same supported 80386/80387 `default-at` child contract. Its historical CGA assertion is corrected to the selected generic-EGA descriptor.

`current.core-machine-controller-authority-smoke` also constructed its ATA HDC topology without the required explicit protocol. It now states `CORE_MACHINE_HDC_PROTOCOL_ATA_PIO` in both its comparison fixture and configured topology.

The focused VADP proof now emits the DAC and snapshot markers it already verifies, so the S4 evidence does not overstate command output. S5 delta is seven test lines added and seven removed; it creates no mutable state, profile field, or public interface.

## Verification

- `cmake --build --preset current-gates-gcc` passed.
- `ctest --test-dir build/mingw-gcc-x64 -R "^(current\\.core-machine-ega-external-port-smoke|current\\.core-machine-port-assembly-smoke)$" --output-on-failure` passed 2/2.
- `powershell -NoProfile -ExecutionPolicy Bypass -File tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .` passed.
- Focused fixture regressions pass. The final complete `ctest --test-dir
  build/mingw-gcc-x64 --output-on-failure` run reached 296/296 with no failures;
  its retained log reports 111.03 seconds of `current-gate` process time.
- `cmake --preset mingw-gcc-x64-release` then `cmake --build --preset
  current-gcc` produced stripped optimized Release
  `build/output/nxvm_0_5_0480.exe`, SHA-256
  `7C18DF509A2359554390B3DF981057CB2F476F88F2E612038F2B02FD08A21535`.

`M5:T480:S5:PROFILE-BOUNDARY:OK`

`M5:T480:S5:REGRESSIONS:OK`

`M5:T480:S5:CLOSURE:OK`

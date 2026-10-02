# M5 T540 S52 Board-Owned VADP

## Scope and actual diff

The sole VADP instance moved from flat `core_machine` into the existing
`core_machine_board_state`. Its guest video state, VRAM and port routes,
snapshot producer and board clock still have one owner; copied snapshots
remain presentation values, not a second mode or frame state. Construction,
configuration, advance, reset and finalization retain the same chip instance
and order. CGA/EGA/VGA behavior, geometry, palette and timing are unchanged.

The display-authority gate now rejects a flat VADP or neutral scheduler
reach-through. The tracked source/gate/test diff changes **8** files, adding
**21** and removing **12** lines (net +9), chiefly instance paths, one direct
private-header include and the gate. No profile, INI, firmware, media,
Shared or MyNES source changed.

## Verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates`: x64 and x86 pass, including display
  authority, strict compilation and document governance.
- Fixed-profile external boot probes, **once per profile and width**:
  Default reaches `dos-prompt`; XT, IBM 5170 and Model 40 reach
  `installer-running`. **8/8** accepted terminals.
- Eight optimized 0540 EXEs rebuilt in the four existing
  `assets/nxvm/<profile>/` directories. PE format is correct for four x64
  and four x86 products; `objdump -h` finds zero `.debug` sections.
  SHA-256 by profile, x64 then x86:
  - Model 40: `EBCADB2475A9F6F73B64B193D5F45477BF0B5EA822D9F0DE68C71B41A84B2B91`, `871E0F7F8909304216DB87484ADB8BDC4C7B1B9FD02762682EEB4FD5C8A39F33`.
  - Default: `6B6208818420DB96DB8097B25F598E5A2B026B469CC465F496C3EA6F39DB0945`, `4282F9BDCFACD2B2A4B4D11C53F6B5524A252A141B03259AF572F5E7025F8440`.
  - XT: `2EC752C40F16D7393DE9D6343A797173AE6CF3BB72538227FB8184AC2AC0F8EF`, `F1F415BDAB15D3AE9D0A173FAB944A1E3C70E0C886699EF6DDF7D3789A0CDF75`.
  - IBM 5170: `96B3C648ED34033AF61E8CCA87942DAF216A19BD1521D893CE708785D9306E19`, `5C7D706F0DF2E6532E12BCB2DB284D0FD0BC65C8657E7E77B89A0C1A38DAED54`.

S52 closes only the VADP instance group. S53 receives remaining board
electrical/callback state; T540 remains open.

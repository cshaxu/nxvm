# M5 T540 S49 Board-Owned FDC

## Scope and actual diff

The one FDC controller instance now resides in the existing sole
`core_machine_board_state`, not flat `core_machine`. The frozen FDC topology,
media registry, drive configuration and board request wiring remain in their
existing roles. No controller copy, mirror, accessor API or parallel route was
introduced. Construction and failed-construction rollback, DMA binding,
port dispatch, deadline/ready effects, IRQ, reset and finalization use the
same instance and order. FDC chip behavior and timing formulas are unchanged.

Three production C files and direct unit/integration consumers were retargeted.
The FDC boundary gate now rejects a neutral scheduler reaching into board FDC
state; its negative fixture checks the new specific rejection. `topology->fdc`
remains a frozen configuration input and `probe->fdc` an integration observer;
neither is a second controller. The tracked source/gate/test diff adds **121**
and removes **109** lines (net +12), chiefly instance paths, seven needed
private-header includes and the gate. No profile, INI, firmware, media,
Shared or MyNES source changed.

## Verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
  The initial x64 run had only the FDC negative fixture's old expected error
  text fail; its injected violation was correctly rejected by the new gate.
  After correcting that expectation, the focused fixture and the entire x64
  suite passed.
- `verify-current-specialized-gates`: x64 and x86 pass, including the FDC
  boundary, strict compilation and document governance.
- Fixed-profile external boot probes, **once per profile and width**:
  Default reaches `dos-prompt`; XT, IBM 5170 and Model 40 reach
  `installer-running`. **8/8** accepted terminals.
- Eight optimized 0540 EXEs rebuilt in the four existing
  `assets/nxvm/<profile>/` directories. PE format is correct for four x64
  and four x86 products; `objdump -h` finds zero `.debug` sections.
  SHA-256 by profile, x64 then x86:
  - Model 40: `CD3040E10DC41B411F1537BE4F6A6F861420AACC592BE50FFCC221366E4C9F76`, `3FEDB47BEB8D0B86C071C67123880D0550FDBE860297E41F844D40BEB713FA4D`.
  - Default: `374EA41E8E0B347A9F6D026EC0B5C3C91B2E327D9C29E97003A035E1A1A990E8`, `539CDE52238FBFEF871038A0269D7E04CD5B16044BBBFE01996322859AF14140`.
  - XT: `F8E4DF76D60303A0C81A0BEB9685BFB88C3E761AE44B17D185B3024E715957D2`, `F5CE320E8AA52401D32843F816D4C7AEDE303D995F656C61F323448EEA9DE573`.
  - IBM 5170: `5E58F64A032EFF59E6EB186678BA1CA0E9052BA6BC15D47D17826C2050815235`, `C02010C26E4611648D12E0972641D42D245F1204F6D49A4C7802C8DABB82CC36`.

S49 closes only the FDC group. S50 receives HDC; T540 remains open.

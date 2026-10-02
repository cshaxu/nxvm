# M5 T540 S51 Board-Owned Keyboard Instances

## Scope and actual diff

The AT 8042 KBC and XT PPI-keyboard instances moved from flat `core_machine`
into the existing sole `core_machine_board_state`. They remain distinct
hardware topologies selected by the frozen board plan, not two copies of one
fact. No keyboard output queue, BAT/typematic phase, IRQ state, shim or
second input path was introduced. The same board constructor connects native
input, ports and PIC/NMI/speaker wiring; reset, event deadline/advance and
finalization retain their existing order. Keyboard command and timing
behavior are unchanged.

The keyboard transport gate now rejects flat instances and neutral scheduler
reach-through. The tracked source/gate/test diff changes **17** files, adding
**93** and removing **79** lines (net +14), chiefly instance paths, one
direct private-header include and the gate. No profile, INI, firmware,
media, Shared or MyNES source changed.

## Verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates`: x64 and x86 pass, including keyboard
  ownership/transport, strict compilation and document governance.
- Fixed-profile external boot probes, **once per profile and width**:
  Default reaches `dos-prompt`; XT, IBM 5170 and Model 40 reach
  `installer-running`. **8/8** accepted terminals.
- Eight optimized 0540 EXEs rebuilt in the four existing
  `assets/nxvm/<profile>/` directories. PE format is correct for four x64
  and four x86 products; `objdump -h` finds zero `.debug` sections.
  SHA-256 by profile, x64 then x86:
  - Model 40: `7F9357B449CA3AD66AC617659CF823908492563557CBC1F20DEE70F02C7DE079`, `410218C6DD0F92A26B941B2D9B8C6B767EC8BA4A4AE7C8BFD322D59A4032751D`.
  - Default: `8773ABD3B40E16FFE34D1140A7884EF34E5F917CA342AA781AEA7962476CDC9A`, `68A52FAA7D31F54BB0A3689F37229147A0797D83B5AF8F37E391166AF67A2D87`.
  - XT: `98A264651C970DFD06101A5884B32A07BA4C7C6E77C955060C2689EE19A9F0CF`, `25F36318906E2E23B7A6864512FEB7485D9114ADD9B0ECF3E7AC1C91DA732464`.
  - IBM 5170: `3636EA6E13BDC38F6F330373C6AEAB37BC4F16B5B5C944FD86FEDA979DCFBC3D`, `7D42CE1955F8F3AF9A086C4E498104A6E9DDEEE27F5DF4CFC37923FADD70681F`.

S51 closes only the keyboard group. S52 receives VADP; T540 remains open.

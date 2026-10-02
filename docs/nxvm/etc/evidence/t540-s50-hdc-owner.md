# M5 T540 S50 Board-Owned HDC

## Scope and actual diff

The one HDC controller instance moved from flat `core_machine` into the
existing sole `core_machine_board_state`. ATA PIO, IBM 5170 WD1003/ST-506,
XT Xebec and Model-40 Compaq/WD remain explicit personalities of that same
controller path. Frozen HDC topology, DMA request binding and media IDs remain
board inputs; integration observations remain observers, not another owner.
No controller copy, mirror, accessor API or parallel media path was added.

Construction, personality port route installation, Xebec DMA binding and
failure rollback, deadline/service, IRQ, reset and finalization address the
same board-owned instance in the same order. HDC chip behavior, media service
and timing formulas are unchanged. The HDC route gate now also rejects a
second flat instance or neutral scheduler reach-through. The tracked
source/gate/test diff changes **21** files, adding **150** and removing **135**
lines (net +15), chiefly instance paths, seven direct private-header includes
and the gate. No profile, INI, firmware, media, Shared or MyNES source changed.

## Verification

- Complete repository-only unit suites: x64 **469/469**, x86 **469/469**.
- `verify-current-specialized-gates`: x64 and x86 pass, including the HDC
  route/owner boundary, strict compilation and document governance.
- Focused external HDD boot and ATA PIO/DOS integration: **2/2** on x64 and
  **2/2** on x86.
- Fixed-profile external boot probes, **once per profile and width**:
  Default reaches `dos-prompt`; XT, IBM 5170 and Model 40 reach
  `installer-running`. **8/8** accepted terminals.
- Eight optimized 0540 EXEs rebuilt in the four existing
  `assets/nxvm/<profile>/` directories. PE format is correct for four x64
  and four x86 products; `objdump -h` finds zero `.debug` sections.
  SHA-256 by profile, x64 then x86:
  - Model 40: `2CEFBED664D55479793DB5C0F8499B73DD65D19D48F1791EE69E7C5A9D9F9AC4`, `0A59837ED4BFA50E768CCAC346047A6B3108A8AC1A6510205555E7E87E1068FF`.
  - Default: `B51B5745BF5D08433375D3D977C507A89866028CFBD1BC7E44FB0FC5C4093389`, `2B9F7F24BA618D537DD32E291BD3CDFCEB13E61039996FC81550DC8EC8027892`.
  - XT: `F0AC7421A8FD3EA01E00918CF9B2317A771A2D46F2E2E065881FA2541A907B36`, `CD4A94845FB045871005F67E6FF9295F1724EECD369779CA23F691D2D281B97A`.
  - IBM 5170: `9A973CDA2DA88FA273BE90CAC0F4B32F06F77C2CA0FBAE3BD46664441279B259`, `1165DCE03F6EE2A38B2B154DEFCFC389D45B0944CBF865CBEB4F6029C2EFC63F`.

S50 closes only the HDC group. S51 receives keyboard state; T540 remains open.

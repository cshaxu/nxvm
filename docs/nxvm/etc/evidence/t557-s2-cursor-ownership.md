# T557 S2 Cursor Ownership

## Admission And Mechanism

The owner approves the three-mode contract on 2026-10-13. Admission baseline
76ef26592 was clean. S1 remains a delivered naming change; this owner-approved
S2 extends T557 to Shared, NXVM and the necessary MyNES receiver/artifacts.
SoftPC is not edited or imported. Its earlier read-only adapter observation
is context, not a requirement to modify that repository.

Previously Core's instantaneous hardware visibility became KVM cursor_visible,
then Window applied its own 250ms phase. Two independent clocks gated the same
cursor. The new contract replaces that boolean with DISABLED=0, HOST=1 and
SOURCE=2. SOURCE consumes cursor_phase; HOST ignores it. Invalid modes fail
frame validation. This changes text-frame ABI; receivers must rebuild and
migrate explicitly, with no old-name alias or heuristic conversion.

## Ownership And Minimality Review

- Lib KVM Base owns the enum/copied contract. Window alone combines mode with
  the selected phase at paint and damage decisions. Its existing timer does
  no phase work in SOURCE/DISABLED; it is not a second SOURCE authority.
  Returning to HOST restarts a bright 250ms interval. Repeated HOST frames do
  not restart it. Existing freeze/unfreeze and timer failures retain their owner.
- KVM Console maps either enabled mode to the independent logical Console's
  enabled native cursor and constant true phase. Native blinking remains a
  host limitation; precise source timing is not promised or emulated in text.
- Emulator compares mode and phase in the existing publication path. No new
  event, queue, worker, clock or lifecycle state is introduced.
- Core snapshots retain hardware visibility and independently publish enable.
  Dark-phase coordinates remain valid. The existing capture-damage cache adds
  the enable field so enable changes during an already dark phase are not lost.
  Hardware field counters and the graphics/raster path are unchanged.
- MyNES's text conversion selects DISABLED instead of boolean false. Its
  graphics, keyboard and snapshot behavior are untouched; pending RDP work
  is not claimed repaired.

The one new private Window helper owns the shared paint/damage visibility
decision; it is not a public wrapper. The one public enum replaces a boolean,
not an additional control function. Core's added enable/cache fields represent
a previously conflated fact, not a second timing owner.

## Coverage Ledger

| Contract | Direct receiver |
| --- | --- |
| Mode values, invalid rejection, copied ABI | Lib frame-copy smoke |
| SOURCE bright/dark ignores timer and invalidates unchanged pixels | Lib Window capture contract |
| SOURCE to HOST starts bright; repeated HOST/unfreeze preserves phase | Lib Window capture contract |
| Disabled/source/host and both phase values in native Console approximation | Lib Console retirement contract |
| Mode-only and phase-only publications survive deduplication | Emulator machine-wait smoke |
| Enabled cursor remains enabled through hardware dark phase | Core video contract |
| Dark/bright snapshot maps to SOURCE with stable coordinates | Core frame smoke |
| Real executor produces phase-only publications | Default runner display cadence |

Sweep: `rg -n 'cursor_visible|cursor_phase|cursor_mode' src test --glob '*.[ch]'`.
The remaining cursor_visible fields belong to Core instantaneous hardware
observations, independent logical Console and observer fixtures, not a retained
KVM boolean interface. Rasterized graphics already contain their hardware
cursor and use no Window cursor overlay. Low-level Console's phase interface is
retained for its separate direct callers; KVM no longer forwards source phase.

## Qualification

The first complete x64 unit run was 591/592: the new Core frame test reset its
snapshot fixture without restoring cursor_enabled before expecting SOURCE.
The fixture was corrected (production unchanged), and the focused frame test
passed. Final complete runs below supersede that failed attempt. An earlier
five-test invocation overlapped unfinished linking and is not counted as proof.

Final qualification passes on both x64 and x86:

- 592/592 repository units, including Lib, Emulator, Shared x86, Core and all
  five Apps. Final elapsed times: x64 102.15s, x86 99.60s.
- 36/36 PC integration executions per width: default 23, XT 3, AT 5,
  DeskPro 5. DeskPro batches took 139.13s and 187.17s respectively.
- 12/12 MyNES integrations per width.
- Both specialized gate aggregates, six Shared and both Core manifests,
  NXVM/MyNES documentation governance and git diff --check pass. The strict
  compilation matrix still explicitly has three historical deferred rows;
  this is not a claim of exemption-free compilation for the whole repository.

These are automated runs in the existing environment. Desktop-labelled native
tests passed separately within those totals: three unit tests per width
(x64 1.60s, x86 1.40s) and two MyNES integration tests per width
(x64 1.32s, x86 1.09s). Background/pure contract passes are not represented
as manual visual or phone-RDP acceptance. Linux was not tested.

Original Ninja graphs and the single persistent build/.ccache are reused
across products. Logs are build/t557-s2-<width>-<app>-unit.log and
*-integration.log, with specialized qualification in *-gates.log.

## Actual Change Review

Against admission 76ef26592, `git diff --numstat -- '*.c' '*.h'` counts
11 production files: +68/-28, net +40; 11 test files: +101/-26, net +75.
Documentation, manifests and generated EXEs are excluded from those counts.
The added code separates enable from phase, validates the enum, selects one
Window phase owner and verifies receiver behavior. No timing algorithm, second
presentation path, public control function, thread or queue is added.
Implementation self-review and the coordinator's actual-change review agree
with the approved three-mode boundary; no further in-scope correction is open.

The Shared x86 source/test pair is unchanged. Lib Audio, Storage, Types and
Console Broker are unchanged. INIs, media, snapshots and external ROMs are
unchanged. MyNES has one receiver line plus its rebuilt binary pair; its RDP
input investigation is not part of this fix. The ABI replacement deliberately
requires external receivers to adopt cursor_mode and rebuild.

Phone/RDP timing and manual visual acceptance are not inferred from mocks or
automated native desktop tests. SOURCE follows emulated hardware time; this
does not promise a fixed 250ms wall-clock cadence when emulation runs slowly.

## Published Artifacts

All ten existing release targets were rebuilt. PC releases remain 0.5.0557;
MyNES retains its current 0.0.0044 identity. Deployment ran the existing PE
architecture check; a separate final read confirms 8664 for x64 and 014C for
x86. The four real PC integration paths checked their deployed identities and
own INIs. Exactly ten EXEs remain in assets; no INI content changed.

| Asset filename | SHA-256 |
| --- | --- |
| my5160_0_5_0557_x64.exe | 2BCD90C0C757C168B78C24F3CD3DE3F717CEBDA507E929D791DC4B526F72F9A7 |
| my5160_0_5_0557_x86.exe | 4DBA88DA224836BC72E16181E5985ECF76637FB07197DA977C1C88A4C2C8946F |
| my5170_0_5_0557_x64.exe | 4E48F930BC92120C867B986BC9ED5C32F0EE76DCDC5B292245ACA61CED7C315F |
| my5170_0_5_0557_x86.exe | 99A8738B4B11A4D17D3C08D206BBB348E28D1A382536182EB11BA68131DA3865 |
| mydeskpro386_0_5_0557_x64.exe | 871B0FEA07825E3368AD6FD726D7CE301DCB5E381F5C1D127CCAE1422F5CDBFE |
| mydeskpro386_0_5_0557_x86.exe | E1FEFCD706F1919E8BC7935624F2A21CEC20D97FC70737F5505378026B635E89 |
| nxvm_0_5_0557_x64.exe | 1870F1D3E5A9E0FAC61DF049D53FEC4EAE9CE94E5BA968C248ED115FBB0F4D30 |
| nxvm_0_5_0557_x86.exe | CB6AEAF49A0A168D5D3C48BE60D858BAAE9407AC6676735F311FFEEAE294D454 |
| mynes_0_0_0044_x64.exe | 0029597788C3CF3ABE087211C0CA62CF0FB62FEBF4D486CD02301A3A84ECFCA8 |
| mynes_0_0_0044_x86.exe | 3C769404DE8B60E1A6C199FFFEF76D2BFA915FA1F7413B220D4B10732B55FB64 |

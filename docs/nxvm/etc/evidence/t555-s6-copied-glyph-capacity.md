# T555 S6 Copied Glyph Capacity

## Admission And Boundary

The owner approved fixed 32-byte glyph slots, unchanged default height 16,
separate S6 after qualified S5, complete dual-width receiving verification,
ten published EXEs and owner acceptance after delivery. Accepted S5 source is
`3cfadc6d4`; admission/closure governance is `468e191ff`.

This is copied bitmap capacity, not complete live EGA/CECG font production.
Construction-time normalized fonts remain 256-by-16. Core copies each glyph's
16 known rows into a 32-byte snapshot slot; the existing snapshot initialization
zeros the remaining rows. Its copied bitmap-height fact is 16 for those real
producers, independently of CRTC cursor/cell height. No nonexistent raster bytes
are invented. Live hardware producers retain their separate S7 qualification.

## Implementation And Quality Review

- Lib uses the Base-owned `KVM_TEXT_FONT_DEFAULT_HEIGHT=16` and the actual
  `KVM_WINDOW_FONT_HEIGHT=32`, enlarges both existing arrays and changes
  the existing renderer's glyph indexing. Zero retains the height-16 default;
  explicit heights 1--32 are valid and 33 returns UNSUPPORTED.
- Text frame size changes from 24,276 to 32,468 bytes. The outer Window frame
  remains 984,080 bytes because graphics still dominates the union. This is
  an intentional copied-layout change, not source/binary compatibility for
  adapters that previously packed fonts at a 16-byte stride.
- Core's copied bitmap-height/capacity facts and converter use the same fixed
  layout. A compile-time count/stride assertion protects that actual boundary.
  No bulk 8192-byte read is made from the unchanged 4096-byte construction font.
- No new Shared function/type, helper, persistent state, object, thread, queue,
  renderer or cache is introduced. The new Core height is a copied value, not
  another state owner. Existing validation and rendering remain the only paths.
- Changed production symbols use Lib Types. Public Lib knows no Emulator,
  x86, Core or App; dependencies remain inward and no native API is exposed.
  Emulator production bytes are unchanged; public x86 changes only private
  Debug capacity naming and a dead definition. Their tests do not
  gain a product or Core fixture dependency.
- Existing Lib tests cover zero/default and all heights 1--32, both banks,
  glyph 255/row 31, a populated row 20 at height 24, exact damage and rejecting
  height 33. Existing copy/mailbox checks cover the enlarged full text payload
  and still reject invalid input without modifying accepted storage.
- Core tests prove the normalized last input byte reaches its new slot, every
  padded row is zero, bitmap/cursor height separation, both banks, height 33
  failure preservation and the actual 5170 composition. Input-font construction
  tests remain at their real 16-byte stride.
- SoftPC is read-only. Its previously inspected packed-16 bitmap producer must
  adapt at its own receiving boundary when adopting this new corpus; it cannot
  bulk-copy old packed fonts and claim the new slot contract is satisfied.

Measured C/H for the complete approved S6 batch: Lib production eight files
+13/-11 (net +2), x86 production one file +2/-3 (net -1), Core production four
files +23/-11 (net +12); existing tests ten files +105/-24 (net +81).
Thus production is +38/-25 (net +13) and tests are +105/-24 (net +81).
README, manifests, governance and EXEs are excluded; counting uses Git numstat
against the accepted S5 source, whose C/H is unchanged by Td S178. The default
is explicitly DEFAULT-named and Base-owned; direct glyph-height/row names
describe actual fixed slots. Existing arrays change layout; no compatibility
aliases are retained. Owner-approved Console/Debug capacity spelling and the
unused Debug definition are repaired. Three unchanged-value assertions and
eight compile-check lines in existing tests explain the extra test accounting;
no new test executable or framework is introduced. The separately indexed
six-tree audit records deliberate genuine-bound/optional-spelling retention.

## Verification

The following earlier qualification checkpoints are retained as history.
The final owner-approved three-repair batch passes complete units 591/591
on each width (x64 287.55 seconds) and MyNES integrations 12/12 on each width
(5.51/4.28 seconds). The final four-PC x64 matrix passes 32/32; final x86
passes default 22/22, XT 2/2 and AT 4/4, but its DeskPro boot times out at
180.12 seconds (the other three DeskPro rows pass). Successful text capture
and BIOS execution do not prove the timeout's cause.

The owner approved a 300-second Model 40 startup wait on both widths. Only
NXVM CMake registration changes: the existing receiver gets 300000 ms,
CTest gets 310 seconds and the integration aggregate 330 seconds for setup,
other rows and cleanup. Other profile and unit budgets, guest timing and
production inputs are unchanged. This bounded budget delta is +15/-4 lines.
The approved-budget repeat passes all four DeskPro rows per width: boot
172.91 seconds x64 and 206.06 seconds x86. Logs are
`build/t555-s6-approved-300s-deskpro-{x64,x86}.log`. Together with the frozen
other three profile groups this completes 32/32 PC integrations per width.
The failed 180-second attempt is not counted as successful qualification.

Read-only disassembly also finds a large stack probe in display publication
before its cadence guard. This is a potential fast-path optimization, not a
proved timeout cause; no production change is made for it in this batch.

The original two Ninja/ccache trees build the modified code with the existing
warning policy; historical CPU exemptions are unchanged, not newly qualified
as exemption-free. Direct receiving tests pass 7/7 per width; six Shared manifests and
affected Types/corpus/dependency/owner gates pass. Full units include all five
Apps in the same graph, avoiding separate Shared compilation for MyNES: 591/591
x64 in 279.67 seconds and x86 in 119.24 seconds, under the unchanged 300-second
aggregate containment. MyNES integrations pass 12/12 per width (5.15/3.95 seconds),
including both automated native Window and Console pixel/text receivers.

The first four-PC integration matrix passed all 32 x64 routes; x86's first
three groups passed, but DeskPro timed out at 180.19 seconds. The paused
diagnostic shows BIOS FDC execution, successful text capture and no terminal.
The failure is retained in `build/t555-s6-deskpro-x86-first-timeout.log`.
A targeted unchanged-budget replay passed in 157.79 seconds; that is not a
proved timeout root cause or a performance repair. The retained narrow-margin
throughput boundary remains explicit. The owner subsequently required constant
renaming, so complete qualification and ten artifact rebuilds are rerun on
the corrected final names, rather than treating old binaries as final proof.
MyNES's source and tests remain unchanged.

The final named contract subsequently passes complete unified units 591/591
on each width (x64 254.27 seconds); MyNES integrations pass 12/12 per width
(5.72/4.64 seconds). The logs are
`build/t555-s6-final-names-unit-{x64,x86}.log` and
`build/t555-s6-final-names-mynes-integration-{x64,x86}.log`.
That earlier checkpoint is superseded by the approved-repair results above.
Source/test production code was frozen throughout the final matrix; only the
explicitly approved DeskPro test budget changed before the targeted repeat.
No manual interactive desktop or electrical/complete-video proof is claimed.
S6 remains open for owner acceptance after scoped delivery and pushed-diff audit.

## Actual Pushed-Diff Review

Coordinator review accepts the bounded implementation in Shared `a7e69a14b`,
NXVM `b2ee99b1c` and MyNES `5067d7b7c`. Each immutable P has exactly one
target. Actual C/H deltas match the measured +38/-25 production and
+105/-24 tests; NXVM CMake's approved budget delta is +15/-4. Core input
normalization, padded copied slots, both-bank strides and renderer bounds
agree. Changed declarations use Lib Types; no new public function/type,
state machine, alternate path, compatibility alias or outward edge is added.
The Debug command table and behavior remain intact.

All eight manifests pass after push. The final owner-local selection of
manifest, Types, corpus, component-DAG, naming and boundary regressions passes
34/34 per width (63.11/68.31 seconds); this is not another whole-tree dynamic
qualification or proof that every historical symbol/test name is perfect.
Changed Shared files are LF and the immutable diff passes whitespace checks.
The ten assets have correct PE widths and no debug sections; current MyNES
and restored default PC assets match their build outputs. No owner INI,
snapshot, media or MyNES source/test change exists in these Ps.
The final review removes an empty evidence heading only; no production change
or rebuild follows. S6 remains open awaiting owner acceptance, T555 remains
open and S7 is not admitted. The potential publication fast-path stack cost
is retained as an unimplemented audit finding, not a confirmed timeout cause.

## Published Artifact Identities

All ten optimized EXEs are rebuilt in the original persistent Ninja/ccache
trees. Shared object targets are reused across profile bindings. PE machines
are 8664 for x64 and 014C for x86; no compiler debug sections remain. The
current MyNES outputs and restored default PC outputs match their assets.
Owner INIs, media and snapshots are unchanged.

| Asset | SHA-256 |
| --- | --- |
| `assets/my5160/nxvm_xt_0_5_0555_x64.exe` | `2AE03AE898399339DF28A1EEE08FE47948DB117FA3B9408E508B5471C93890D4` |
| `assets/my5160/nxvm_xt_0_5_0555_x86.exe` | `B7A7BC44F0687E437783A65C7FC085F4676E5A3F868C3C14DB49E7C50410AA50` |
| `assets/my5170/nxvm_at_0_5_0555_x64.exe` | `ACD17A43B431022F200875392A8CFD4AD58AD45F4AD6328F2739455BAA4C7FBA` |
| `assets/my5170/nxvm_at_0_5_0555_x86.exe` | `7D3681BE499D97B371B0F7F56B67244FEC3D6057A6CA64B652C8EC14D3BD4900` |
| `assets/mydeskpro386/nxvm_model40_0_5_0555_x64.exe` | `D0F0D7FEF8DF291C619C9627A624B297F9AF80C7E6300B6F2B2F61A7B67F2129` |
| `assets/mydeskpro386/nxvm_model40_0_5_0555_x86.exe` | `E9137EED520CFF589469215F7E65B9CEF4DFD492158A324FDFF6273687CF3135` |
| `assets/nxvm/nxvm_default_0_5_0555_x64.exe` | `EA8B1D363F3237DF4EA03906892BE3EA0B9BC893E5C9E1BC4300EBBBB5CE5A7C` |
| `assets/nxvm/nxvm_default_0_5_0555_x86.exe` | `66E891523FCB62A5CC4DE80AE5234221F2C889588D11375F90F8ED093FABE3A8` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | `1D2D7712395D8AA9490609C27D0851757454DDD196F110524C6D12192EAE721E` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | `3D853C03EEFCE77B694FA5CC537C6ACD6721600606D557761A6DC202B929D77F` |

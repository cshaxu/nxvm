# T540 S72: constructor port failure ownership

Baseline: S71 P2 `2307dbbe2`. This consumes the constructor-registration
class in the pre-relocation ledger, not reset-time I/O or physical movement.

## Intake and complete failure class

Board construction contains one raw `registration_begin`, one final status
query and one rollback before whole-machine destruction. The candidate has
not been published. Every actual route-producing stage already returns its
typed Core batch status: A20, VADP CGA, XT PPI or AT KBC, DMA, PIC, primary PIT
and optional auxiliary PIT. Each return is checked and destroys the failed
candidate. Chip preparation failures use the same parent exit. Other binding
calls install signal callbacks/settings, not ports.

`core_machine_install_port_routes` owns its own checkpoint and rollback.
`core_machine_destroy` finalizes the board and then frees every remaining
Core port entry. The outer Board checkpoint therefore neither supplies a
missing error nor owns a necessary partial rollback. Delete it instead of
exporting another transaction facade. Preserve every constructor call and
failure return in its original order.

Search `core_machine_port_(registration_begin|registration_status|rollback_registration)`
through `src/app-nxvm`. Registry mechanics belong to port.c/port_interface.c;
the removed Board sites are the entire outside-owner construction class.
Three `core_machine_port_write` calls program refresh PIT during reset/configure.
They are a separate reset-I/O receiver: direct chip writes would omit the
existing Core port scratch-value side effect, while paused-debug bus writes
do not admit the construction/reset lifecycle. Do not silently substitute
either path in this S. Firmware and attachment cuts also remain required.

## Receiving proof

Expand the existing port-assembly test, using its original allocation injector.
For AT, AT with auxiliary PIT and XT, measure successful registration count,
then fail each allocation once. Each failure must return NO_MEMORY, leave
the public candidate NULL and stop at the selected attempt. The first attempt
beyond the count must succeed; a fresh default candidate must still construct.
No test-only API, machine configuration file or external unit input is added.

Full units, independent Core proof, gates, eight rebuilt products and one
headless external boot each precede acceptance. Shared, MyNES, INIs and asset
masters remain unchanged. The receiving results below complete this intake.

## Implementation checkpoint

The outer checkpoint/status/rollback is deleted, with all original stage
order, error returns and candidate destruction unchanged. The expanded
port-assembly matrix passes its two existing success markers on x64.
The authority gate scans all NXVM production, rejecting registration
mechanics outside port.c/port_interface.c. The similar-issue sweep leaves
raw registry calls only at those Core owners. No public ABI changes.

Git numstat across three tracked source/test/gate paths counts +34/-21,
net +13: production deletes 13 lines; the existing test grows by 22 net
lines and the prevention gate by four. Additional code proves complete
constructor failure coverage, not a second runtime mechanism.

Both-width complete unit/gate and eight-product Release jobs returned zero.
Logs use `build/s72-{build,unit,gates}-<width>.log` and
`build/s72-<profile>-<width>-{build,neutral,boot}.log`. The retained t540-s4
unit trees and t535-s4 Release trees are needed by this receiver.
Both full unit suites pass 470/470: x64 in 270.01 seconds, x86 in 78.16
seconds. Both-width specialized gates pass, including 402 strict compilation
rows (377 retained strict, 25 declared deferred). The original verification
job returns zero; it is not restarted.

All eight matching Release products/probes and independent neutral Core
executions pass. The independent proof retains `M5:T540:S69:NEUTRAL-LINK:OK`;
it links all sixteen neutral sources without PC board archives, but does not
replace the remaining physical Shared relocation.

Each boot row was executed once: default x64/x86 reaches `dos-prompt`;
XT, AT and Model40 x64/x86 reach `installer-running`, all with exit zero.
An initial Model40 x64 invocation omitted the asset-root argument and failed
before opening a session; correcting the harness arguments performed its
single actual boot. This is headless INI/Core/firmware/media proof, not a
native KVM/audio desktop acceptance or an indefinite intermittent-fault claim.
No completed unit suite or product job was repeated after observation timeout.

## Artifact identity and review boundary

All eight 0540 EXEs are optimized Release; post-build architecture checks and
objdump confirm x64 `pei-x86-64`, x86 `pei-i386`, with no `.debug` sections.
SHA-256:

| Product | Width | SHA-256 |
| --- | --- | --- |
| default | x64 | 5D495743787872DE0C36F3381336C01157E8D9237583BFA72B7EDFC868266729 |
| default | x86 | 72E27952E0385D56EC763C7A5A5A0C3D9A960B85F3CCEC402E27D7EB05D1757B |
| XT | x64 | D9763618015EF0C7368519A3E152E9A6DE286B5CA92F153C882864AC5F5D076C |
| XT | x86 | 4C7B5B3D7EC206846EA624D4FB711EDD59B8BE78D2AFB07F6B1A5F1C3ED81AF4 |
| AT | x64 | 8181399AA91FEA468CD8AC53DDCD25298C55FAEF380BCC94B444D4432BBBFAC7 |
| AT | x86 | 35A5FEA47AEC38DDD8C35CCAA81B2129F88C9A91792DB9E6786C625A7C90C7DC |
| Model40 | x64 | E234ADC13281F7DABC3F57785BBB92ED12696A0261E5C527302999C3125E5F4A |
| Model40 | x86 | 498576F6718997ED4E8C17CF223299E7E75678F408771E3A7F08922714BB2C83 |

Executor review checks all three source/test/gate diffs, task documents and
eight artifacts against the packet. No Shared, MyNES or owner INI diff exists.
The reset-I/O, firmware/provider/attachment and physical-move receivers remain
open in T540; this constructor cut does not claim their completion.

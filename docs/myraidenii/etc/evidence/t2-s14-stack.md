# T2 S14 Native GP Stack And Frames

Baseline `361d436c3`. This batch consumes native GP stack/register-set/frame
forms of V06, not segment-register transfers, PSW/MD, CALL/RET/interrupt frames,
complete native semantics, mode/8080, timing or Machine/board qualification.

## Source And Reproduction

The pinned [NEC October 1986 manual](../research/references.md), SHA-256
`fabbfcd19d4a2fcdaac9f0bc06b00bf0a6a4beedafbd3ff16accd075a374ea4c`,
printed 12-146--12-153 defines PUSH/POP, PUSHR/POPR, PREPARE and DISPOSE.
POPR restores IY, IX, BP, skips the saved-SP slot, then restores BW, DW, CW,
AW, advancing SP by sixteen bytes with seven transfers. PREPARE uses the full
eight-bit lexical level, not the later Intel five-bit interpretation.

The actual CPU baseline fails with
`V30:STACK-RESULT:7:0700:0:reads=3:writes=0:stop=1`: a provider refuses the
saved-SP word which POPR must not read. The repaired native branch skips that
word and completes the seven required loads, including odd and segment-tail
contexts. This is a synthetic original-vector reproduction, not a game fault.

Read-only behavioral research also inspected BSD-3-Clause NEC files credited
to Bryan McPhail at MAME revision `8a18b751b388390db32812be43b8802de8cac839`:

- [necpriv.ipp](https://github.com/mamedev/mame/blob/8a18b751b388390db32812be43b8802de8cac839/src/devices/cpu/nec/necpriv.ipp),
  SHA-256 `43EF08521B1A4A0605974325F39CBC277E1D2807706857C29B06B5751E596135`;
- [necmacro.h](https://github.com/mamedev/mame/blob/8a18b751b388390db32812be43b8802de8cac839/src/devices/cpu/nec/necmacro.h),
  SHA-256 `9946F4BD74C49DF63BD97AB93E3BBCA271B99D0502A2A967E9010705D1F87DB8`;
- [necinstr.hxx](https://github.com/mamedev/mame/blob/8a18b751b388390db32812be43b8802de8cac839/src/devices/cpu/nec/necinstr.hxx),
  SHA-256 `152F25CC0C3B86BA69A7E47ED6CCF1D8FD03540E312A8E17C0D82443C4CD13E8`.

No external source, table, control flow or manual payload is imported or
transliterated. Original test expectations follow documented operations.
Ambiguous SP alias ordering retains a behavioral L2 disposition: direct 54
pushes decremented SP, the existing FF /6 register-temp path pushes original
SP, and POP SP finishes with the loaded value. These are not newly certified
silicon facts. The reference reads the discarded POPR slot; the primary
manual's explicit seven-transfer contract takes precedence here. Native data
words at FFFF retain S8's separately sourced physically contiguous L2 tail
model; sixteen-bit SP arithmetic still wraps. No timing grade is promoted.

## Single Owner And Similar-Issue Sweep

Only the sixteen-bit POPA handler's V30 branch changes: advance SP over the
discarded slot instead of calling the ordinary pop reader. Intel sixteen- and
thirty-two-bit paths remain unchanged. All scalar push/pop, memory lanes,
instruction advancement, transaction/CPU rollback and external-cycle ownership
remain in their existing implementations. No new production helper, API,
type/layout, persistent state, memory path, replay journal or executor exists.

The complete `_kec_push`/`_kec_pop` caller sweep covers GP register/immediate/
memory forms, segment/flags, near/far control and interrupt consumers, PUSHR/
POPR and PREPARE/DISPOSE. Admitted GP forms reuse their current scalar helpers;
PUSHR already captures original SP, and PREPARE already preserves full lexical
depth. SP-source/target aliases are recorded above rather than guessed from
generic pseudocode. Thirty-two-bit/protected Intel helpers remain genuine
profile-specific behavior, not candidates for a native-V30 rewrite.

Manual 6-4 requires inhibition after all segment MOV directions and POP.
Current `_e_pop_sreg`/MOV 8E give DS/ES no native shadow, and MOV 8C has no
corresponding inhibition. This confirmed source-level gap is retained in
V06/V11 for a complete segment-transfer/shadow batch. It is not patched only
for POP or hidden by a second shadow flag here. PSW/MD and control/interrupt
frames similarly remain their original complete receivers.

## Owned Tests

`cpu_v30_stack_smoke.c` uses an independent mathematical register/word-transfer
model, not production metadata decoding. Each case checks complete CPU and
one-MiB real backing memory, exact ordered physical lane address/width/direction,
BEGIN/COMMIT/CANCEL phases and accepted write prefixes. Distinctive upper GPR
halves catch unintended thirty-two-bit writes. Preview is observational and
cannot consume data or modify CPU. Refused reads/writes must stop with complete
CPU rollback, no invented guest exception, and only already accepted writes.

The 3,356 success vectors include every GP register in direct and r/m aliases,
all 256 sign-extended immediate bytes, representative immediate words, all five
short segment choices, direct/BP memory addressing, aligned/odd/zero/FFFF stack
boundaries and overlapping operands. Register sets check captured original SP
and seven-load POPR. PREPARE covers lexical levels 0--255, both alignments,
overlapping/non-overlapping frame chains and zero/FFFF allocations; additional
boundary frames use 7FFF. DISPOSE checks independent old-SP/BP combinations.
Every scalar/set/boundary/DISPOSE data provider position is refused; the full
PREPARE matrix adds every position at levels 0/1/2/31/32/255. The separate POPR
provider always rejects the unused slot, including successful cases.

The existing memory-transfer fixture is extracted once into CPU-owned
`support/cpu_v30_memory_fixture.h`, shared by memory and stack units. Record
capacity grows to 2,048 for the longest lexical frame; this is bounded test
storage, not production state. The existing memory assertions remain unchanged.
Test seed memory is generated once then copied per case; no physical signals,
ROMs, App fixtures, second interpreter or new test framework are required.

## Actual Delta And Verification Boundary

Production +5/-1/net +4; tests/registration/shared fixture +453/-96/net +357;
combined +458/-97/net +361, excluding manifests, documents and artifacts.
The initial test estimate was net +400--600; extracting the existing fixture
instead of duplicating it keeps the actual increase below that estimate.
Eleven test-local helpers own the independent word model, expectations,
fixture preparation, record/failure checks and finite case enumeration. No
production helper or state is added. Existing strict-warning exemptions are
retained without expansion, not described as waiver-free compilation.

## Verification

Both original Release Ninja/ccache graphs build all targets. First complete
repo-only unit suites pass x64 666/666 in 574.51 seconds and x86 666/666 in
116.63 seconds, including 177 X86 Core units and all receiving App units.
Architectures run sequentially, without retries, budget changes or assertion
removal. The stack unit passes in 3.84/12.00 seconds, below its existing
thirty-second budget; the extracted memory unit also remains enabled/passing.
All registered manifest, Types, dependency, ownership and negative gates pass.
Manual corpus/manifest checks and product documentation governance pass too.

Frozen production SHA-256:
`F89F103C85F04CF6C3D6BE0268864A2D637E0DAF8CBBEEE8989499A3CD218AA6`.
Stack unit:
`6CD8B597277B5A35F0300414739B5B55E0F4854150C141A5831AADE6BEFBCB77`.
Extracted fixture:
`1E9E49274E81975E6F2DEF7293795173C1F12F88CCDD5D9DF4F5B3DDFF8E9B6B`.
Retained memory unit:
`8789D725B59F7526553FD09A6E8404151EF715E5B38E714674BCB146B6DF6837`.

Shared `1ec55bd3d` and NXVM `d3ad9d003` are pushed. Both graphs link/deploy all
four PC profiles at retained revision 0563, reusing CPU objects and restoring
the default profile after each width. All eight PE/product/version/lowercase-INI
checks pass; the [artifact inventory](t2-s14-pc-artifacts.json) records exact
architectures, byte sizes and hashes. Assets retain exactly ten EXEs including
the unchanged MyNES pair. INIs, snapshots, media, ROM inputs, other Shared
packages and MyNES source/tests remain unchanged.

Governance, relative documentation links and whitespace pass. The coordinator
reviews actual production, fixture extraction, complete new unit, registration,
manifests, eight artifact identities and documents before final acceptance.
Ignored baseline/target/build/unit/caller/artifact logs under
`build/myraidenii/t2-s14-*` remain needed for final T2 qualification; all batch
build/test/deployment handles have exited. No external-ROM integration, desktop/RDP,
physical audio or actual board/game run is claimed. No MyRaidenII EXE exists.
The lossless-colour [High TODO](../../states/TODO.md) remains a final-release
gate. T2 and the full original MyRaidenII goal remain open.

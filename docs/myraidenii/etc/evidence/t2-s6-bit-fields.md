# T2 S6 NEC Bit Fields

Baseline `3ceb11ef9`. This batch implements the four documented INS/EXT
encodings, NEC suffixes 31/33/39/3B, at the existing CPU owner. It does not
qualify the entire NEC space, V30 bus, timing or board execution.

## Sources, Ownership And Uncertainty

The pinned October 1986 NEC manual, printed 12-21--12-24 (PDF 66--69), was
rendered and inspected. It specifies register-register and register-immediate
forms, zero upper operand nibbles, length-minus-one, AX data, ES:DI insertion,
DS:SI extraction, one/two-word transfers and post-transfer index/offset update.
All six arithmetic flags are undefined. The actual example streams are
reproduced by CPU execution, not only a synthetic conversion oracle.
No external implementation or tables were copied.

One private NEC format classifier still feeds metadata, preview and the single
escape. Two private register-selection formats express the real encoding
difference: register length uses all three register bits, whereas immediate
length requires those bits to be zero. Both require MOD=11. The existing
ModRM register decoder captures both input bytes before mutation. The one
bit-field handler uses bounded word masks and existing logical memory access;
there is no second decoder, executor, public API/layout, persistent iterator,
clock, queue, worker, transfer journal or byte-splitting workaround.

All original inputs are captured before transfer, including AL/AH overlap
with AX and a shared offset/length register. Outputs follow the manual's
transfer-then-next-field order. The manual does not explicitly resolve the
final AX versus AL/AH output collision: this deterministic source-order model
is tested as a model, not a silicon-exact oracle. General source override is
implemented through the existing DS override owner; insertion remains ES.
Exact bit-field prefix/silicon and physical-bus contexts remain part of V05/V14.
Undocumented nonzero upper operand nibbles stop diagnostically, without a newly
invented guest instruction exception; their hardware results are not claimed.

The exploratory FFFF-start word case failed through the pre-existing V30
segment-limit path. S6 does not mask it by splitting accesses in this handler
or weakening an assertion. It is explicitly unqualified under V14, whose
receiving bus/address batch must resolve the shared owner against primary
address/transfer evidence. The accepted boundary proof here covers ordinary
odd word addresses and a bit field crossing FFFE to the next word at 0000.
It does not imply FFFF-start words or real V30 bus lanes are qualified.

## Owned Proof And Failure Boundary

The new CPU unit uses an independent bit-at-a-time oracle against the production
word-mask algorithm. It covers every offset 0--15, length 1--16 and legal
register pair, including high bytes, same-register inputs and accumulator
aliases, both immediate forms, odd addresses, field-index wrap, segment choices
and DF independence. Preview/truncation, preserved unrelated GPRs/memory/flags,
undefined flag marking, exact next-field updates and write counts are checked.
All 256 selection bytes for each form are checked through metadata/preview;
reserved forms execute only the existing diagnostic stop. The all-256 NEC
suffix test admits just these four additional forms, retaining all previous
families and negative cases.

Manual examples run two consecutive actual instructions for insertion and
extraction. They check intermediate offset 8, final offset 5/index+2,
inserted 0x001555A8 and extracted AX 0x000A then 0x1355. These independent
constants prevent the generic oracle from being the sole source of truth.

Every first/second read and write failure position is injected for all four
forms. The assertions check the exact attempt count, unchanged PC/registers/
flags and retained first accepted write, with no extra request or replay.
Index/offset and extracted AX publish only after successful memory operations;
already accepted MMIO/memory writes are not rolled back. The source-order
alias model and unknown physical bus details remain explicit qualifications.

The initial focused compile rejected one test conditional's signed promotion;
the test now uses an explicit Lib unsigned value. The first focused semantic
run exposed the FFFF-start inherited limitation above. Neither finding was
hidden by changing production compiler warnings or claiming a full-bus pass.

## Verification And Delivery

Shared `5ce9a3a9b` and NXVM artifacts `293dd5ac0` are pushed. Both existing
Release Ninja/ccache graphs build all current targets. Complete repo-only unit
suites pass without retry or budget changes: x64 659/659 in 293.09 seconds;
x86 659/659 in 159.30 seconds. All 170 X86 Core units and every other registered
repo-only unit are included. The new bit-field unit and original native,
bit/nibble and packed-BCD families pass. Manifests and registered dependency,
Types, ownership and negative gates pass within those full runs.

The eight PC 0563 artifacts are actually linked and deployed through both
original graphs, reusing their CPU/component objects across the four profiles.
Both graphs restore the default profile. Each product's PE width, identity,
version, lowercase INI token and SHA-256 is verified in the
[artifact inventory](t2-s6-pc-artifacts.json). MyNES pair and all INIs/media are
unchanged; no other Shared source/test package is edited. No external-ROM
integration, desktop/RDP, physical audio, game boot or gameplay is claimed.
There is still no MyRaidenII executable.

Actual `git diff --numstat` source/test accounting, excluding manifests/docs/
artifacts: production +86/-7/net +79; tests +290/-1/net +289 (new 287-line unit,
one registration and updated suffix inventory); total +376/-8/net +368.
The positive delta implements a real instruction family and independent proof,
not a wrapper/framework. Source SHA-256 is
`70A85CE5ED5987ED5756FF2F2D3487A4422AF5FA2527DFD9DE48406E166CAE7F`.
The instruction file's historical warning exception remains; a separate
`-Wall -Wextra -Wpedantic` syntax review reports no warning in the changed
handler region, not a claim that the complete legacy file has no exceptions.

Ignored build/focused/full-unit logs are under `build/myraidenii/t2-s6-*`;
the initial focused failure is retained rather than counted as a pass.
Four owned manual render PNGs were removed after inspection; pinned PDF/text
remain for the next CPU batch. Governance, relative links and whitespace are
checked before delivery. S6 closes only this implementation batch; the ledger
retains alias, prefix/bus, mode, timing, Machine and real-board prerequisites.

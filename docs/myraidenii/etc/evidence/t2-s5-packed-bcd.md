# T2 S5 Packed BCD String Family

Baseline `a06307ff4`. The receiving batch is ADD4S/SUB4S/CMP4S, NEC suffixes
20/22/26, with all documented lengths and valid packed-decimal operands.
It does not close the entire NEC/V30 or gameplay domain.

## Sources And Sole Path

The pinned NEC October 1986 manual, printed 12-42--44 (PDF 87--89), supplies
the encodings, one-to-254 digit range, ES destination/overridable DS source,
carry/zero behavior and odd-count uncertainty. All three pages were rendered
and inspected. The repeated compare/add/subtract example also demonstrates
unchanged indices and count between operations. No external implementation,
protected bytes or tables were copied.

The existing NEC escape now owns opcode fetch and admission once, dispatching
to bit/nibble or packed-decimal semantics. One private operand-format classifier
feeds metadata, preview and execution: unsupported, ModRM, ModRM+immediate,
implicit. This replaces the assumption that every admitted NEC form needs
ModRM, without another decoder, public metadata layout or mutable registry.
The existing bit/nibble handler retains its data path and no longer fetches
or admits its escape independently. Old Intel 0F and 8086 POP CS stay intact.

Packed arithmetic uses local base-100 carry/borrow and aggregate zero, with
existing per-byte logical read/write owners. CMP never requests a write. Source
override does not change ES destination, DF does not reverse the defined
forward string, and SI/DI/CX are preserved. Each offset wraps at 16 bits.
The longest valid operation is 127 bytes and at most 381 recorded data accesses,
inside the existing 512-entry observation capacity; no persistent iterator,
second clock, queue, worker, journal or snapshot is introduced.

Even-count CF/ZF are defined; OF/SF/AF/PF are marked undefined. For an odd final
digit, unused input high nibbles are discarded and the output upper nibble and
CF/ZF are explicitly treated as unqualified/undefined, matching the manual's
uncertainty rather than pretending exact silicon behavior. Legal low digits
still participate in the full carry/borrow chain. CL zero/255 and non-decimal
used digits stop diagnostically, without an invented guest INT 6; results for
those undocumented inputs are not hardware claims or Machine qualification.

## Owned Proof And Failure Contract

The owned CPU unit compares actual execution to an independent digit-at-a-time
oracle, not a copy of production base-100 arithmetic. It exhausts all 100x100
valid byte pairs for each opcode, checks all 254 documented lengths in both DF
states, every source segment choice, fixed ES, odd unused non-decimal high
nibbles, wrap and overlapping operands. Preview lengths/truncated buffers and
actual execution agree; all GPRs/indices/count remain unchanged, and compare
has no writes. Defined flags and observation capacity are asserted.

An actual six-instruction CPU stream independently reproduces the manual's
07654321 example through compare, add, compare and three subtracts, including
zero and 92345679 underflow. Semantic memory/flag/PC checkpoints, not an
instruction/time budget, define success.

Provider injection refuses every source/destination read and destination write
position of a three-byte string. It checks the exact accepted prefix, no extra
attempt/retry, retained earlier writes, fault/PC/register checkpoint and a
one-shot stop request. No rollback of accepted host/MMIO effects is promised.
One exploratory assertion incorrectly called raw refresh again after that
request; source review showed refresh is an explicit execution operation, not
a persistent stop latch. The existing Machine owner consumes the request.
The test now validates that actual contract, without adding another stop state.

The similar-issue sweep covers all NEC metadata/format/preview/escape consumers,
all three arithmetic variants and read-only/modifying failure paths. The prior
bit/nibble admission test accepts the newly admitted implicit forms while its
original semantics remain covered. Remaining bit fields, mode/emulation,
prefix/bus/physical time and Machine/game qualification retain their receivers.

The first full x86 run exposed one obsolete S3 native negative vector: 0F20
was previously rejected as an Intel MOV CR dialect, but now correctly decodes
NEC ADD4S. The native isolation sample is now 0F21 MOV DR, still outside admitted
NEC forms; its rejection/exception/register assertions are unchanged. The new
BCD unit independently proves 0F20's actual data path. This is a necessary
negative-vector update, not a production fix or removal of dialect isolation.
The first in-flight x64 run and full final reruns remain separately reported.

## Verification And Size

Both original Release Ninja/ccache graphs complete full builds; the final
test-vector update needs only its owned object/EXE relink. Initial full runs
each report 657/658 with the obsolete native negative alone failing (x64
315.24 seconds, x86 188.55 seconds). Final full runs pass x64 658/658 in
101.37 seconds and x86 658/658 in 186.45 seconds, including 169 X86 Core units
and all other registered repo-only units. No timeout budget was increased or
assertion removed; both initial failures remain recorded. The new BCD unit
takes 0.16/0.30 seconds in those final suites respectively.

Manifest, inward dependency, Types/ownership and existing negative gates pass.
Eight receiving PC 0563 EXEs are rebuilt in the existing graphs, restoring
default selection and reusing the unchanged components between product links.
Their per-file [PE/name/version/INI/hash record](t2-s5-pc-artifacts.json)
accompanies this delivery; all eight deployed identities pass.
MyNES 0044 pair, all user INIs and external firmware/media stay unchanged.
Lib, Emulator, X86 Debug/IBMPC/NXVM source/test packages stay unchanged.
Inspected temporary manual PNGs were removed; pinned PDF/text remain research.

All runtime evidence here is background CTest execution, not new desktop/RDP,
external-ROM integration, physical audio or RAIDEN II gameplay. None of those
unexecuted qualifications is claimed. Physical V30 timing, Machine admission,
remaining instruction/mode/bus work and the lossless-colour gate remain open.
Shared source/test/manifest delivery is `0fbc2cf9e`; NXVM artifact delivery is
`b4d5f2013`, each pushed immediately. Compiled instruction SHA-256 is
`721C637FAE753EB6549593ED9C6A5023E23D33A465857C3AF2C49AFE648D03E6`.
The receiving PC version remains 0563, independent of MyRaidenII T2.
Separate coordinator actual-change acceptance follows the product-record P.

Actual counted production delta is +130/-23/net +107; tests +295/-2/net +293,
including the new 291-line CPU unit. Git numstat covers the tracked C/registration
changes and the added unit's line count, excluding manifests/docs/artifacts.
The estimate was production +100--170/-20--45, tests +250--400/-2--6. The
private enum/helper expresses real implicit/ModRM operand formats shared across
three consumers, not another instruction registry or persistent mode state.

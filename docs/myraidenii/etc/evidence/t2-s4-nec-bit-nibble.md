# T2 S4 NEC Bit And Nibble Family

Baseline `ed0fda0bb`. This bounded V04 subfamily consumes all eighteen suffixes
10--1F, 28 and 2A, not the whole NEC space or V30 qualification.

## Source And Implemented Contract

The T1-pinned original NEC October 1986 manual supplies encoding and behavior:
printed 12-45--46 (ROL4/ROR4), 12-81--99 (single-bit operations), and Table 12-4
for defined versus undefined flags. PDF pages 90, 91, 126, 135 and 140 were
rendered and visually inspected, with extracted surrounding encoding text.
No external implementation/table, game image or protected payload was copied.

| Suffix | Defined family |
| --- | --- |
| 10/11, 18/19 | TEST1 byte/word, CL/immediate |
| 12/13, 1A/1B | CLR1 byte/word, CL/immediate |
| 14/15, 1C/1D | SET1 byte/word, CL/immediate |
| 16/17, 1E/1F | NOT1 byte/word, CL/immediate |
| 28, 2A | ROL4/ROR4 byte operand and AL low nibble |

Register and memory forms use the existing ModRM and segment/bus owners.
Only the low three/four count bits select a byte/word bit; this is not Intel
BT memory bit-string addressing. TEST1 updates ZF and clears CF/OF, marking
SF/AF/PF undefined through the existing instruction record. Other operations
preserve flags. Nibble operations leave AL's upper nibble unspecified; retaining
its post-write value is deterministic implementation behavior, not a hardware
guarantee. An AL destination checks only its defined final low nibble.

The approved NEC instruction-space enum is appended, with prior values stable.
The same metadata admission owns execution and preview. One private NEC-family
handler is reached by the existing 0F dispatcher, using the original trace and
group-handler style. Intel 0F dispatch, 8086 POP CS and the sole executor remain.
There is no separate interpreter, mutable mode mirror or new public function.
TEST1 uses the read-only bus path; modifying operations use existing RMW checks.
AL is changed only after a successful nibble destination write. Existing fault
publication restores the register checkpoint on provider failure.

Remaining NEC suffixes are diagnostically non-eligible, not declared hardware
illegal. Packed BCD, bit fields and mode entry remain complete later receivers
under V04/V09/V13. Timing and Machine admission remain explicitly unqualified.

## Owned Proof And Review

One CPU unit covers every admitted suffix, byte/word and CL/immediate forms,
all eight register encodings including high bytes and count/destination aliases,
direct/8-bit/16-bit displacement memory and default/override segments, count
mask boundaries and zero/mixed/all-one operands. It compares pure preview,
execution preview and actual retirement length, rejects every truncated bit-form buffer,
and checks read-only versus modifying memory publication.

Nibble vectors cover both directions, every byte value, all register encodings
and memory. Flags and the defined AL nibble are asserted; unspecified upper AL
is not treated as an oracle. Metadata/preview scan all 256 suffixes, accepting
only this batch. Reserved ModRM fields are rejected without guest writes or
invented INT 6. Read and write failure injection covers all eighteen suffixes,
including TEST1's absence of a write attempt and preservation of failed CPU/
memory publication. The existing native and complete Intel regressions remain.

The instruction-space sweep also found that an unknown enum previously fell
through with the initial valid-primary metadata. The same owner now rejects
unknown spaces, with a regression; an Intel profile cannot satisfy the NEC-only
minimum. This adds no alternate validation path or public error enum.

The initial focused test compile exposed one integer-promotion signedness
warning in its expected-memory expression; the operand now uses lib_u32 before
shifting. No warning suppression or behavioral expectation was weakened.

## Verification And Delivery

Both original Release Ninja/ccache graphs complete their full build and final
incremental links. Full repo-only units pass x64 657/657 in 290.39 seconds and
x86 657/657 in 154.04 seconds, including all 168 X86 Core units, all six Shared
packages and five Apps. No retry, increased budget or weakened assertion was
needed. The new family unit itself takes 0.55/0.14 seconds respectively.
Manifest, inward dependency, Types, ownership and existing negative gates pass.

Both graphs rebuild the four receiving PC products at current version 0563
and return to default selection, reusing CPU/components between product links.
The per-file [PE/name/version/INI and SHA-256 record](t2-s4-pc-artifacts.json)
accompanies delivery; all eight deployed identities pass.
All four user INIs and MyNES 0044 pair remain unchanged; Lib/Emulator, X86 Debug,
X86 IBMPC and X86 NXVM source/test packages are unchanged. Temporary inspected
manual PNGs were removed; the pinned source PDF/text remain ignored research.

These are background build/CTest results. No new external-ROM integration,
desktop/RDP, physical audio or RAIDEN II gameplay was run or claimed passed.
Timing, Machine admission and the owner-declined lossless-colour gate remain.
Per-target delivery and separate coordinator actual-diff acceptance follow.

Shared delivery `25895916f` and NXVM artifact delivery `c3a20dc4c` were each
pushed immediately. Compiled instruction source SHA-256 is
`4FD07B8DCFCE091C09E12D49EB9343423E3FAD36F69E58FCC591E18B9A5E2651`.
The eight artifacts retain PC version 0563; MyRaidenII's T2 is independent.

Actual source/test delta is production +101/-9/net +92 and tests +276/-0/net
+276, including the new 275-line CPU unit. This uses Git numstat for existing
source/registration plus the new file's line count; manifests, documents and
EXEs are excluded. The estimate was production +130--220/-15--35 and tests
+200--350. Reusing the existing ModRM, logical memory, RMW and preview owners
keeps the actual source smaller without deleting necessary semantics.

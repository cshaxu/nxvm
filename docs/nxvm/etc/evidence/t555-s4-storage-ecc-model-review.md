# T555 S4 Storage ECC Model Review

## Review Status And Scope

Prepared on 2026-10-11 against accepted S3 `2ad955473` plus the uncommitted
S4 worktree. Following the unblock approval, the owner explicitly replied
"Approve this concrete L2 model and continue implementation" to the model
review request. CURRENT records that authority; this supporting record does
not grant admission or change closure criteria. The arithmetic checks below
are not qualification of future production implementation.

The proposed change stays in Core. Shared-six, MyNES, owner INIs and external
masters remain unchanged. It uses the approved copied-record/track boundary,
existing HDC phases and scheduler, and installed-lease metadata. There is no
new public API, disk backend, sidecar, recovery thread or secondary scheduler.

## Original Evidence And What It Does Not Establish

* IBM Options and Adapters Technical Reference, volume 2, April 1984,
  printed Fixed Disk Adapter page 12, PDF page 314 (one-based): Initialize
  accepts eight parameter bytes, including maximum ECC data-burst length;
  command 0D reads ECC burst length. Archive SHA-256:
  `B5BF24EA3E63082D5C637DB8B08469C6D4929B4B9F6B7B24C7A211338B42A15F`.
* Xebec S1410 supplement, August 1982, supplies the four-byte check field and
  eleven-bit burst capability, but not a reproducible selected-IBM polynomial,
  bit order or initial register. Its generic command grammar is not a substitute
  for IBM's CHS grammar. Archive SHA-256:
  `EB60BB918EDD71D6E3CE20BB38B33EF20B2F5D7801822F7A1BA34E36F021135B`.
* Western Digital Storage Management Products Handbook, 1986, printed 6-134,
  PDF page 494: WD1002-WAH uses
  `x^32+x^28+x^26+x^19+x^17+x^10+x^6+x^2+1`, a five-bit correction span
  and a maximum 516-byte record. Archive SHA-256:
  `72382885ABFDE5CDEE875384AED7CA97C17112DB047D7792D3A4B4D3ED0C895F`.
  This is a WD-family reference, not proof of the selected IBM WD1003 algorithm.
* Compaq DeskPro 386 Technical Reference, volume 2, September 1986, printed
  9-7/9-8/9-12/9-13: Drive/Head bit 7 selects ECC, corrected multisector
  reads continue, uncorrectable reads deliver their flawed payload and stop
  at the failing address, and the selected 40 MB Long transfer is 512+4 bytes.
  Its four-byte field does not identify a polynomial or an eleven-bit span.
  Archive SHA-256:
  `E523F1ACF81B7223D94835FB9BEDDEF0738BA6B1C5E1C740028FDCF3DA79357F`.

Two primary mathematical references demonstrate a 32-check-bit, eleven-bit
single-burst Fire code: [US4498178](https://patents.justia.com/patent/4498178)
and [US4677623](https://patents.justia.com/patent/4677623). They do not identify
the code installed in Xebec or Compaq. Their factored generator, expanded
independently here, is `(x^21+1)(x^11+x^2+1)`:
`x^32+x^23+x^21+x^11+x^2+1`. Use the factorization rather than treating a
missing term in a transcribed expanded equation as an alternate polynomial.
No patent source implementation or emulator source is imported.

## Owner-Approved Explicit L2 Contract

1. Xebec and Compaq use the Fire generator with low 32 bits `00A00805` and
   an eleven-bit maximum single-burst correction span. Compaq's choice is an
   explicit model, not an original-hardware capability claim. IBM WD1003 uses
   the referenced WD-family generator `140A0445` with a five-bit span, likewise
   L2 rather than exact-controller proof. ATA PIO does not gain this physical
   check-field protocol.
2. Encode 512 bytes MSB first, initial remainder zero, no reflection, no final
   XOR. Append 32 zero bits, divide by the degree-32 generator over GF(2), and
   serialize the remainder as four big-endian bytes. Checking all 516 bytes
   yields zero for an intact codeword. This defines the emulator's record ABI;
   it does not claim physical on-disk preambles, scrambling or wire bit order.
3. A raw payload image without an explicit check field denotes a healthy
   logical sector. At the HDC boundary derive its check bytes from that payload.
   This does not pretend raw images carry stored physical ECC. Normal writes
   replace data and generated tags together; Long writes preserve supplied
   tags verbatim, including invalid tags. Long reads bypass correction and
   return the complete data/check bytes. No constant padding is used.
4. Normal reads check an explicit supplied tag, correct only a unique bounded
   single-burst candidate in the copied working record, then check again.
   Correction does not write back to media. A zero syndrome is intact; an
   ambiguous or absent candidate is uncorrectable. Errors beyond the stated
   capacity can alias another codeword: the model does not promise universal
   detection of arbitrary corruption or eliminate documented miscorrection.
5. Xebec's Initialize byte 7 limits correction to 0..11 bits; zero disables
   correction while retaining detection. Before Initialize the model limit is
   eleven; reset restores that limit and clears last corrected-burst length.
   Larger values fail through the existing command result rather than being
   silently clipped. These default/zero/upper-bound choices require review;
   the selected source specifies the field but not these details. Command 0D
   returns the last actual corrected span, not a fixed success byte.
6. IBM/Compaq Drive/Head bit 7 enables correction. When clear, retain a logical
   four-byte check field and detection, but do not correct. This is explicitly
   not a simulation of a physical CRC16 record. Raw encoding of the alternate
   CRC wire format and physical ID/check circuits remain outside this logical
   media representation, documented rather than hidden behind success.
7. Compaq marks corrected reads with its existing CORR status and continues;
   UNC uses the existing error/status and phase path to deliver flawed data,
   stop, and preserve the failed address/count. IBM WD uses CORR `04` and
   UNC `40` in its existing status/error registers. For selected IBM-Xebec,
   propose the explicit L2 completion model corroborated by the S1410 read
   description and pinned XT implementation's error vocabulary: corrected
   payload is transferred, then the command ends with sense `18`; uncorrectable
   sense `11` ends without transferring invalid data. Sense retains the affected
   CHS, and 0D supplies actual corrected length. This termination/code mapping
   is part of the requested L2 approval, not a selected-IBM hardware claim.
   It is not applied to Compaq, whose original source requires corrected
   multisector continuation and flawed UNC payload delivery. Use existing
   command/status/response ownership, not a second result queue or mirrored
   error flag.
8. Format invalidates old tags through the existing lease owner. For payload-only
   formatted records the same healthy-logical-sector derivation applies; no
   whole-image tag allocation is needed. Raw Long tags persist only in the
   approved installed-lease metadata and disappear with that lease. Close,
   replacement, reset and partial-transfer rules remain as already defined.

## Bounded Correction Procedure

Use polynomial arithmetic local to `core/chips/hdc`, not a public service or
full-record table. A normalized burst has its lowest coefficient set; its
degree plus one is its span. For each of the 4,128 codeword positions, shift
the syndrome backward by one factor of x modulo the chosen generator. Because
both generators have constant coefficient one, the inverse is:

```text
previous = (syndrome >> 1)
if syndrome's low bit is set:
    previous ^= 0x80000000 | (polynomial_low32 >> 1)
```

A candidate must be odd, fit the configured span, and fit wholly inside the
codeword. Count candidates before mutating the copied record. Exactly one
candidate permits correction; zero or multiple candidates do not. Recheck
the corrected codeword before delivering it. Leading/ending zeros do not
create multiple representations of the same normalized burst. Work is bounded
by record bits, not disk-image size. No repair mutates the provider's record.

## Independent Encoding Checkpoints

These values were computed with a separate BigInt GF(2) long-division calculation,
not with future production shift-register code. They are proposed encoding
checkpoints, not captured hardware vectors or decoder qualification:

| 512-byte payload | Fire model | WD-family model |
| --- | --- | --- |
| All zero | `00000000` | `00000000` |
| All FF | `64FFF4D8` | `0830D99D` |
| Byte i equals i modulo 256 | `3B404E21` | `3FD4534C` |
| First byte 80, remaining bytes zero | `D6800EB4` | `062DB771` |
| Last byte 01, preceding bytes zero | `00A00805` | `140A0445` |

The Fire factor `x^11+x^2+1` has measured polynomial period 2047; the complete
code's referenced period 42987 exceeds this 4128-bit shortened codeword. Before
production implementation, a separate review-only inverse-syndrome check swept
every position for masks `1`, `3`, `5`, all-one maximum-span, and first/last-bit
maximum-span. Fire passed 20,617 cases; WD passed 20,629 cases. Every case had
exactly one candidate at the injected position with the injected normalized
mask, including data/check boundaries. This is not exhaustive testing of every
possible burst pattern, production code or command outcome. Any collision
inside the promised capacity must reject the proposal rather than being
papered over by a successful encode test.

A subsequent review-only uniqueness check enumerated every odd normalized
burst mask of span at most eleven/five at every legal position in the
4,128-bit codeword. A separate polynomial multiply-by-x recurrence generated
each syndrome; a sorted integer array exposed equal syndromes without using
the proposed inverse decoder. Fire: 4,217,855 legal bursts; WD: 65,999 legal
bursts. Each set had zero zero-syndrome entries and zero duplicate syndromes.
Thus every error in the proposed bounded single-burst domain is detectable
and has a unique syndrome. This is exhaustive for that error domain only;
it is not a promise for multiple independent bursts, proof of hardware bit
order, production command qualification or authorization to implement ECC.
The transient arithmetic arrays are not a proposed production lookup table.

Named outside-capacity checks also produce no bounded correction candidate:
Fire syndrome from the twelve-bit mask `00000FFF`, and WD syndrome from the
six-bit mask `0000003F`, at the codeword's lowest position. These are useful
explicit failure vectors, not evidence that all larger errors are detectable.

## Implementation Boundary And Expected Diff

If approved, edit existing `core/chips/hdc/hdc.c` and private `hdc.h`, the current
chip/Board-Base/installed-lease receiving tests and fixtures, and Core manifests.
The existing record ABI is sufficient. Two private scalar values may be needed
for Xebec's configured correction limit and last actual corrected span; they
are real command state, not duplicated media ownership. Helpers must perform
encoding, syndrome/correction arithmetic or command outcome mapping; pure
forwarding wrappers are not justified.

Expected ECC production change: +220..320/-15..40 lines; test change:
+300..500/-20..60 lines. These are estimates, not measured diff promises.
Where a test provider claims HDC normal-write support, supply its real copied
record callbacks; do not weaken the registry to discard generated metadata.

Required proof: independent encoding vectors; every single-bit position and
boundary-crossing bounded bursts; named uncorrectable examples; correction
disabled/limited/maximum; raw import and format; Long invalid-tag transport;
normal-write tag replacement; multisector corrected/UNC status/address results;
partial-transfer/TC/reset no-commit; two-controller independence; installed
lease cleanup and actual Board-Base/Common receiving. Existing E3 logical-ID
and E4 healthy-controller model boundaries remain explicit, not ECC-circuit proof.

The adjacent Compaq checkpoint adopts busy/DRQ register protection and the
pinned WD/AT first-write/final-read IRQ behavior as explicit L2, with direct
and Board receiving proof. It does not claim exact selected-hardware IRQ edges.
After approval and implementation, run focused x64/x86 receivers, complete units,
four-profile integration once per width, rebuild all eight current PC EXEs,
verify manifest/PE/hash/scope, then deliver one complete S4 P for review.
No partial S4 implementation commit or closure is claimed by this document.

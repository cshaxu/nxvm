# T555 S3 RTC And Keyboard Source Ledger

## Baseline And Review Boundary

This is supporting evidence for the active T555 S3 packet, not a second
task authority. The pushed implementation baseline is `69dca87fb`; accepted
S2 governance is `f474ed9dd`. On 2026-10-11 the working tree contained only
S3 admission changes to Current and Queue. No production change, test run,
artifact rebuild or S3 qualification is claimed by this initial audit.

The complete S3 batch consists of RTC/CMOS, KBC/AT keyboard and XT
PPI/keyboard, their chip reset/register/deadline contracts and their selected
board port, IRQ, NMI, A20 and clock receivers. Shared-six and MyNES remain
read-only. A confirmed individual defect does not exhaust this batch.

## Original RTC Source

The owner-managed archive copy is Motorola MC146818A, ADI1026R3,
`manuals-nxvm/rtc/motorola-mc146818a-real-time-clock-ram.pdf`.
Its SHA-256 was directly verified on 2026-10-11:
`2BC7AE3D16D6A2D234481EFF4FAFB04955BDF88075BC626887121293400D1F1D`.
The existing archived copy was used; no new original was acquired or committed.
This scan has no usable extracted text. PDF pages 13--17 were rendered and
visually inspected; their printed page numbers are 12--16. Page references
below use printed numbers. Renderings remain ignored build intermediates.

Table 4 on p.13 specifies operating DV selections `000`, `001`, `010`
for 4.194304 MHz, 1.048576 MHz and 32.768 kHz respectively; `110` and
`111` hold the divider reset. Other combinations are test functions, not
normal operating bases. Table 5 specifies identical RS3--RS15 rates across
the three operating bases, with distinct RS1/RS2 entries for 32.768 kHz.
SQW frequency equals periodic event frequency, not half that frequency.
Figure 15 and Table 6 distinguish the 244 microsecond lead from the
248/1,984 microsecond update duration. Register B's description explicitly
clears UIE when SET is raised and holds SQW low while SQWE is clear.

## RTC Finite Audit Rows

The current owner is `src/core/chips/rtc146818/rtc.c`. Matching direct
receivers are `test/core/chips/rtc146818/rtc_contract_smoke.c` and
`rtc_calendar_smoke.c`; board receivers are in `test/core/board-base`.
An inspection result is not a passing regression. Every open row below must
receive its final source/implementation/test disposition before S3 acceptance.

| Row | Source and finite contract | Current inspection | Required disposition/proof |
| --- | --- | --- | --- |
| RTC-01 | Table 3; 00h--09h, BCD/binary, 12/24 hour and calendar rollover | Encoding, rollover, leap-year and alarm equality paths exist. DSE bit is stored without April/October special updates. | Audit full valid calendar corpus and implement confirmed DSE behavior; malformed programmed dates remain explicitly non-qualified. |
| RTC-02 | p.12; 64 addresses, 50 RAM bytes; seconds high bit unavailable | Register bound is 64, seconds writes strip bit 7, 32h remains ordinary chip RAM. | Preserve and test bounds, read-only bits, RAM/reset retention and board-owned century convention. |
| RTC-03 | Table 4; three normal DV selections and two reset selections | Confirmed defect: running predicate accepts 20h/50h/60h, frequency helper maps 50h/60h as normal bases. | Replace incorrect decoding; test all eight DV values without assigning hardware behavior to undocumented test modes. |
| RTC-04 | Table 5; RS0--RS15 per operating base | Confirmed defect: shifting the raw input base is wrong for the fast bases. Existing selected 32k RS3--RS15 formula is correct. | Table-conditioned full rate matrix; preserve PF independent of PIE and no output at RS0. |
| RTC-05 | pp.13--15; SQW frequency, SQWE low output, divider reset | Confirmed defect: output toggles once per periodic event, halving frequency; SQWE clear can retain a high sampled output. | One divider phase owner for PF and both SQW half-cycle edges; test phase, enable/disable/reset and elapsed-chunk equivalence. |
| RTC-06 | Table 6/Figure 15; UIP lead, update duration, UF at completion | UIP derives from one phase with configured lead/update ticks. Default duration is always 1,984 us, including fast bases. | Reconcile normal-base default durations, configured board quantization and completion phase; do not invent values for undefined reads during update. |
| RTC-07 | p.15; SET aborts update, clears UIP and UIE; periodic/SQW continue | Confirmed defect: B writes retain UIE with SET. Calendar phase stops completely during SET while periodic phase advances. | Verify divider phase continuity and abort/release semantics against original source; include no calendar update/UF/AF while SET. |
| RTC-08 | p.13; reset-to-operating first update relation | Current reset release seeds half a second after every non-running selection, not specifically divider-reset selections. Existing test incorrectly treats DV000 as stopped. | Correct test and distinguish explicit divider reset from non-qualified test combinations; resolve start/completion relation from source. |
| RTC-09 | pp.12,15--16; PF/AF/UF independent flags, enabled OR IRQF, C read-clear | Current enabled OR, callback edges and C acknowledgement have direct receivers. | Sweep all enable/flag combinations and deferred enable; retain one IRQ output and non-reentrant execution contract. |
| RTC-10 | pp.15--16; chip RESET preserves A, SET, DM, 24H, DSE, RAM/time; clears delivery enables/C | Current reset preserves those mode bits and calendar phase, clears C and SQW. It assigns D VRT unconditionally. | Confirm retained-powered model and reset distinction; no invented battery-loss API. |
| RTC-11 | p.16; D VRT and power sense | Power-loss input is absent; construction assumes valid retained power. | Explicit model boundary; do not claim battery failure qualification or add public power API without demonstrated selected consumer. |
| RTC-12 | Chip timing input and read-only IRQ deadline | Periodic/update/alarm deadlines share chip phase; calendar addition can overflow for enormous elapsed inputs. | Reconcile prediction with advancement, SET/reset/rate changes, no-pending behavior and elapsed arithmetic; no second scheduler. |
| RTC-13 | Selected IBM/Compaq ports, IRQ and NMI wiring | Board-base owns consecutive ports, index masking, NMI bit and PIC source. Configure installs routes and applies NVRAM defaults once. | Original board-source comparison and actual port-to-PIC/NMI receivers for every retained variant. |
| RTC-14 | Selected clocks and quantization | PC/AT profile uses 8/65 tick phase inputs for source-rational mode; fallback inputs are zero. | Audit all three retained AT-family descriptors and truthful precision tiers; XT RTC absence is non-applicable, not a synthetic device. |
| RTC-15 | Selected reset and CMOS persistence | Board invokes chip reset; configured checksum is board-owned and RTC general RAM has one owner. | Prove calendar/RAM/IRQ reset route; preserve owner media/configuration and no duplicate persistent CMOS store. |
| RTC-16 | Electrical oscillator/bus timing and inaccessible update data | No electrical bus or power-loss implementation is selected. | Retain explicit L4 electrical boundary and undefined update-access boundary, without weakening defined logical register contracts. |

## Remaining Batch Audit

KBC source inspection covers controller command/data, output origin/IRQ,
translation, delayed replies, native serial backlog, BAT/repeat admission,
reset, AUX capability and deadline APIs. The truncated command-switch portion
was separately reread. These are controller/firmware transport semantics,
not execution of an Intel 8042 MCU; the UPI manual alone cannot prove IBM
firmware commands or their service durations.

PPI and XT keyboard source inspection covers mode/control/latches, directions,
BSR, reset, FIFO/overrun, serial completion/backpressure, clock/clear lines,
BAT and deadlines. The AT keyboard endpoint was also read completely,
including command parameters, reply/resend, BAT, typematic and native
make/break admission. No endpoint qualification follows merely from reading it.

## Original PPI Source And Initial Rows

Intel 8255A-5, order 231308, 1993, archived as
`manuals-nxvm/ppi/intel-8255a-peripheral-interface.pdf`, has directly verified
SHA-256 `1F2DF669139B8F51B7B6CD7EB6B854A0F5F37A8A59943A02F2141B32E3ECC5C4`.
Printed/PDF pp.5--6 were rendered and visually inspected. No acquisition or
protected-source repository change was made.

| Row | Source and contract | Current inspection | Required disposition/proof |
| --- | --- | --- | --- |
| PPI-01 | p.5/Figure 6; RESET makes all 24 pins input/high impedance | Reset installs 9Bh and zero latches; mode-0 direction masks match Figure 6. | Exhaust all 16 mode-0 direction combinations and selected XT reset route. |
| PPI-02 | p.5; mode selection resets all output registers/status flip-flops | Confirmed defect: mode-control writes only replace control; latches retain prior values. Existing direct test expressly expects retained A5h across mode selection. | Clear output latches on mode selection; correct regression to distinguish mode reset from BSR, and verify selected board pin side effects. |
| PPI-03 | p.6/Figure 7; BSR changes the selected C bit without mode selection | Current BSR changes one latch bit and does not replace the mode word. | Retain full set/reset-bit matrix plus other-latch/mode preservation. |
| PPI-04 | p.6; mode-0 outputs latched, inputs unlatched | Current reads combine actual supplied pins with direction masks. | Retain per-port read/output proof and board-selected pin observations. |
| PPI-05 | pp.5--6; independent group modes and handshake INTE | Current API exposes no handshake edges; non-mode-0 reads use a simplified fallback. | Inspect selected IBM mode-1 use before classifying; do not claim full mode-1/2 support or substitute invented status/IRQ behavior. |
| PPI-06 | Electrical read/write/handshake timing | Chip API currently expresses synchronous register transactions. | Explicit source/model boundary; selected board mechanisms, not host sleeps or a second scheduler, own timing. |

## IBM Board Source Leads And Remaining Finite Rows

Original archived IBM PC/AT Technical Reference March 1986 has SHA-256
`3224C26B424A4745C765F56AF7D957D3C4AD959E0F719EBEB39540A2508E95D5`.
Original IBM PC XT Technical Reference April 1983 has SHA-256
`18CD473851FDFE40C5BF2C7CCC870772857D84509D95ADA7880473CD802C63B8`.
Both hashes were directly verified. Extracted AT pp.1-42--1-54 and XT
p.1-10 provide the following leads; critical scan pages require visual
confirmation before selecting repairs. The scan-code table demonstrates OCR
ambiguity (for example letters B/D rendered as 8/O), so extracted bytes alone
are not authoritative command or translation values.

| Row | Source/function universe | Current inspection and remaining action |
| --- | --- | --- |
| KBC-01 | IBM AT power-on/self-test, system flag, interface inhibit/release | Model reset enables keyboard IRQ/translation without disable bit; self-test does inhibit. Compare cold construction, board reset and firmware-selected initialization before selecting a global reset repair. |
| KBC-02 | Status OBF/IBF/SYS/CD/inhibit and selected AUX/error capability | Command write sets then immediately clears CD, making ordinary post-write reads always show data. Confirm original persistent input-buffer flag semantics. IBM AT bit 5 is transmit error, whereas enhanced AUX personalities use it for AUX origin; keep capability distinctions, not two byte interpreters. |
| KBC-03 | Command byte and 20/60/AA/AB/AC/AD/AE/C0/D0/D1/E0/F0--FF | Ordinary register, self-test, input/output and reset pulse routes exist. AC dump is absent and non-reset pulse bits are not modeled. Audit public consumer/output pin needs and source/model dispositions rather than guessing MCU firmware state. |
| KBC-04 | AT native scan translation, break prefixes, Pause, identity | Current retained map and E0/F0/E1 translation exist. Compare every selected key/response mapping to original keyboard editions; distinguish 84-key and enhanced capabilities. |
| KBC-05 | Single output, delayed command replies, OBF consume and IRQ1/IRQ12 | Native serial scan admission is single-byte while replies use a bounded FIFO. Read acknowledges origin. Audit reply ordering/backpressure and all actual PIC consumers; do not replace one path with per-BIOS queues. |
| KBC-06 | Serial timeout/parity, inhibit switch, clock/data/A20/reset | No incoming parity/time-out input exists. Board callbacks own A20 and reset; current service/serial delays are configurable. Resolve defined selected logical behavior separately from excluded electrical inputs and firmware-time estimates. |
| KEY-01 | Endpoint ED/EE/F0/F2/F3/F4/F5/F6/FE/FF and parameters | Complete endpoint source read; accepts scan sets 1/2, rejects 3, identifies enhanced keyboard. Compare actual selected endpoint capability and original command bytes before labeling omissions or changing responses. |
| KEY-02 | Set-3 F7--FD per-key/all-key controls | F7/FB/FC/FD ACK without state changes; F8/F9/FA resend. These require a selected-capability disposition, not a generic promise that every PS/2 command exists on IBM AT. |
| KEY-03 | BAT, reset/default/scanning, resend last non-FE output | Endpoint produces pending BAT and controller owns publication; repeat and reset-stream callbacks are present. Board-level BAT deadline and output conditions must be reconciled with source and retained earlier fixes. |
| KEY-04 | Typematic delay/rate, repeatable keys, extended-prefix make/break | One repeat-byte state exists; per-byte admission handles F0 and ordinary map. Audit multi-byte enhanced keys and large elapsed delivery/backpressure against exact endpoint semantics. |
| XT-01 | IBM XT 60h--63h PPI mode99h, switches and keyboard latch | Original mode word is mode0, not mode1. Board-base pinmap is distinct from AT port61. Verify actual clear/ack, direction changes and no synthetic RTC/KBC on XT. |
| XT-02 | Shift frame, bit clocks, reset clock-low threshold and BAT | Chip owns one serial/FIFO/BAT state; Board owns clock/clear and latch backpressure. Audit completion/deadline and mid-transfer clock hold/clear, not only successful whole-frame receipt. |
| XT-03 | FIFO whole-sequence overflow FF and keyboard reset queue behavior | Overrun replaces last queued character, serial byte remains separately in flight. Compare original FIFO/reset semantics and test all capacities/receiver outcomes. |
| XT-04 | PPI direction/mode changes, speaker/Gate, parity/channel-check NMI | Current mode write publishes keyboard lines only if delivery becomes enabled, and not speaker; clearing chip latches alone would leave stale external observations. Repair must include actual board pin publication if confirmed, not just the chip register. |
| BOARD-01 | Selected AT/DeskPro/default KBC/RTC identities, port ranges, masks, clocks and reset | Original IBM/Compaq source and all profile descriptor comparison outstanding. Shared chip logical facts cannot silently impose IBM-specific wiring on Compaq. |
| BOARD-02 | Four-machine direct-to-board/PIC/CPU receiving proof | Existing chip and board tests are inventory, not new qualification. Complete caller inspection, source-conditioned regressions and per-variant receiving tests remain outstanding. |

RTC and PPI defects above are source-backed, but no subset closes the S3
batch. All remaining rows require original-source and current-test
reconciliation before the cohesive owner-local repair and diff estimate.

## Original Scan And Actual Caller Reconciliation

The 2026-10-11 continuation visually inspected original IBM XT PDF p.22
(printed 1-10) and IBM AT PDF p.69 (printed 1-50), plus PDF pp.193--195
(printed 4-42--4-44). The archive hashes above still identify these sources.
Rendering/extraction is ignored research output, not an imported firmware or
source body. These observations replace OCR-only leads for these exact rows:

- XT port 62h PC5 is PIT channel 2 OUT. The actual
  `core/board-xt/xt_ppi_keyboard.c` input helper supplies only switches and
  parity/channel-check inputs; its config and bindings supply no PIT input.
  The actual PIT channel 2 already belongs to `core/board-base` and is sampled
  for speaker observation. This is a confirmed missing receiver, not a need
  for another PIT state or another port route. Existing
  `core_machine_xt_ppi_keyboard_smoke.c` asserts switches, speaker, IRQ1,
  parity/NMI and BAT, but does not assert PC5 against the real PIT output.
  The repair design must report its necessary borrowed-input/binding surface
  before changing a public interface; no such expansion has been made.
- AT status CD is set by a write to 64h and cleared by a write to 60h.
  Clearing it immediately after command dispatch is contrary to this defined
  input-buffer provenance. The same original page says inhibit status is
  sampled when data is placed in the output buffer; the current status reader
  instead samples the current input port. Both rows need source-conditioned
  direct and actual board receivers, not a BIOS-specific status exception.
- Enhanced F0 explicitly selects scan sets 1, 2 or 3 and clears the output
  buffer and typematic key. F7--FA configure all key types; FB--FD configure
  individual keys identified by set-3 codes. They are accepted in other scan
  sets but affect only set-3 operation. Enhanced F5/F6 preserve the LEDs.
  ED waits with scanning suspended, resumes the previous scanning state
  after its option, and a new command cancels the incomplete option sequence.
  These are confirmed enhanced-endpoint gaps, not optional ACK placeholders.
- The current endpoint always identifies AB83; it rejects set 3, implements
  only F7/FB/FC/FD as ACK/no-op, and rejects F8/F9/FA. The actual Machine mapper
  branches on set 1 and treats every other selection as set 2, so an endpoint-
  only acceptance of set 3 would publish wrong guest bytes. Repair disposition
  must cover endpoint, mapper, transport and selected board together.
- The original 84-key chapter instead explicitly reserves F7--FD and EF--F2
  as acknowledged NOPs. The My5170 descriptor's 84-key reference cannot prove
  an enhanced AB83 personality. This edition/capability conflict remains open
  pending the complete selected-keyboard disposition; no generic enhanced
  behavior has been silently claimed as original 84-key firmware behavior.

The Compaq archived Volume II copy was hashed and extracted for comparison:
`manuals-nxvm/platform/compaq-deskpro-386-technical-reference-vol2-sep1986.pdf`,
SHA-256 `E523F1ACF81B7223D94835FB9BEDDEF0738BA6B1C5E1C740028FDCF3DA79357F`.
Its chapter 5 distinguishes 84-key from enhanced scan modes. PDF pp.54--56
(printed 5-30--5-32) were rendered and visually inspected. Table 5-10 directly
confirms enhanced AB83 identity, all three scan modes and F7--FD key-type
controls; set-3 input is consumed without controller translation. The table
also says scanning is disabled after FB/FC/FD, so parameter admission and
restoration must be reconciled with the IBM description, not guessed from
the current ACK/no-op behavior. Extracted 84-key command descriptions remain
source leads. A shared byte interpreter must not erase the distinction
between the two actual keyboard personalities.

Board caller review also confirms that PPI mode-reset cannot be fixed only
inside the chip: mode-control writes currently notify keyboard lines only
when delivery becomes enabled and omit speaker publication. The direct board
test even writes PB while input and later changes mode expecting its previous
latch to survive. That oracle needs reconciliation with the original 8255
mode-reset rule, not preservation merely because the old test passes.

## Command-Byte And Keyboard-Edition Reconciliation

The next continuation visually inspected IBM AT PDF p.71 (printed 1-52).
This resolves two additional KBC-03/KBC-05 facts against the same archived
original, rather than against a later PS/2 command-byte definition:

- Bit 5 selects IBM Personal Computer keyboard-interface mode. In that mode
  the controller does not check parity or convert scan codes. The current
  no-AUX branch in `x86_kbc8042_set_command_byte()` forces this bit high as
  `KBC_COMMAND_DISABLE_AUX`, while the translation path continues converting
  set-2 bytes. `x86_kbc8042_set_aux_present()` repeats the forced-bit behavior.
  A selected original-AT personality therefore needs its own interpretation
  of this existing command-byte bit, conditioned on the existing AUX
  capability, not a second keyboard parser or new mirrored command byte.
  The AUX-capable personality must retain its distinct bit-5 contract.
- Bit 0 enables an interrupt when the controller places data in its output
  buffer. `x86_kbc8042_refresh_current_irq()` instead explicitly deasserts
  IRQ1 for `X86_KBC8042_OUTPUT_CONTROLLER`; dequeue acknowledges IRQ1 only
  for keyboard-origin bytes. The original-AT controller-reply path needs
  direct publication/acknowledgement and actual PIC receiving proof. An
  existing AUX receiver expressly expects no PIC interrupt for A9h; that
  assertion is a separate personality disposition, not authority to suppress
  all original-AT controller replies.

The current actual board tests were located under `test/core/board-at`, not
`test/core/board-base`: `core_machine_kbc_controller_smoke.c`,
`core_machine_kbc_aux_port_smoke.c` and
`core_machine_kbc_serial_cadence_smoke.c`. Inspected receiving cases cover
mixed keyboard/AUX/controller output, set-2 translation, reset/BAT ordering,
typematic inhibition and serial cadence. The no-AUX translation case
currently requires bit 5 high while expecting translation; this is a
source-contradicting oracle, not a contract to preserve. Remaining portions
of the large receiving tests still require complete review before edits.

An additional original source is IBM 5155/5160 Technical Reference,
March 1986, archived as
`manuals-nxvm/platform/ibm-5155-5160-technical-reference-mar1986.pdf`.
Its verified SHA-256 is
`3B0A2E6E5BED837EEBC7FE7FAF3ABB6EB44F2B816E1A785F71AB5323D0612A9C`.
PDF p.120 (printed 4-24) was rendered and visually inspected. It describes
the **101/102-key XT-interface keyboard**, not the original 83-key keyboard:

- Its 16-byte FIFO excludes response codes; a multi-byte keystroke must fit
  completely or be discarded with overrun. The page expressly places the
  overrun code at the seventeenth byte. The current implementation replaces
  the last queued byte at full capacity. The earlier XT-03 description must
  not be mistaken for source proof of that replacement algorithm. Resolve
  the selected keyboard edition and overrun slot before changing capacity
  or approving the existing overflow test.
- Except Pause, this edition's keys are make/break and typematic, with only
  the most recently pressed held key repeating. While transmission is
  inhibited, repeated makes do not accumulate in the FIFO. These facts do
  not by themselves establish the original 83-key keyboard's exact rate.
- PDF p.122 (printed 4-26) was also rendered and visually inspected; it
  confirms the minimum 12.5 ms clock-low reset request for this edition.
  The existing release-triggered BAT path
  cancels serial activity but leaves queued scan bytes intact. Reset queue
  disposition remains part of XT-02/XT-03, not an accepted repair yet.

The April 1983 original's PDF p.78 (printed 1-66) identifies the selected
8048 keyboard's buffering of up to sixteen scan codes, but its extracted
paragraph does not specify the overflow-slot algorithm. Neither a current
code comment nor the enhanced edition can silently fill that source gap.
All finite rows remain in the S3 coverage universe; the edition distinction
changes required evidence, not the task's scope or completion predicate.

The selected My5160 values in `profiles/xt_5160_268.c` bind XT PPI ports
60h--63h, a single PIC, 8253 and the macro-proportional 4,772,727 tick axis.
They do not explicitly select an enhanced 101/102-key endpoint or its
typematic capability. Its scan-set getter returns set 1 for the XT topology;
the XT serial chip accepts already-encoded native sequences. Thus the
enhanced manual is supplementary interface evidence, not proof that the
selected 83-key composition already implements that complete endpoint.

Additional current receiving oracles were inspected, including the AT
controller test's F5/F6/FD block. It explicitly expects FD to ACK without a
parameter and leave scanning enabled; this matches the old placeholder,
not the selected AB83 enhanced command contract. The reset/serial cadence
test also relies on immediate scan admission after the generic controller
reset. Cold-reset changes must reconcile these actual callers and explicit
test initialization, not simply alter defaults and delete failing assertions.

## Admission Mechanism And Remaining Interface Decisions

The original March 1986 XT PDF p.100 (printed 4-4) was rendered and
visually inspected. Its **83-key** chapter explicitly describes every key
as typematic and make/break, with break equal to make plus 80h. This is
independent evidence of the selected keyboard's repeat capability, not a
borrowed enhanced-keyboard rate. The native XT serial owner currently has
no repeat state, repeat timing input or repeat deadline. Machine input
passes KVM makes/breaks through the mapper to that serial owner; this route
does not provide a guest-time repeat owner. Host-delivered repeated makes
are not proof of the original keyboard's timed repeat behavior.

The existing Board constructor supplies 12.5 ms reset, 300 ms BAT, 60 us
first-edge and 25 us serial-clock values on its one macro-proportional axis.
Only four durations exist in `x86_xt_keyboard_timing`. Therefore repeat
cannot be correctly enabled by reusing its serial duration or adding a
second Board timer. The design must separately report the necessary chip
timing input and selected nominal/model provenance before public edits.
The source proves the capability; it does not supply an exact 83-key
typematic delay/rate in the inspected chapter. No L3 timing upgrade is claimed.

The enhanced AT input mechanism has a related, distinct interface constraint:

- `x86_keyboard_admit(keyboard, byte)` updates repeat state and returns only
  a status. KBC then queues the incoming bytes unchanged.
- Set-3 make-only key types must accept a released key without publishing
  its break bytes. Returning an error for an intentionally suppressed break
  would incorrectly report a product input failure. Returning OK cannot tell
  the current KBC caller to suppress it.
- Suppressing only the final byte is insufficient: its F0 prefix could
  already have entered the serial backlog. Filtering in Machine instead
  would duplicate endpoint key-type policy and bypass direct KBC callers.

Consequently the repair needs one endpoint-owned, bounded native-sequence
admission contract which distinguishes accepted input from bytes to publish,
and preserves failure atomicity against the KBC backlog capacity. This is
an actual existing-contract gap, not a reason to add a generic input
framework. Its final public signature and byte-wise-caller migration remain
to be designed and reported before implementation. Existing endpoint
repeat emission also sends only `repeat_byte`; complete extended sequences
must use the same native scan publication path, not command-reply staging.

The current KBC repeats are generated even while serial delivery is
inhibited. `x86_kbc8042_repeat()` discards native-backlog admission failures;
`ticks_until_event()` likewise includes repeat deadlines regardless of
inhibition. KEY-04/KBC-05 must therefore cover repeat eligibility and
capacity together, preserving accurate accepted input and avoiding a
second queue or replay of already accepted bytes.

The cohesive implementation surface is now bounded as follows; these are
planned repairs/retentions, not accepted regression outcomes:

| Ledger batch | Planned owner-local disposition | Required receiving proof |
| --- | --- | --- |
| RTC-01/02 | Preserve encoding/RAM; implement defined DSE rollover with only its necessary private fall-back phase. | BCD/binary and 12/24-hour calendars, spring/fall transitions, reset and RAM retention. |
| RTC-03/04/05 | Correct all normal DV/RS rows and derive PF/SQW from one divider phase; disable/reset SQW correctly. | Full rate/base matrix, half-cycle edges and elapsed-chunk equivalence. |
| RTC-06/07/08/12 | Keep one calendar/divider owner; correct base-dependent update defaults, SET/UIE, reset-release start/completion and overflow-safe phase addition. | UIP/UF boundaries, SET abort/release, preview/advance agreement and configured quantization. |
| RTC-09/10/11/16 | Retain one read-clear IRQ and retained-powered RESET model; battery loss, electrical buses and undefined update reads remain explicitly unqualified. | Enable/flag matrix, C acknowledgement, reset/teardown and unchanged boundary documentation. |
| RTC-13/14/15, BOARD-01/02 | Retain board-owned ports/NMI/checksum and selected clock ratios, with actual receivers and honest IBM/Compaq source tiers. | Real 70h/71h, IRQ8/PIC/NMI, selected descriptors and CMOS persistence. |
| PPI-01/02/03/04, XT-01/04 | Clear chip latches on mode-set, preserve BSR, republish actual board pin effects and add the missing borrowed PIT2 input. | Mode/direction/BSR matrix and real 60h--63h speaker/Gate/IRQ/NMI/PIT2 routes. |
| PPI-05/06 | Selected XT uses mode 0; unselected handshake modes and electrical timings remain explicitly unqualified rather than acquiring fabricated wiring. | Selected 99h contract and unchanged non-qualified-mode boundary. |
| KBC-01/02/03/05/06 | Correct original-AT command/status/IRQ and reset distinctions, preserve actual AUX capability, output/reset/A20 bindings and the sole output publication path. | Cold/reset initialization, C/D and sampled inhibit, OBF/PIC acknowledgement, command/output-port matrix and AUX separation. |
| KBC-04, KEY-01/02/03/04 | Retain actual enhanced AB83 endpoint, reconcile all three sets and F7--FD, LED/default/parameter/BAT/repeat contracts, and migrate the mapper and admission boundary together. | Original key tables through actual KVM mapping, endpoint and KBC/PIC, including failures and extended sequences. |
| XT-02/03 | Preserve one serial/BAT/FIFO owner; reconcile reset flushing, repeat capability and overrun-slot provenance before choosing their implementation. | Clock/clear and mid-frame states, queued reset, complete-sequence overflow and inhibited repeat. |

The remaining design decisions are finite: bounded enhanced input admission,
borrowed XT PIT2 binding, XT repeat timing input/provenance and the selected
overflow contract. They are not additional S identifiers, a new controller
framework or permission to close a partial batch. The no-ABI PPI mode/pin
portion has entered implementation after the source/caller dispositions
above and its reported delta estimate. Public binding and enhanced endpoint
changes still require their concrete contract/delta report before edits.

## First Implementation Progress: PPI Mode And Driven Pins

The initial production surface is only `chips/ppi8255/ppi.c`, its existing
interface comment and `board-xt/xt_ppi_keyboard.c`. No API, type, persistent
state or helper is added. Mode-set clears the three chip latches; BSR
retains the selected mode and unrelated port latches. The board publishes
speaker and keyboard lines after every PB/control write and normal reset,
using the existing drive mask. An input-mode PB write cannot clear the
external keyboard shift register. That register retains its own IRQ/ack
ownership; clearing a chip latch does not fabricate an acknowledgement.

Teardown review found that Board-Base destroys the native XT keyboard
before the PPI attachment. Its finalizer therefore revokes the existing
line observer before resetting the attachment; normal reset still publishes
the clock state. Speaker/PIC contexts remain alive for final reset. This
reorders two existing private assignments, rather than creating a new owner.

The existing direct test checks every mode encoding's cleared latches before
refilling them and retains the direction/read matrix. All eight BSR bits
also prove A/B and output direction are unchanged. The real-board receiver
now expects mode-set to clear PB and requires an explicit PB6 release; it
adds speaker direction/mode-reset and input-mode acknowledgement checks.
The original repeated-read, serial/BAT, refused-completion and reset tests
remain. Mode-1/2 handshakes are still explicitly unqualified.

Both original persistent Ninja/ccache trees built the PPI contract and XT
board receiver with jobs 8. Final focused executions pass 2/2 on x64
(1.11 seconds) and 2/2 on x86 (0.21 seconds), including the added pin/ack
assertions. Core source/test manifests, Core corpus boundaries, NXVM
documentation governance and `git diff --check` pass. The five tracked
source/test paths total +46/-12, net +34: production +14/-12, net +2;
tests +32/-0. This excludes manifests, documentation and artifacts.

The similar-issue sweep inspected PPI write/output callers and mode-set
comments in `src/core` and `test/core` (`rg` for `ppi8255_write`,
`mode-set`, `retains latches` and `Mode-0`). There is one chip write owner
and one XT board receiver; the direct mode matrix and real-board
mode-release oracle are corrected. Unselected mode handshakes remain
explicitly unqualified, and missing PIT2 input remains its separate row.
Actual-diff review confirms no second pin state, port path or controller
framework was introduced.
This is internal S3 progress, not a complete S3 P, artifact qualification
or permission to close the task. No Shared/MyNES or owner asset is changed.

## RTC Implementation Progress

The archived original's PDF pp.14--16 (printed 13--15) were visually
rechecked before the RTC edit. Tables 4--6 distinguish normal/reset/test
DV values, periodic frequency, update start versus completion, SET/UIE
and DSE. No original or rendering enters tracked source.

The production surface is `rtc.c`, its private `rtc.h` and the existing
public comment; there is no new API/type or change to Board input values.
`rtc_update_ticks()` is the single base-dependent default-duration selector;
explicit configured durations are preserved. `rtc_phase_ticks()` is moved,
not duplicated. All elapsed arithmetic is performed with bounded remainders
before adding to either phase. The divider's `periodic_ticks` now denotes
tick phase independent of selected RS; PF and SQW are derived from it.
The redundant `square_wave` cache is removed. The private
`daylight_fallback_done` bit replaces it and has one real responsibility:
prevent repeating the autumn hour twice. Normal RESET preserves that
calendar fact; date/hour programming reinitializes it, and midnight clears
it. Alarm prediction copies this same calendar state and uses the same
increment function as advancement; it does not own another clock.

| Rows | Implemented behavior and direct receiver | Remaining qualification boundary |
| --- | --- | --- |
| RTC-03/04 | All 8 DV x 16 RS encodings; independent Table-5 expected rates for normal bases, no PF/SQW for RS0 or inactive test/reset modes. | Test-mode internals/electrical oscillator remain unqualified; their inactive compatibility model is not silicon proof. |
| RTC-05 | Both SQW half cycles per PF period, immediate SQWE disable, enable at current phase, disabled-RS phase continuity and 45 whole/split advance contexts. | Initial electrical edge polarity/oscillator acquisition is not a new exact phase claim. |
| RTC-06/08 | Each normal base and both reset encodings: first UIP lead, update start at half-second and UF only after base-dependent update duration. Existing explicit 8/65 Board quantization remains. | Coarse clocks whose update window cannot fit are still caller model limits, not exact microsecond claims. |
| RTC-07/12 | SET clears UIE/UIP, does not update calendar/UF/AF, retains divider phase; UINT64_MAX elapsed under SET cannot wrap calendar phase. Deadline agrees on release. | Extremely large active-calendar advances still use the existing per-second loop; no bounded-runtime claim. |
| RTC-01 | 32 DSE cases cover BCD/binary, 12/24 hour, spring/fall, last/non-last Sunday and enable/disable; alarm preview predicts the same special update. Autumn remains single-use after RESET. | Complete ordinary valid-date/rollover and enable/flag audit still needs its final row disposition; malformed dates and setup inside the documented two-second preparation interval are not qualified. |

Existing calendar, RAM/reset, read-clear IRQ and actual 70h/71h/PIC receiver
tests remain. The previous DV000-as-stopped oracle now explicitly uses a
reset encoding, and the SQW test observes the actual half-cycle instead of
the former half-frequency output.

Counted five RTC source/test paths are +273/-44, net +229. Production is
+72/-41, net +31; tests are +201/-3, net +198. The positive test delta is
finite register/rate, timing, chunk and daylight context proof, not a new
test framework or a duplicate product test. Public signatures and private
scalar count are unchanged. Final focused executions pass 3/3 on x64
(0.19 seconds) and 3/3 on x86 (0.17 seconds), after rebuilding the final
flat DSE scenario matrix. Both Core manifests, Core corpus boundaries,
NXVM documentation governance and whitespace review pass. The similar-issue
sweep searches `calendar.periodic_ticks`, `square_wave`, `rtc_divider_hz`
and `rtc_update_ticks` across all Core production/tests: no external user
reads private phase, no square-wave cache remains, and the only duration
selection is the chip helper. Actual-diff review retains one calendar
advance/alarm-preview function and the unchanged Board publication path.
No complete S3 qualification,
new artifact or desktop/integration result is implied.

## RTC Calendar, Flag And Alarm-Horizon Reconciliation

The retained direct tests now cover all 100 programmed years and twelve
month-end boundaries in both BCD/binary and 12/24-hour encodings: 4,800
contexts. The test's independent month-length table includes February leap
years, December/year-99 wrap, weekday-7 wrap and midnight encoding. A further
96 contexts cover each hourly transition in all four encodings, including
11 AM to noon, 11 PM to midnight and day increment only at midnight. This
does not qualify malformed dates or every within-hour transition.

The IRQ test covers the six reachable combinations of PF/AF/UF against all
eight PIE/AIE/UIE combinations. AF is generated during an update, so AF with
UF absent is not separately reachable before the shared read-C acknowledgement.
All flags are generated with delivery disabled; later enabling asserts the
corresponding IRQ, disabling preserves flags, and re-enabling republishes the
level. Reading C deasserts and clears every flag. Writes to C/D are ignored;
all fifty general RAM bytes survive RESET.

Review of MC146818A ADI1026R3 printed pages 14--15 reconciles the existing
per-update alarm comparison with the defined one-hour autumn DSE repeat.
The retained preview loop incorrectly stopped at 86,400 elapsed seconds:
at Sunday October 25 00:00:01, the next midnight is 89,999 seconds away.
The new four-encoding regression fails on that exact deadline before the
production edit. The one preview loop now covers the maximum 25-hour civil
day, still using the sole calendar increment function and preserving
non-mutating preview. Each encoding proves no AF before the predicted tick,
AF on that tick, and the next day's midnight. No state, public interface,
thread or second deadline owner was introduced.

This continuation adds 176 test lines and changes production +2/-1 (net +1)
relative to the preceding local RTC work. The five complete uncommitted RTC
source/test paths are now +451/-45, net +406; production +74/-42, net +32,
tests +377/-3, net +374, counted using `git diff --numstat`. Original
divider/update/DSE/reset/board receivers remain. Final focused RTC executions
pass 3/3 on each architecture in the original
persistent Ninja/ccache trees. These are focused results, not complete S3,
desktop, artifact or cross-profile integration qualification.

Wildcard-alarm completeness and actual selected-board
clock/port/reset reconciliation still require their final dispositions.
KBC/enhanced keyboard and XT repeat/PIT2 work remain pending. The newly
confirmed horizon defect is part of RTC-01/12, not a new task or partial P.

## KBC Status Latches And Original-AT Command Personality

IBM AT March 1986 printed pages 1-50 and 1-52 were visually rechecked before
the controller edits. KBC-02 now preserves C/D until another host write:
command-port writes set it, data-port writes clear it, status/data reads
and synchronous dispatch completion do not alter it. The new direct test
fails against the preceding controller before the production change.

INHIBIT is sampled when a byte becomes the current output-buffer byte. The
one new private `lib_bool` retains that hardware-defined observation rather
than mirroring the live input pin. First publication and queued-successor
promotion capture the switch; changing the switch while a byte is present
does not rewrite its observation. Reset preserves the existing model's
released-switch initial status. No original source qualification is claimed
for a cold state before its first output placement. The actual board-port
test likewise distinguishes live admission eligibility from sampled status.

KBC-03/05 retain the existing AUX capability as the personality distinction.
Without AUX, bit 5 is PC keyboard-interface mode, not an automatically forced
AUX-disable bit. A private translation predicate now serves both native
set-2 publication and identity reply conversion. With AUX, the existing
bit-5 disable contract is preserved. Unsupported A7/A8 extension commands
do not rewrite the original-AT PC-mode bit. No parity injection capability
was added; the excluded electrical-error boundary is unchanged.

Original-AT controller-origin output now participates in the same IRQ1
publication/acknowledgement path as keyboard output. It never acquires an
endpoint-output callback. AUX-capable controller replies retain their
existing no-IRQ disposition, including the separate A9 receiver. Changing
the existing AUX capability reconciles current IRQ delivery through the same
refresh function, not a second interrupt owner.

Eight direct personality/translation contexts cover both AUX capabilities
and all bit-5/translation combinations, copied command bytes, scan bytes,
identity bytes and unsupported extension non-mutation. An actual Board-AT
port-to-PIC receiver observes six original-AT commands (20, AB, C0, D0, E0,
AA), IRQ1 vector 09, read-60 acknowledgement and PIC EOI. Existing mixed AUX
output, keyboard BAT/repeat and serial/backpressure regressions remain.
The old no-AUX forced-bit-5 oracle is corrected rather than preserved.

Counted four KBC source/test paths are +141/-19, net +122: production
+30/-17, net +13; tests +111/-2, net +109. One private sampled bool and one
translation predicate are necessary to distinguish historical observation
from live pin state and to give scan/identity conversion one policy owner.
No public signature/type, queue, thread or board-specific response path is
added. One intermediate incomplete header edit failed compilation; the field
was completed before final dual-width builds and has no qualification value.
Final four focused receivers pass 4/4 x64 (1.05 seconds) and x86
(0.21 seconds) in the original Ninja/ccache trees.

Cold initialization, unmodeled MCU diagnostics/electrical inputs and the
enhanced endpoint/complete-sequence admission, repeat and selected-keyboard
source conflict still require their remaining ledger dispositions. These
focused results are not full S3, artifact, desktop or integration acceptance.

## Verification Status

### XT PIT2 And Speaker Wiring Progress (Uncommitted)

PPI/BOARD rows: IBM XT April 1983 PDF 32, printed 1-20, was rendered and
visually inspected. Its circuit is PB1 AND PIT OUT2; PB0 controls PIT Gate2,
not a speaker bypass. IBM AT March 1986 PDF 38, printed 1-23, was likewise
visually inspected and shows the same AND. The shared Board-Base speaker
owner incorrectly bypassed OUT2 whenever PB0 was low. That condition is
removed, keeping one PIT state, gate path and observation publication.

The missing XT PC5 input requires actual composition wiring, not a mirrored
output flag. Before its edit the executor reported the bounded public need
under the approved Core automatic scope: one new
`core_machine_xt_ppi_keyboard_bind_timer_output()` function, no new public
type. Board-XT retains only the borrowed function/context pair; Board-Base
samples its existing PIT channel 2 through the callback. Read port C applies
the sampled bit 5 through the existing PPI input-direction rules. Binding
does not change PIT output or create a synthetic edge. Reset retains the
physical binding; finalization clears it before PIT destruction. An unbound
fixture keeps its prior disconnected-low model, not selected-board proof.

The existing actual XT board receiver programs PIT2 mode 0 via the installed
ports, obtains low/high OUT through the actual chip, and checks all four
PB0/PB1 combinations against both speaker AND and port-62 PC5. This is direct
wiring proof, not a new timeline test or electrical/acoustic qualification.
The initial new receiver failed before the AND correction. After production
repair the old assertion of forced-high output with Gate2 low also failed;
it was corrected to compare against actual OUT2, preserving the real source
contract. Final PPI, XT board and AT parity/NMI receivers pass 3/3 on both
widths in the existing Ninja/ccache trees.

The five changed source/test paths count +80/-13, net +67 against HEAD,
including prior PPI changes in the XT owner/test. Production is +39/-12;
test is +41/-1. The positive source delta supplies the missing physical input
binding and its unique composer; there is no new state mirror, queue, clock,
manager or Shared/MyNES change. Full S3 units, four-App integrations and
artifact delivery remain outstanding.

### Enhanced Keyboard Parameter Progress (Uncommitted)

KEY-01/02/03: ED and F3 stop scanning while waiting for an option and resume
the prior enable state afterwards (IBM enhanced chapter 4-44/45); F0 returns
to its previous scanning state and clears the typematic key (4-42). A new
keyboard command supersedes an incomplete option. The existing interpreter
instead consumed EE as an LED/rate option and continued input/repeat delivery
through the transaction. The single keyboard owner now derives eligibility
from its existing enable latch and pending parameter. One private predicate
is used for copied signals, ordinary admission, repeat advance and deadline;
no saved-enable mirror, new state or public API/type is added. ED--FF are
dispatched by the existing command switch after cancelling the old option.
F0 clears the existing repeat key and partial encoding through the existing
stream-reset callback. Clearing already queued whole sequences remains part
of the transport/admission reconciliation, not a claim of this fragment.

The direct test covers all three parameter commands in enabled/disabled
contexts, blocked admission/repeat/deadline, option completion, explicit
cancellation and all 19 command-byte replacements. Replacement responses and
signals are compared with the same idle command; this proves cancellation,
not correctness of the still-incomplete F7--FD implementations. The controller
interleaving test now expects EE through both controller personalities and
all 16 intervening controller commands, rather than the erroneous FA/FE
parameter response. Defined F3 values 00--7F remain exhaustive; reserved
80--EC retain the old formula/model checks but do not qualify hardware
semantics. ED--FF are no longer tested as rate/LED options: the replacement
matrix covers their command interpretation instead. The valid eight LED
patterns remain exhaustively covered by the defaults matrix.

Five keyboard/KBC/AUX/serial/actual-board receivers pass x64 and x86 in the
existing persistent Ninja/ccache trees; the final expanded direct replacement
matrix was rebuilt and rerun on both widths. The two keyboard source/test
paths now count +111/-10, net +101 against HEAD, including the prior defaults
fragment. The parameter increment alone is production +16/-4 and direct
test +72/-2. The already-dirty controller test is counted in the final S3
whole-file review. Focused green does not qualify artifacts, full units,
desktop, external integrations or the remaining S3 set-3/BAT/XT/RTC rows.

### Enhanced Keyboard Defaults Progress (Uncommitted)

KEY-01/03: the original IBM March 1986 enhanced-keyboard pages 4-40,
4-43 and 4-44 (PDF 191, 194, 195) were visually reviewed again. F5 disables
scanning; F6 enables scanning; both preserve the existing status lamps.
The existing `keyboard_defaults()` incorrectly cleared lamps for both, and
F6 inherited disabled scanning after F5. The one defaults helper now leaves
lamps alone; full reset still clears them through initialization or the FF
reset branch. F6 explicitly enables the existing scanning state. There is
no new helper, state, public API, type or alternative input path.

Direct tests retain reset/ACK/stream ordering and now prove all eight lamp
combinations survive F5/F6, disabled admission after F5, scanning after F6,
and FF clearing lamps. Actual AT KBC port tests likewise preserve the LED
value programmed by ED, rather than asserting the old incorrect zero.
The two newly changed keyboard source/test paths count +23/-4, net +19
(`git diff --numstat` against HEAD): production +2/-1, tests +21/-3.
The already-dirty board receiver additionally changes three LED expectations
in place (+3/-3 for this defaults fragment); its full S3 delta is counted at
final batch review, not attributed entirely to this fragment.
The three selected targets build in both persistent Ninja/ccache trees.
The selected keyboard/controller/XT/board receivers pass 4/4 per width;
after extending the eight-lamp matrix, the exact keyboard receiver passes
again on both widths. No desktop, external integration, full S3 qualification
or artifact delivery is claimed. Parameter suspension/cancellation,
set-3/key types, complete-sequence admission and selected-personality
reconciliation remain in the same active S3 batch.

### RTC Wildcard Proof Progress (Uncommitted)

The existing three-field alarm comparator correctly recognizes each alarm
byte with both high bits set as don't-care. No production repair is needed
for that predicate. The calendar receiver now covers all eight field masks,
all 64 low-bit variants of the wildcard byte, binary/BCD and 12/24-hour
formats: 2,048 matching contexts. Each unmasked field is independently
changed to a nonmatching value, adding 3,072 negative contexts. The total
5,120 contexts prove AF/IRQ, prediction of the next matching update, no early
IRQ and register-C read-clear behavior. Existing rollover/DSE/phase tests
remain in the same receiver; no new test executable or framework was added.

The test adds 62 lines relative to the preceding fragment (the complete
calendar-file S3 delta is +222/-3). Its first x64 compile rejected an
incorrect memory-helper name; the unnecessary call was removed in favour
of a zero-initialized fixture before successful qualification. Final focused
calendar execution passes x64 1/1 (0.62 s) and x86 1/1 (0.12 s). This is
chip proof, not selected-board qualification. Current Board tests include
a separately assembled RTC/PIC route and a port/NMI-mask receiver; their
existence alone does not prove the selected Board clock/deadline route.
That actual receiving route still requires reconciliation before S3 closes.

### RTC Actual Board Clock Route (Uncommitted)

The existing Board authority receiver now constructs the real plan and RTC
topology, freezes/reset the actual Board and programs 70h/71h. It exercises
32,768-Hz RTC conversion against 1, 8 and 16 MHz macro axes. The latter two
are mathematically the selected AT/DeskPro RTC ratios; this does not assign
physical L3 eligibility to the CPU macro axis or qualify their complete App
composition. The source declaration for the 8/65 RTC phases stays distinct
from the macro-axis conversion claim.

Each ratio proves an absolute one-second alarm deadline, no calendar update
or IRQ before the final source tick, the remaining one-tick deadline, actual
IRQ8 through the cascaded PIC with vector 70h, C-register AF/UF/IRQF and
read-clear, slave/master EOI, and RESET retaining calendar/CMOS while clearing
delivery. It adds +66/-0 test lines; no source/API/state/helper or test target
was added. Existing DMA/refresh and synthetic RTC cases remain intact.

The first candidate used direct create, omitting timing declarations, and
correctly received no admitted Board deadline. Diagnostics identified that
fixture mistake; it now uses the production create-from-plan path rather
than changing the eligibility gate or writing private state. Final RTC
calendar/contract/actual-Board executions pass x64 3/3 (3.21 s), x86 3/3
(0.99 s). Test manifest, whitespace and governance checks follow this update.
Selected-App integration, the default unqualified-clock disposition and
remaining source boundaries still belong to final S3 reconciliation; focused
results do not imply artifact or whole-batch qualification.

### XT Repeat Progress (Uncommitted)

XT repeat capability is required for all 83 keys by the IBM March 1986 XT
reference, chapter 4-4 (PDF 100). That page does not supply exact repeat
durations. The existing timing input therefore gains paired delay/period
fields; both zero disable repeat, otherwise both must be positive and serial
timing must be enabled. This necessary Core-only public struct expansion was
reported before editing. Board-Base supplies nominal 500 ms/100 ms L2 inputs,
not the enhanced-keyboard page's 10.9-cps rate or a claimed exact 83-key rate.

The keyboard owns the last admitted one-byte set-1 make and its remaining
repeat duration. Repeats enter the existing 16-byte FIFO and nine-bit serial
route. A matching break or reset clears the repeat selection. Clock/clear
inhibit, full FIFO and refused completion suspend repeat without catch-up.
Last-key selection and those suspension choices are explicitly L2 model
boundaries, not newly sourced 83-key MCU rules. Multibyte extensions remain
transportable but are not qualified by the 83-key repeat model.

The direct receiver covers every make/break code 01--53, inhibition and
refused completion, last-key replacement, nonmatching break and invalid
timing pairs. The actual Board receiver uses its single guest-time clock:
the nominal repeat deadline queues a scan, the first edge follows 60 ticks
later, and only the completed 260-tick frame reaches PPI latch/IRQ1. Matching
break cancels subsequent repeats. No Board scheduler, second FIFO, public
function, worker or mirrored IRQ state was added.

Both focused receivers build and pass once on each width in the persistent
Ninja/ccache trees: x64 2/2 (1.12 s), x86 2/2 (0.25 s). The Board test
increment is +55/-0; final whole-S3 counts include its previous PPI/BAT
changes. These passes do not qualify full units, external integrations,
desktop behavior or rebuilt artifacts. Whole-sequence keyboard transport,
set-3, RTC reconciliation and the remaining KBC source boundaries remain
in the active S3 batch.

### Enhanced Set-3 Table And Transport Reconciliation (Uncommitted)

The IBM March 1986 AT original was rehashed with the recorded SHA-256.
Printed 4-43/44 and 4-58--60 (PDF 194/195 and 209--211) were visually
reviewed, including table columns and layout-specific footnotes. The set-3
table contains 103 distinct make codes across both layouts: 59 labelled
Typematic, five Make/Break and 39 Make only. Code 5Ch belongs only to the
101-key layout; 53h and 13h belong only to the 102-key layout. This is not
a 103-key physical keyboard. A selected-layout proof must exclude its
other layout's entries rather than inventing a second physical key.

The finite source sets below preserve the default-type oracle for the
existing endpoint receiver. Codes are hexadecimal; they are source facts,
not a passing implementation matrix or a new production catalogue.

```text
Typematic:
0E 16 1E 26 25 2E 36 3D 3E 46 45 4E 55 66 0D 15 1D 24
2D 2C 35 3C 43 44 4D 54 5B 5C 1C 1B 23 2B 34 33 3B 42
4B 4C 52 53 13 5A 1A 22 21 2A 32 31 3A 41 49 4A 29 64
61 63 60 6A 7C
Make/Break:
14 12 59 11 19
Make only:
39 58 67 6E 65 6F 6D 76 6C 6B 69 77 75 73 72 70 7E 7D
74 7A 71 84 79 08 07 0F 17 1F 27 2F 37 3F 47 4F 56 5E
57 5F 62
```

The IBM table gives the possible F0-prefixed break encoding even for keys
whose default type is Make only. The endpoint therefore needs a key-type
decision before publishing any part of a release sequence; the mapper must
not duplicate that decision. The 7Dh entry for key 101 is visually confirmed;
OCR must not substitute 70h. Physical key-number mapping still requires the
source's layout/set-1 tables, not inference from the set-2 translation table.

IBM 4-43 says each identified key is configured by FB/FC/FD, but does not
explicitly specify how a list of identifications ends or when scanning
resumes. Compaq Table 5-10 expressly calls each a two-byte command and says
scanning is disabled after the command. This is a source-boundary question,
not proof of an indefinitely active parameter state or automatic resume.
The existing one-option transaction model cannot be called exact for both
sources without a recorded selected-personality disposition. In particular,
the current AB83 identity must not silently qualify an original 84-key MCU.

The actual receiving chain remains KVM event -> Machine mapper -> Board
native-sequence receiver -> AT adapter -> KBC backlog -> controller output
and IRQ1. `controller.h` allocates **64** native backlog bytes, distinct from
the enhanced keyboard manual's 16-byte output buffer. Whole-array admission
prechecks that capacity, then calls the endpoint byte by byte; no whole-key
identity or accepted-versus-suppressed byte count crosses that boundary.
Single-byte injection is also a live public Board/adapter path, not merely
a discarded test helper. A replacement must account for its prefix callers;
silently deleting them or leaving a second endpoint interpreter is excluded.

`x86_kbc8042_publish_native_byte()` already invokes set-2 translation only
when the endpoint reports set 2. It otherwise publishes native bytes. No
additional set-3 bypass flag/helper is necessary. Compaq's instruction that
software should disable controller translation for mode 3 is not evidence
for introducing another controller policy state. The stream-reset callback
currently clears only partial translation, not the native backlog or the
already host-visible OBF byte; those are separate buffer ownership boundaries.

This reconciliation changes the next implementation constraints, not the
qualification result. No production/public ABI, test or artifact was changed
by this source review. KEY-01--04/KBC-05 remain pending together; their next
repair must report the necessary bounded-sequence signature and caller
migration before editing, retain one repeat/publication owner, and prove
default/all/per-key types, suppressed releases and atomic capacity failure.

### Enhanced Set-3 Machine Encoding Progress (Uncommitted)

The original IBM set-1 tables, printed 4-50--52 (PDF 201--203), were rendered
and visually reviewed against the previously reviewed set-3 tables. The
Machine mapper now joins physical key identities rather than treating every
non-set-1 selection as set 2. Its stateless set-3 conversion distinguishes
the navigation and numeric clusters, both Ctrl/Alt keys, keypad slash/Enter,
Print Screen and Pause. Set-3 uses one make byte and F0 plus that byte for
the possible break; the endpoint, not this mapper, will apply key-type break
suppression. It introduces no keyboard state or repeat/queue owner.

The retained model is the 101-key layout. The 102-key-only position 56h is
explicitly unsupported, not assigned an invented US-layout code. Unknown
scan sets return UNSUPPORTED with an empty sequence instead of silently
falling through to set 2. Print Screen/Pause key identities are recognized
independently of native metadata; unsupported extended ordinary keys are
rejected. Existing set-1/set-2 conversion is retained unchanged.

The existing receiver contains 101 source-ordered physical-key rows, with
KVM scan codes and extended flags supplied separately. Both transitions
for each key give 202 conversion contexts. It also tests all 253 unsupported
scan-set values, absent native Print Screen metadata and unsupported layout/
extended positions. This proves only stateless encoding, not endpoint key
types, serial emission, IRQ delivery or actual selected-App behavior.

Before editing, the necessary existing Core enum append SCAN_SET_3 = 3 was
reported and included in Current's file/ABI surface. No new function/type,
changed existing enum value, Shared API, target or test framework was added.
The endpoint still rejects F0=3: it must not expose the third set before
the cohesive key-type/admission/transport repair is qualified. That temporary
remaining gap stays explicitly open in the same S3, not a shipped fallback.

Actual tracked code/test delta for these three files is +146/-2, net +144:
mapper +61/-0, existing Board interface +2/-1, existing test +83/-1. The
first edit used two nonexistent test vocabulary names; review corrected
them to the existing absent-key value and anonymous copied event before
compilation. Both persistent Ninja/ccache trees build the existing mapper
target. Focused execution passes once per width: x64 1/1 (0.15 s), x86 1/1
(0.13 s). Manifests are updated. Full S3 units, selected-App integration,
desktop acceptance and rebuilt artifacts remain pending.

### Enhanced Whole-Sequence Admission Progress (Uncommitted)

The previously reported existing Core admit/link signatures now accept a
bounded byte array and prepare transport output/count. The endpoint validates
against a local candidate and publishes its state only after the complete
submission fits and is valid. Failure returns the original status, zero output
count and unchanged prefix/repeat state. The KBC byte entry delegates to the
same sequence path; the former per-byte admission loop is removed. No Shared
contract, new public operation/type, queue or worker is introduced.

Native Set-1/Set-2 repeat tracking retains the complete make identity, including
E0; an unextended release cannot cancel its extended counterpart. E1 Pause
is validated as a bounded sequence without arming repeat, and Print Screen's
extended make is not typematic. Repeated invalid prefixes reject the complete
submitted suffix without consuming a prefix accepted by an earlier call.
The bounded prefix and single repeat packet are endpoint-owned decoder state,
not another transport backlog.

The repeat sink now returns a real status for a whole packet. Only accepted
packets count as delivery; failure retains one due repeat, which can be retried
with zero elapsed ticks. A zero computed period cancels repeat, including a
typematic option that rounds small configured inputs to zero. One KBC predicate
governs both advancing repeat and advertising its deadline: scanning, line
eligibility and room for the complete packet. Clock inhibition/full backlog
freeze the timer without manufacturing catch-up output. This is an explicit
L2 transport model, not a newly proven original keyboard MCU timing rule.

Chip tests prove both sets' extended make/repeat/break identity, capacity and
malformed-stream failure atomicity, Pause, due-delivery retry and zero-period
cancellation. KBC proof observes the native bytes and IRQ1 through actual OBF,
line inhibition, full serial backlog, rejected break, later accepted break and
the absence of partial/extra output. The 64-byte KBC backlog is retained as its
existing model capacity; it is not equated to the manual's keyboard buffer.

The seven chip/Board/host-ingress/AUX routes pass x64 7/7 (4.30 s), x86 7/7
(0.67 s), in the original persistent Ninja/ccache directories. The subsequent
zero-rounded-period assertion passes the keyboard route x64/x86 separately.
The initial Board failure was a fixture injecting three isolated E0 prefixes,
not a valid key stream. It now uses complete non-typematic Ctrl makes while
retaining the original full-OBF/typeahead/break-cancellation/reply-order proof.

All six users of the migrated Board state fixture were then rebuilt per width.
Topology, default composition and 5170 clocks passed, while 5170 composition
and two Model40 routes initially failed on old no-AUX bit-5/speaker assertions.
These are reconciled against the already reviewed source dispositions above:
original AT bit 5 is PC interface mode, not synthetic AUX-disable; PB1 enables
the speaker AND with PIT2 OUT even when PB0 is low. Four App test files replace
only those assertions. Speaker output is compared to the actual PIT output,
not to an invented always-high level; live PIT transitions/reset checks remain.
The three corrected routes plus D4 speaker/platform pass x64 4/4 (2.85 s),
x86 4/4 (0.43 s). No receiving App production change was needed. Together these
are fourteen distinct passing focused routes per width, not a full S3 suite.

The similar-issue sweep covers all Core and four PC App source/test callers of
`x86_keyboard_admit`, keyboard advance/link callbacks, native enqueue and the
old no-AUX/speaker assertions. Every production caller uses the single bounded
sequence path; test-only one-byte setup helpers invoke it without a second
interpreter. Remaining `0x20,0x20` port writes are PIC EOI, not KBC assertions.
Six production files and four receiver/fixture files reviewed here cumulatively
differ from S2 by +610/-163, net +447; the four corrected App test files add
8/remove 8, giving +618/-171, net +447 for these fourteen paths. This includes
their earlier uncommitted S3 corrections, not a falsely isolated turn delta.
Production is +207/-125, net +82; most growth is explicit failure/sequence proof.
Actual-diff review retains one endpoint state owner and one KBC transport owner.

F0=3 is still disabled. KEY-01--04/KBC-05 remain pending for cohesive Set-3
default/all/per-key behavior and command output clearing. RTC board source
reconciliation, KBC initialization and XT buffer/reset dispositions are also
pending. Full dual-width units, selected-App integrations, desktop acceptance,
eight S3 artifacts and complete-P delivery have not run. Shared-six, MyNES,
owner INIs/media/snapshots and deployed S2 EXEs remain unchanged.

### Keyboard Command Output Ownership Progress (Uncommitted)

Original IBM enhanced-keyboard pages 4-40--43 distinguish F4 enabling from
F6 defaults: F4 clears output and the last typematic key before scanning;
F6 continues scanning. The rendered 4-43 page was reviewed again. F6's current
enable behavior is retained, not changed to preserve a prior disabled state.
The F7/FB/FC/FD ACK-only implementation and absent F8/F9/FA remain confirmed
unfinished key-type behavior; these are not accepted merely by command ACK.
The IBM/Compaq per-key transaction boundary recorded above remains unresolved.

The existing stream-reset callback now discards unsent native keyboard backlog,
serial remaining time and partial translation, but never retracts the controller
output FIFO, delayed replies, IRQ ownership or BAT. Current records that semantic
correction before source edits. F4 also clears the endpoint's partial prefix and
uses this same callback before its ACK. F0/F5/F6/FF keep the existing callback
path; FF still acknowledges before reinitialization. No function, type, state,
queue, scheduler or second output path was added. Native backlog capacity remains
the explicitly modeled 64 bytes, not a claim of the manual's 16-byte capacity.

The existing keyboard receiver now proves F4 discards E0/F0 decoder prefixes,
clears repeat and resets transport before ACK. The existing KBC receiver adds
eight F0/F4/F5/F6 contexts across immediate and timed serial delivery: a prior
controller-owned output byte remains readable, the unsent native suffix is gone,
ACK still reaches OBF/IRQ1, and later advancement cannot deliver stale bytes.
A translated F0-prefix context independently proves F4 clears both endpoint
and controller partial interpretation; the next make is 1Eh, not a spurious
9Eh break. These contexts exercise the real admission/publication path, not
only reset counters. Initial bind/queue ownership is unchanged.

Incremental reviewed delta for this step is production +9/-1 (net +8) across
four existing files, and tests +70/-0 across two existing files. The one removed
line is the superseded stream-reset comment, not an algorithm or caller. This
is separate from the cumulative S2-to-S3 deltas reported above. The only helper
additions are two test scenario functions; production uses the existing owners.
The caller sweep finds exactly one production binding, Board AT's existing
keyboard stream-reset callback, and one equivalent chip-test binding. Neither
flushes host-visible/controller response ownership.

Persistent x64/x86 Ninja trees rebuilt the thirteen relevant existing targets.
Keyboard/KBC/Board contract routes pass 3/3 per width (x64 1.15 s, x86 0.31 s).
Ten additional serial/AUX/host-ingress/topology/default/5170/Model40/clock routes
pass 10/10 per width (x64 7.97 s, x86 0.98 s). An initial build target spelling
`vm-d4-platform-smoke` was rejected before compilation; it was removed from
this transient selection, not counted as a test or qualifying build. Core
source/test manifest and corpus checks pass. Shared-six, MyNES, deployed EXEs
and owner INIs/media/snapshots are unchanged. Full S3 units, selected external
integration, desktop validation, eight artifacts and complete-P delivery remain
pending. This step is not S3 or whole keyboard qualification.

### Enhanced Key-Type Implementation Disposition (Before Edits)

Coordinator disposition for the existing AB83, 101-key endpoint: use the IBM
default key-type/code tables and any-scan-set command acceptance, with effects
confined to Set 3. FB/FC/FD use Compaq Table 5-10's explicit two-byte transaction;
the option is a valid selected-layout Set-3 make code, after which scanning
remains disabled until F4. IBM does not specify list termination here, so this
combined endpoint policy is explicitly modeled where that source is incomplete,
not an exact original IBM 84-key MCU or a second board-specific personality.
No timing accuracy is downgraded. The source difference is retained, not erased
by a successful test, and no indefinite identification-list state is invented.

One private key-type array will hold the real per-key programmable state. The
existing bounded admission operation will withhold Set-3 F0 until the following
key identifies whether the release is allowed; matching release still cancels
repeat even when no break is emitted. Complete output capacity and decoder/key
state commit remain atomic. Default/all/per-key commands will clear the existing
native backlog through its one callback. Native Sets 1/2 retain their decoder.
F0=3 is enabled only with this endpoint/filter implementation and its receiving
tests in this same uncommitted S3 batch. Estimated implementation +110--150/
-20--35 and tests +180--260; no new public API/type, output queue or interpreter.

Source/caller/test inspection, RTC/PPI/IBM/Compaq archive hashing and the
listed original scan reviews have run. S2's passing suites do not qualify
the identified S3 defects. PPI production/test progress is recorded above;
no complete S3 suite, artifact rebuild, desktop or external receiving test
is claimed. RTC final reconciliation, enhanced keyboard/KBC and XT buffer/reset
source disposition remain pending; their full source boundaries are not reduced
by the focused PPI results.

### Enhanced Key-Type Receiving Progress

The admitted Set-3 implementation is now present in this uncommitted S3 batch.
One private, selected-layout key-type table is the programmable state; the
existing admission operation filters releases and owns repeat cancellation.
Defaults, F7--FA and the selected Compaq FB--FD transaction use the same
existing stream-clear callback. Sets 1/2 retain their original interpretation.
No public function/type, new output queue or second transport was added.
Invalid per-key options return FE and retain the pending option; that retry
policy is a declared endpoint model, not a newly proven original MCU detail.

The independent test oracle covers 101 keys under defaults and four all-key
settings (505 contexts), then three per-key settings (303 contexts). It also
covers interrupted transactions, legacy-set command acceptance, reset/query,
malformed prefixes and failure-atomic capacity. The KBC receiver verifies
suppressed releases with a full native backlog, matching repeat cancellation
and complete two-byte break admission. The Machine receiver sends actual
neutral KVM events through the existing mapper, keyboard endpoint, KBC output,
PIC IRQ1/vector 9 and guest port-60 read; 40 event contexts cover normal,
extended/modifier, Pause and Print keys under defaults and FA.

Persistent x64/x86 builds and the four keyboard/KBC/mapper/Board routes pass.
The additional KBC full-backlog receiver passes on both widths (0.14/0.12 s).
The first mapper build failed because its test assertion header was omitted;
adding the existing Lib Types test header corrected compilation before the
successful execution. No failed attempt is counted as passing proof.

The earlier "F0=3 disabled" paragraphs describe prior intermediate states,
not this receiving result. RTC selected-board reconciliation, KBC
initialization/diagnostic/pulse/BAT and XT buffer/reset dispositions remain
pending. Full S3 units, external/desktop validation, eight rebuilt artifacts,
actual-diff acceptance and complete-P delivery are not yet claimed. Shared-six,
MyNES, deployed EXEs and owner INIs/media/snapshots remain unchanged.

### Remaining KBC Initialization/Diagnostic/Pulse Recheck

The archived IBM March 1986 original still hashes to
`3224C26B424A4745C765F56AF7D957D3C4AD959E0F719EBEB39540A2508E95D5`.
PDF pages 57, 72 and 73 (printed 1-42, 1-53 and 1-54) were rendered and
visually inspected. An initial guessed page 61 instead showed the 102-key
layout, and was not used as initialization evidence.

The initialization paragraph describes inhibition followed by parameter
setup, inhibit override, successful output and released lines. It does not
justify the current production comment's unconditional assertion that the
completed AA operation remains inhibited until a later host enable. The
existing implementation does remain inhibited, while its cold reset starts
with IRQ/translation enabled and SYS clear. Existing chip and Board receivers
explicitly rely on these states. Before changing that owner, reconcile the
selected firmware's actual AA/command-byte/AE sequence and distinguish the
documented initialization stages from the retained completed-command model.
This is a source-contract candidate, not a completed correction or an excuse
to preserve a BIOS-specific branch. KBC-01 remains unresolved.

AC expressly requires sixteen internal RAM bytes, input/output ports and PSW
in scan-code format. The current controller model has neither the MCU RAM/PSW
nor that diagnostic encoder, and its dispatch currently has no AC case.
Dumping zero-filled invented MCU state would not satisfy this command. Record
the exact firmware/source prerequisite for its model-boundary disposition;
the absence must remain visible rather than being counted as a supported ACK.

F0--FF selects low pulses on output bits 0--3, nominally about six microseconds.
The existing implementation calls its reset receiver only for bit 0 selected
low and never changes the persistent D1 output latch. Board AT observes reset
and A20 through distinct existing callbacks. Bit-1 transient A20, the remaining
electrical pins and pulse duration are not represented by the current model.
The next disposition must account for those actual board consumers and all
sixteen command selections; the single FE test cannot qualify that matrix.
No pulse timer, public API, state or production behavior was changed by this
recheck. KBC-03 remains open, not silently marked complete.

After manifest reconciliation, Core production/test manifest, corpus and
NXVM documentation gates pass. A fresh execution of the four existing
keyboard/KBC/mapper/Board receivers passes 4/4 per width (x64 0.19 s, x86
0.25 s). These results qualify only their stated receiving contexts, not the
unresolved initialization/diagnostic/pulse class or the entire S3 batch.

### XT Reset Queue Disposition (Before Edits)

The selected original 83-key endpoint retains its sourced sixteen-code
capacity and all-key make/break/typematic capability. The threshold/BAT/edge
durations remain the already declared board-supplied L2 model; the later
101/102-key reset description is supporting reset-interface evidence, not
proof of an original 8048 firmware image. Its two overflow descriptions also
do not establish the selected original endpoint's exact overflow slot.
Retain the existing last-slot-FF policy as explicitly modeled, and remove
the false production comment claiming the selected reference requires it.

The admitted reset-state contract does require clearing pre-reset native
input. Current line-triggered BAT cancels serial/repeat state but retains
FIFO contents, so it can replay old makes after BAT. Reuse the existing
chip reset operation before starting BAT; it already owns all queue, partial
frame, pending response and repeat cleanup. No new helper, state or public
API is needed. Add threshold-minus-one versus exact-threshold receivers with
partial/refused completion, verify only BAT then fresh input reaches the
receiver, and retain the separately tested receiver/backpressure behavior.
Estimated incremental production +3/-6 and tests +40--65/-0. This correction
does not turn the retained 83-key reset timing or overflow policy into L3.

### XT Reset Queue Receiving Progress

Line-triggered reset now invokes the existing chip reset before scheduling
BAT. FIFO, partial/refused serial frame, pending BAT response and repeat all
remain owned by that one reset implementation. The incremental production
change is +3/-6 (net -3): one reset invocation and the corrected two-line
overflow-source comment replace three duplicated state clears and the old
three-line provenance assertion. No public API, field or helper was added.

The existing receiver adds four contexts: partial versus refused-completed
serial delivery, each below versus at the reset threshold. Below threshold,
the two queued codes survive ordinary clock inhibition. At threshold, only
BAT is delivered, no stale queue/repeat is left, and fresh input delivers
normally. Its first execution failed because the new test omitted the
existing receiver-ready handshake between ordinary bytes. The fixture was
corrected to obey that production contract; no production workaround was
added. Final chip and actual XT Board receivers pass 2/2 on x64 (0.22 s) and
x86 (0.27 s), built in the original persistent Ninja/ccache trees with jobs 8.
The new test function and main call add 48 lines; it is a bounded scenario,
not a new framework or public fixture. Whole S3 verification/artifacts remain
pending, as do the KBC initialization/diagnostic/pulse and final RTC board
dispositions. Shared-six and MyNES were not modified.

### DeskPro RTC Original Board Source Reconciliation

On 2026-10-11 the missing original COMPAQ DESKPRO 386 Technical Reference
Guide, Volume I, First Edition (September 1986), was acquired from the
[PCjs archive scan](https://archive.org/details/compaq-desk-pro-386-technical-reference-guide-vol-1-1986-09).
The external archive copy is `manuals-nxvm/platform/compaq-deskpro-386-technical-reference-vol1-sep1986.pdf`;
SHA-256 is `A307D26E29DAC2429806BC372CC6D5B5C76410C3F5BEC3B406F98862442132A1`.
The visually checked notice page identifies assembly 108037-001, Volume I
subassembly 108038-001 and text 108040-001. OCR misreads those identifiers as
100xxx; the scan, not the OCR, supplies the recorded edition identifiers.
The Bitsavers DeskPro386 subdirectory inspected first contains a BIOS ZIP,
not this manual, and was not used or downloaded as hardware evidence.

Printed pages 2-53, 2-65--2-67 and 4-41 (PDF pages 93, 105--107 and 243)
were rendered and visually inspected. They identify MC146818, its 64-byte
register/RAM map, read-only C/D and A bit 7, and unavailable seconds bit 7.
Ports 70h/71h are index/data; index bit 7 masks NMI, while six low bits select
RTC storage. Table 2-22 identifies checksum 2Eh/2Fh, century 32h and the
ordinary configuration locations. The RTC interrupt uses IRQ8/INT70h;
the documented BIOS periodic service is 1024 Hz. These facts reconcile the
existing selected DeskPro topology and single Board-base RTC/NMI/PIC route.

This reading does not independently establish an oscillator crystal value,
the checksum algorithm, electrical bus recovery implementation or a physical
power-loss model. Chip documentation, selected firmware/configuration evidence
and declared model boundaries still own those separate claims. In particular,
the manual warns against word I/O recovery timing; the logical port adapter
does not thereby become an electrical bus model. No production behavior was
changed merely to match an OCR statement or a BIOS service rate.

Current-source inspection also confirms the Board receiver already includes
1 MHz, IBM 8 MHz and DeskPro 16 MHz source-axis contexts; no redundant 16 MHz
test was added. Its one-second alarm test is not itself proof of the 1024 Hz
periodic BIOS route. RTC-13--RTC-16 require the remaining source-to-receiver
mapping before final acceptance. Original scans remain external, ignored
extracts/renderings stay in `build/t555-manual-review`, and no firmware,
Shared-six, MyNES or product artifact was changed by this source acquisition.

### Newly Located DeskPro KBC Source Differences

Volume I also supplies the previously missing controller command table:
printed 2-83/2-84, PDF 123/124, visually checked after OCR location.
AA explicitly disables the keyboard, clears buffer pointers and returns 55h.
This supports the retained completed-command inhibit state for DeskPro; it
does not settle IBM's differently worded initialization paragraph. AC is
reserved for diagnostics here, without IBM's specified RAM/PSW dump format.
Commands A3/A4/A5 are Compaq-specific speed/read functions, not generic AUX
commands or evidence for inventing an IBM firmware response.

The pulse duration is explicitly approximately 2 microseconds, versus
approximately 6 microseconds in the reviewed IBM source. This is a real
selected-firmware distinction: do not hard-code either duration into the
generic controller and then claim both boards qualified. The source also
reserves speed bits against ordinary D1 writes. KBC-01/KBC-03 therefore need
selected-firmware disposition and an existing board-owned timing/input route
before any further production change. No universal AA policy, fabricated AC
dump, pulse timer/API or Compaq speed response was added by this inspection.
These findings refine the original-manual batch rather than hiding the
unimplemented firmware personality behind successful boot tests.

### RTC Periodic Board Receiver Completion

The existing `core_machine_rtc_clock_deadline_route()` now additionally
programs DV010/RS6 and PIE after its reset-retention check. For each existing
1 MHz, 8 MHz and 16 MHz source-axis context it asserts the rounded-up
source-tick deadline for a 1024 Hz event, no IRQ immediately before that
deadline, IRQ8/INT70h on completion, and exact PF/IRQF read-C acknowledgement.
The integer rounding is the existing rational clock contract, not a claim
that the host or CPU physically executes at that frequency.

The first execution failed on both widths: the new section followed a
whole-machine reset without reinitializing PIC, then incorrectly expected
the pre-reset vector. Reusing the existing PIC initialization before the
periodic section corrected the receiver. No production workaround or
additional state was introduced. Final rebuilt receiver passes 1/1 per
width: x64 2.00 s, x86 1.05 s. The original persistent Ninja/ccache trees and
jobs 8 were used; the one persistent command handle reached terminal exit 0.
Incremental test delta is +19/-0; this file's complete S3 delta is +85/-0.
The test manifest was refreshed. This closes the missing direct periodic
route proof, not KBC's remaining source/implementation dispositions or
whole S3 qualification. No desktop or external integration was run here.

### KBC Output Pulse Implementation And Receiving Proof

Before edits the coordinator reported the complete F0--FF selection defect,
the actual A20/reset receivers, proposed source/test/profile surface and
estimated production +65--95/-15--25, tests +70--110/-5--10. The active
packet now records the necessary Core-only surface: chip/board pulse timing
setters, board/profile service-tick input and removal of the redundant chip
link reset callback. No Shared API or additional output path is introduced.

One private pulse mask/remaining duration lives beside the existing output
latch. D1 still owns that persistent latch; its existing publication helper
applies the transient mask before calling the single output sink. Advance
restores the latch once at expiry; the existing deadline query includes the
pulse alongside replies/serial/repeat. Its serial minimum was kept as a
minimum rather than overwriting an earlier pulse deadline. Reset cancels
the transient and preserves configured timing. Board AT already consumes
output bit 0 as reset and bit 1 as A20, so the old reset-only callback was
removed, not retained as a duplicate side effect.

The immutable 5170 profile supplies 48 service ticks (nominal IBM 6 us on
its 8 MHz macro axis); DeskPro supplies 32 (Compaq 2 us on its 16 MHz macro
axis). Those source-duration conversions remain L2 because the execution
axis itself is not physical L3. The default profile's uncalibrated zero
selects synchronous low/restore ordering, explicitly L1. No pulse duration
is guessed from keyboard serial timing or command reply timing.

The unchanged D0 read contract returns the persistent latch during a pulse;
the selected firmware's possible overlapping read sampling is not claimed
electrically qualified. Overlapping pulse commands merge active low bits and
reload the selected duration; FF is a no-op. These bounded command-overlap
semantics are a declared logical model, not inferred original MCU code.
Unconnected bits 2/3 are delivered by the same output sink. Subsequent visual
review of Compaq Figure 2-15, recorded below, identifies those two bits as
unused, not speed/PIT wiring. Compaq's separate speed-control class remains
an unresolved selected-board receiver, not a reason to assign those bits.

The existing chip receiver adds 128 single-pulse contexts: all sixteen
commands, four latch patterns, and synchronous/timed modes. It verifies
selected low output, preserved latch, deadline-minus-one, restoration and
reset indications. Additional contexts cover D1 during a pulse, overlapping
commands/FF and reset cancellation/configuration retention. The actual Board
receiver verifies A20 low for 31 ticks and release on tick 32 without a reset.
Three existing App receivers additionally require configured values 48/32/0.

Both widths build in the original Ninja/ccache trees at jobs 8. Chip/Board
receivers pass 2/2 per width (x64 1.12 s, x86 0.22 s); selected App plan/
composition receivers pass 3/3 (1.13 s, 0.30 s). After adding the overlap
context, the chip receiver passes again (0.18 s/0.12 s). These are focused
receivers, not complete units, firmware boot or desktop acceptance. Manifests
are refreshed; artifacts and the complete S3 actual-diff/qualification remain
pending. No partial P is formed, and Shared-six/MyNES/owner assets stay intact.

### Controller Initialization Stream Boundary

Current dispatch inspection found that AA cleared output/serial queues but
retained the controller's pending 60/D1/D4 data destination and partially
translated F0/E1 state. Compaq Volume I printed 2-83 explicitly calls this
controller initialization with cleared buffer pointers. Retaining a prior
controller transaction across that boundary is inconsistent with the selected
logical initialization model. The source does not supply the MCU's exact
internal pointer layout; this repair does not qualify that layout, IBM's
completed inhibit policy or AC's RAM/PSW dump.

The existing output flush now calls the existing controller stream-reset
owner instead of duplicating three serial fields. AA clears its own pending
data destination. It does not reset the independent keyboard, cancel an
endpoint parameter or change command-byte policy. Production changes for this
correction are +2/-3, net -1; the existing controller receiver is +49/-0,
net +49. No interface, state field, helper or alternate reply path is added.

The twelve contexts cover three old controller data destinations, F0/E1
translation fragments and both existing AUX-capability settings. They assert
the controller state immediately after AA, exact 55h and subsequent EEh
delivery, then actual translated A make after release. The independent
keyboard is explicitly sent F4 before new input: AA does not own its partially
admitted Pause sequence. The first runs omitted initial BAT consumption and
endpoint preparation, and failed both widths. Adding those real preparation
transactions fixes the fixture without changing expected controller results.
The final chip and Board receivers pass 2/2 per width, x64 0.18 s and x86
0.15 s. These are not complete S3 or firmware qualification results.

### Complete Repository-Only Regression Checkpoint

After the initialization correction and manifest refresh, the complete
`build-unit-tests` dependency target finishes successfully in both original
Release Ninja/ccache trees. The existing `run-unit-tests` aggregate then runs
sequentially: x86 passes 546/546 in 75.83 s; x64 passes 546/546 in 250.94 s.
Each aggregate exits zero within the unchanged 300-second containment. The
registered automated desktop-labelled unit routes are included (3 per width);
no owner-interactive desktop acceptance is claimed. The underlying CTest
terminal records are in each tree's `Testing/Temporary/LastTest.log`.

Core production/test manifests, Core corpus, NXVM documentation governance and
`git diff --check` also pass. The existing Model40 CRLF-to-LF Git warning is
not a whitespace error. No Shared-six, MyNES or deployed asset diff exists.
This checkpoint proves current repository-only regressions, not remaining
source/model dispositions, external four-machine integration, refreshed eight
PC artifacts or complete S3 acceptance. S3 remains uncommitted and active.

### Compaq Speed-Control Class: Owner And Required Receiver

Current-source reconciliation uses Volume I printed 2-35--2-36, 2-58,
2-79 and 2-83--2-84. Figure 2-15 (PDF page 119) was rendered and visually
inspected: output-port bits 2/3 are explicitly unused. They must not be
repurposed as SLOWD or speed-selection pins. The previously possible
output-pin interpretation is rejected; no such connection was implemented.

The command table explicitly defines A3 enable, A4 toggle between COMMON
and the switch-selected mode, and A5 special read without setting OBF.
It refers to A1--A6 but does not give sufficient individual semantics for
A1/A2/A6. Those commands require another original authority before coding;
the BIOS function numbers are not interchangeable controller opcodes.
The current `x86_kbc8042_write_command()` handles none of A3/A4/A5, so this
is a real selected-firmware capability gap, not an electrical-only exclusion.

The board text describes a second-PIT counter-2 mode-1 one-shot, refreshed
from the primary PIT's refresh request, with a 4 MHz input and SLOWD enable.
Its output extends CPU refresh HOLD; ordinary DMA must still receive bus
service during the extension. Current `d4_platform.c` binds primary
counter-1 refresh and auxiliary failsafe only; its refresh completion clears
one pending request. There is no SLOWD gate, auxiliary counter-2 refresh
trigger, or speed-dependent HOLD extension. Current Board AT's KBC output
receiver only implements reset/A20. Together these facts identify one
cross-device missing class rather than three unrelated local patches.

| Member | Current disposition | Required owner and proof |
| --- | --- | --- |
| A3/A4 selected speed control | Confirmed missing; not accepted by S3 chip tests or boot. | S6: selected controller-firmware dispatch through one existing board composition boundary; no generic Compaq name switch or guessed output bits. |
| A5 special read | Confirmed missing; normal D0/OBF replies do not implement it. | S6: selected firmware read contract, exact bit-4/5 meaning, unchanged OBF/IRQ, actual guest port-60 read and cross-profile non-interference. |
| Auxiliary counter-2 refresh trigger and HOLD extension | Confirmed missing downstream wiring. | S6: Model40 D4 owns refresh/enable wiring; PIT owns mode-1 state, Core owns time/bus service. Prove refresh trigger, 4 MHz ratio, HOLD release, intervening DMA and reset. |
| A1/A2/A6 and default switch selection | Original command definitions remain insufficient here. | S6: acquire or reconcile exact original selected-firmware authority before implementation; report unresolved prerequisite rather than fabricate a response. |

Coordinator transfers this complete class from S3's KBC-03/BOARD-02
disposition to the existing S6 board/reset/deadline receiver. S3 still owes
its complete finite row review and receiving integration/artifacts. T555
cannot close with the confirmed members merely declared L1/L2, replaced by
host SPEED, or hidden behind successful boot. The transfer does not retire
a machine capability, change a timing grade, add a Shared dependency, or
approve an unspecified public interface. Any required interface is reported
before implementation under the active packet's stop conditions.

### Finite S3 Row Reconciliation Before Receiving Qualification

This matrix supersedes the initial inspection status, not the chronological
source and failed-fixture records above. `Proven` means the named logical
contract has direct source-conditioned receivers in the completed dual-width
546-unit checkpoint; it does not qualify electrical timing or firmware MCU
internals. `Boundary` retains the stated non-eligible model, not an invented
hardware fact. `S6` is still incomplete work in this T. Integration, artifacts
and complete actual-diff acceptance remain separate pending S3 gates.

| Row | Current disposition and direct receiver |
| --- | --- |
| RTC-01 | Proven: calendar encoding/rollover and last-Sunday DSE, calendar smoke month/hour/daylight and wildcard contexts; invalid programmed dates remain non-eligible. |
| RTC-02 | Proven: 64-address bound, seconds bit 7, read-only C/D, 50 RAM bytes; contract `check_month_inputs`, `check_ram_reset` and Board CMOS route. |
| RTC-03 | Proven: eight DV selections; only three source operating bases and two divider-reset selections are assigned defined behavior. `check_divider_rates`, `check_update_bases`. |
| RTC-04 | Proven: complete DV/RS matrix and PF independent of PIE; `check_divider_rates`. |
| RTC-05 | Proven: both SQW edges, disabled-low output and reset/rate phase; `check_divider_rates`, `check_phase_continuity`. |
| RTC-06 | Proven logical phase: separate fast/slow default update durations and configured quantization; `check_update_bases`. In-update inaccessible values remain undefined, not guessed. |
| RTC-07 | Proven: SET clears UIE/UIP and aborts calendar delivery without stopping the divider; `check_set_phase`, `check_phase_continuity`. |
| RTC-08 | Proven: explicit divider-reset release relation distinguished from normal/test selections; `check_update_bases`. |
| RTC-09 | Proven: reachable PF/AF/UF and enable combinations, IRQF and read-C acknowledgement; `check_flags_and_enables` plus actual PIC receiver. |
| RTC-10 | Proven retained-powered RESET: mode/time/RAM survive, delivery enables/C clear; `check_ram_reset` and Board reset receiver. |
| RTC-11 | Boundary: construction assumes valid retained power; no selected battery-loss input or power-failure qualification. No new power-state owner. |
| RTC-12 | Proven bounded arithmetic/deadline and chunk equivalence: `check_set_phase` huge elapsed case, `check_phase_continuity`, daylight alarm horizon and Board deadline receiver. |
| RTC-13 | Proven logical selected port/index/NMI/IRQ8 path: actual Board RTC authority receiver; XT RTC is absent by selected composition. |
| RTC-14 | Proven declared ratios, not physical-clock accuracy: actual 1/8/16 MHz source contexts, 1024 Hz IRQ deadline and selected profile assertions. Existing L2 source axes stay L2. |
| RTC-15 | Proven: actual reset preserves RAM/calendar and clears IRQ through the same RTC; Board RTC authority receiver. No owner file or second CMOS store changed. |
| RTC-16 | Boundary: electrical oscillator, bus recovery, power loss and undefined update accesses are outside this logical selected model. |
| PPI-01 | Proven: reset and mode-0 direction matrix in PPI contract, actual XT reset receiver. |
| PPI-02 | Proven repaired latch reset for mode selection, including actual keyboard/speaker pin publication; PPI contract and XT Board receiver. |
| PPI-03 | Proven: all eight BSR set/reset selections preserve other latches and mode; PPI contract. |
| PPI-04 | Proven: latched output/unlatched supplied input and per-pin direction; PPI contract, actual PIT2-to-PC5 receiver. |
| PPI-05 | Boundary/non-applicable: selected XT mode 99h is mode 0; mode-1/2 handshakes are not implemented or qualified by the simplified fallback. |
| PPI-06 | Boundary: synchronous logical transactions are not electrical read/write/handshake timing. |
| KBC-01 | Proven bounded initialization: AA clears controller transactions/translation and returns 55h; `initialization_clears_controller_stream`. Compaq completed inhibit is sourced; IBM completed inhibit/cold MCU staging remain explicitly modeled. |
| KBC-02 | Proven: persistent CD, sampled inhibit, current OBF/SYS and selected AUX distinction; `status_latches`, `command_personalities` and actual Board contract. Physical IBF execution time/error inputs remain non-eligible. |
| KBC-03 | Proven selected logical ordinary commands and F0--FF pulses: controller contract and actual A20 receiver. Boundary: IBM AC MCU RAM/PSW cannot be fabricated from this transport model. Compaq speed/read class remains incomplete S6 as listed above. |
| KBC-04 | Proven selected 101-key enhanced Set1/2/3 mapping, prefixes and identity translation; mapper corpus and actual KBC/PIC/port-60 receiver. No original 84-key MCU or extra 102-key position claim. |
| KBC-05 | Proven: one output/reply/serial path, whole-sequence admission, saturation, controller IRQ and acknowledgement; `transport`, `packet_saturation`, response/deadline tests and Board receiver. Logical queue capacity is not original MCU RAM capacity. |
| KBC-06 | Proven logical inhibit, A20/reset and selected pulse deadline; Board receiver. Boundary: no physical parity/timeout/serial waveform input or held-reset electrical model. |
| KEY-01 | Proven selected enhanced command/options, all three scan sets and buffer/default semantics; endpoint command/lifecycle/parameter/order receivers. Not an original 84-key identity claim. |
| KEY-02 | Proven selected Set3 default/all/per-key behavior and failure atomicity; `set3_key_types`, `set3_capacity_and_reset`, `set3_legacy_and_transactions`. Compaq two-byte/F4 transaction is selected; IBM list termination stays source-incomplete. |
| KEY-03 | Proven: sole BAT publication, reset/default/resend and endpoint/controller stream boundaries; endpoint lifecycle/order, KBC transport and Board cadence receivers. Physical MCU BAT timing remains modeled. |
| KEY-04 | Proven bounded complete-sequence repeat/admission and command parameters; repeat matrix, native repeat transport and mapper corpus. Board nominal repeat/serial clock conversions remain L2. |
| XT-01 | Proven: selected mode0 60h--63h directions, latch/ack and switch input; actual XT Board receiver, no AT KBC/RTC substitution. |
| XT-02 | Proven logical partial-frame inhibit/refused completion and reset-before-BAT; XT contract `refused_completion`, `inhibited_bat`, `line_reset_discards_input` and Board route. Physical bit waveform/selected threshold remain explicit model boundaries. |
| XT-03 | Proven reset clears queued/partial/repeat state; XT reset and saturation receivers. Boundary: selected original 83-key source gives 16 scans but not overflow-slot algorithm; retained last-slot FF is L2, not enhanced seventeenth-slot proof. |
| XT-04 | Proven: mode/direction publication, PIT2 OUT speaker/PC5, clock/clear and parity/channel NMI route; PPI/XT/Board receivers. |
| BOARD-01 | Proven current selected descriptors and logical clock/reset inputs through chip and three AT-family profile receivers; analog/electrical MCU facts are not inferred from descriptors. |
| BOARD-02 | Proven direct logical port-to-PIC/A20/CPU consumers by Board receivers and actual KVM-to-guest ingress. S6 still owes Compaq speed/HOLD and final cross-controller qualification; S3 external four-profile receiving replay is pending. |

All 38 original rows now have an explicit current disposition. This is not
S3 acceptance: the confirmed transferred class is still unresolved, and the
remaining artifact/integration and actual-diff gates must be completed before
forming the complete implementation P. No row is deleted because its current
boot succeeds, and no lower timing grade is promoted by the unit result.

### Current S3 Artifact Checkpoint

The persistent artifact pipeline (handle 20543) exited zero after building
all eight `vm-0-5-0555` products at jobs 8 in the same original eight trees
used by accepted S2. Each product's existing build gate reports the correct
x64/x86 PE architecture. Ignored transcript:
`build/t555-s3-artifact-requalification.log`. Direct deployed SHA-256:

| Artifact | SHA-256 |
| --- | --- |
| nxvm_xt_0_5_0555_x64.exe | 21EC795D18111DB0C38DC4F6F7DFBB9E8E8AFA393AD07858F794A6FCA1223194 |
| nxvm_xt_0_5_0555_x86.exe | 3A2E09F85BEBACE7C8FDB9CB775AC7215B5533496C0790A93B4735E07A06E10C |
| nxvm_at_0_5_0555_x64.exe | 7BE413AE594A2E3B381E6AE5F88A560FF611CDE9AB7B8BD60D6A86BF7EE4DDA7 |
| nxvm_at_0_5_0555_x86.exe | AB5F42837039D00A517F8B6F0F686E131E367A0CB70C3B00203167975CEC5BE1 |
| nxvm_model40_0_5_0555_x64.exe | 91400C5517ECB63E2A6F260185B01ABE8F5DB461CCF3DC91370F206C735D0717 |
| nxvm_model40_0_5_0555_x86.exe | 775A5C69D3637DA1924F6D9A7DA9B15686F58AE10C652A495DEFCEFCD5CE4276 |
| nxvm_default_0_5_0555_x64.exe | F151EDBCB1426B5A34B65829A01468724B275F837710C59EB260CD5279A8BBFD |
| nxvm_default_0_5_0555_x86.exe | 0A0EC5CA0DD5A8D60686430D7493FA4184BC2159DE2247D39BCD846CEA5E800E |

The four adjacent owner INI hashes still match the accepted S2 checkpoint
exactly. Direct Git inspection finds no Shared-six, MyNES source/test/EXE
or owner INI diff. No artifact version or machine identity changed. These
are current uncommitted S3 candidates, not accepted release artifacts.
The sequential receiving pipeline (handle 65077) runs each original
selected group's `run-integration-tests` once at jobs 8; its ignored
transcript is `build/t555-s3-receiving-integration.log`. No terminal
integration result or interactive desktop acceptance is claimed here.

### Receiving And Actual-Diff Review Checkpoint

The same live receiving pipeline has completed the default x64/x86 groups
at 22/22 each (64.08/79.39 s), My5160 at 2/2 each (22.98/28.36 s), and
My5170 at 4/4 each (47.66/62.02 s). No group was restarted or repeated.
The two DeskPro groups are still pending; this checkpoint does not claim
the complete four-machine receiving result or S3 acceptance.

Executor actual-diff review has read all changed production and test C/H
and test-registration changes. RTC expected tables and calendar boundaries
are independent of implementation helpers; Set-3 expected groups derive
from the separately recorded original key tables. The Machine receiver
passes actual KVM events through mapping, endpoint admission, KBC output,
PIC acknowledgement and port 60h, rather than testing only a conversion
helper. The Board RTC receiver checks the existing clock conversion and
IRQ8/deadline route at three selected source rates. The XT receiver checks
PIT2 OUT, PPI direction and acknowledgement, serial completion and repeat
on the same guest-time path. App assertions change only for corrected
speaker AND, original-AT command-bit meaning and explicit pulse timing.

Counting tracked C/H/CMake diff paths with `git diff --numstat` gives
production 26 paths, +682/-218 (net +464), and tests 18 paths, +1818/-66
(net +1752), excluding manifests, documentation and executable artifacts.
Production growth implements missing logical register/stream/repeat behavior
and the complete selected Set-3 table; tests add source-conditioned contexts
and downstream receiving proof to existing executables, not a new framework.
The removed reset-pulse callback leaves the existing output-port sink as
the single pulse publication path. Chip state, board wiring and guest-time
owners remain separate. No Shared-six or MyNES source/test/artifact changed.

Both Core manifests, the Core corpus boundary, NXVM documentation governance
and `git diff --check` pass at this checkpoint. The existing Model40 file
has a Git LF-normalization warning, not a whitespace-check failure.
Final coordinator acceptance still requires the terminal receiving result,
complete document/ledger reconciliation and review of the complete delivery.
Interactive desktop acceptance has not been performed.

### Terminal Receiving Result And Remaining Failure

Pipeline 65077 exited 1 after all eight original selected groups ran once.
Default passed 22/22 per width, XT 2/2 per width and AT 4/4 per width.
DeskPro x64 passed 4/4 in 147.04 s (boot 145.94 s); DeskPro x86 passed
three receivers but its boot receiver failed at 180.11 s. Thus receiving
qualification is 32/32 on x64 and 31/32 on x86, not complete S3 acceptance.

The failed observation is running (`lifecycle=1`), not process shutdown:
elapsed source tick 411895109, next valid deadline 411895192, CPU
F000:90CF, bytes B0/00/E6/43/EB/00/EB/00, FDD 80x2x15, POST flag 90h.
The baseline S2 earlier timeout observed F000:90E5/90E9 in the same PIT
service region at approximately 420--422 million source ticks; its final
accepted original-budget run had only 0.07 s margin. This comparison is
evidence for investigation, not proof of an identical root cause or a waiver.
The existing Model40 x86 throughput TODO likewise does not satisfy this S's
receiving exit. No unchanged retry, timeout expansion, host-system change,
profile/media change or guest-time shortcut is selected.

Next diagnosis compares the exact firmware wait and current hot-path
arithmetic with the accepted baseline, distinguishing missed guest progress
from a controller/deadline defect before selecting a repair. All passed
groups, earlier failure records and candidate artifact hashes remain valid
as scoped evidence; no implementation P or S closure is claimed.

### RTC Empty-Advance Correction And Requalification

Current caller inspection confirms `core_machine_board_rtc_advance` reduces
source ticks through the existing rational clock and passes the result to
the chip even when that result is zero. The S3 RTC advancement otherwise
does multiple 64-bit quotient/remainder operations without changing phase
for zero elapsed input. The existing public register writes and reset
already refresh IRQ; no pending RTC transaction requires `advance(0)`.
The chip entry now returns for zero elapsed, without changing Board trace,
clock accumulation, positive-tick advancement or deadline publication.
This is an identified empty-work correction, not a proven timeout root cause
or measured throughput benefit.

The production correction is +1/-1 (net zero). The existing contract test
adds four contexts: SET on/off crossed with before/after a periodic edge.
It verifies unchanged deadline, IRQ callback count/level, SQW, seconds and
read-C flags. Both original-width RTC contract/calendar selections pass
2/2 (x86 0.19 s; x64 0.71 s). No public API, state, helper, timing recipe,
extra scheduler, retry or qualification-budget change is introduced.
Previous artifact hashes and whole-suite results describe the preceding
candidate; this final source change requires fresh full qualification and
all eight artifact identities before acceptance.

Final persistent pipeline 9601 uses the same original Ninja/ccache trees
and jobs 8. It rebuilds eight products, runs DeskPro x86 receiving first,
then complete x64/x86 units and the remaining seven receiving groups,
stopping on any nonzero result. Its ignored transcript is
`build/t555-s3-final-requalification.log`. No final result is claimed yet.

### Final Receiving Checkpoint After Empty-Advance Correction

The same pipeline 9601 completed the DeskPro x86 receiving group at 4/4
in 180.81 s aggregate; the boot receiver passed in 179.76 s under its
unchanged 180 s budget. This is one current-source pass, not evidence of
adequate throughput margin or a proven causal explanation of the earlier
timeout. The remaining complete dual-width units and seven receiving
groups are still running in that pipeline; no S3 acceptance is claimed.

All eight current product builds completed their PE-width gates before
this receiver. Final tracked C/H/CMake counts, excluding manifests,
documents and artifacts, are production 26 paths +683/-219 (net +464)
and tests 18 paths +1845/-66 (net +1779). The last correction introduces
no state, helper or public interface and adds 27 contract-test lines.
Both current Core manifests verify. Earlier artifact hashes above are
superseded by the final rebuilt candidates; final qualification will
record their identities together with terminal whole-suite results.

Direct inspection of the eight final candidates finds the expected PE
machines (8664 for x64, 014C for x86) and no `.debug` sections. Their
current SHA-256 values supersede the preceding artifact checkpoint:

| Artifact | SHA-256 |
| --- | --- |
| nxvm_xt_0_5_0555_x64.exe | B98348577D8D9CC2E3427A8074C5227CB374E6DA171244D265317434A8EBD6EE |
| nxvm_xt_0_5_0555_x86.exe | 0B81691E99AE0779557DAA336BC7126C2AA29CC3808861D3C2F11C6910A79C1E |
| nxvm_at_0_5_0555_x64.exe | DA19747B89F83336E007A33DB7ADB1E6FA86CB82F04EDC68AFD01ABFE9A91986 |
| nxvm_at_0_5_0555_x86.exe | 0DC3AB2E337DFC030EE3DEA7BF562A4EDAE5E88CD07560CFAFFF379984275033 |
| nxvm_model40_0_5_0555_x64.exe | 984102997465FABE18ADAA857EEB8C8FF7C2FED4B3645F21C345742FFF2808CD |
| nxvm_model40_0_5_0555_x86.exe | 6A772729D248A1E332A74FAD5FC0EEEAFCCFA5A451A4D4D31DC2812D31B044B6 |
| nxvm_default_0_5_0555_x64.exe | 832F9B35C4E758BB62E465C394926EF576477C42D46C444CFDD754A855B611E9 |
| nxvm_default_0_5_0555_x86.exe | A8C6985CE73C56ED3D71B4744A0464113FDB2B636DE76CF9B338C577DCCC5B63 |

The current Core corpus boundary and documentation governance checks pass;
`git diff --check` reports no whitespace error. These static/build facts
do not replace the still-running complete runtime qualification.

The final pipeline's complete x64 repository-only unit suite exited zero:
546/546, 253.86 s real time, under the existing 300 s aggregate bound.
It includes the three registered automated desktop-labelled routes; it
does not establish interactive desktop acceptance. The x86 whole-suite
and seven remaining integration groups are still pending in handle 9601.

The same pipeline subsequently completed x86 repository-only units at
546/546, 88.17 s real time, exit zero. Both current-source full unit
suites therefore pass; the seven remaining integration groups now run
sequentially in the original selected trees. No failed group has been
restarted, and no containment budget or predicate changed.

### Terminal Final-Pipeline Failure And Integration-Observer Repair

Pipeline 9601 exited 1 at default x64 receiving: 20/22 passed, while
`vm-debug-pause-boundary-smoke` and `vm-dos-keyboard-smoke` exited 1
without failure-stage output. The remaining six integration groups were
not run by this stopped pipeline. The DeskPro x86 4/4 and both current
546-unit passes remain scoped proof, not complete S3 acceptance.

Both existing App integration receivers now print the failed stage and
relevant status/state rather than silently returning 1. This diagnostic
change does not change input order, expected output or timeout. The first
diagnostic-only x64 execution passed 2/2 (9.68 s); that does not explain or
erase either earlier failure.

Source inspection identifies a definite pause-boundary observer race:
`emulator_machine_resume` clears the pause request and wakes the worker,
but the worker publishes RUNNING later. Immediately waiting for PAUSED
can therefore observe the old pre-resume state, before trace execution.
The synchronous debug service rejects a request when pause is no longer
requested, despite that stale sampled state. The receiver must wait for
a new completed PAUSED fact, not guess completion from the old state.

The existing test now registers the existing state sink before starting
the same described Machine driver and counts PAUSED facts with a local
Lib Types atomic. Trace-one snapshots that count and waits for it to
advance under the unchanged 2,000 ms budget before reacquiring its lease.
No production change, new API/thread, execution path or shared component
edit is needed. The test's explicit create/bind uses the same public
operations previously called by the integration start helper, allowing
safe sink registration before the worker starts. Cleanup still belongs
to the existing integration close operation, after worker shutdown.
The initial callback build omitted the typed cast required by the atomic
macro; correcting it precedes qualification and is not a passing attempt.

DOS keyboard has no confirmed production cause yet. Its stage/status
diagnostics are retained for the next complete receiving attempt. No
guest-input, RTC or host-system change is selected merely because the
diagnostic-only run passed. Final acceptance remains pending.

### Consolidated Receiving Observer

The test-local create/bind and counter described above were subsequently
removed. The existing `test/core/setup/session_ini` fixture now owns the one
PAUSED sink counter, initialized and registered before its original worker
start. Debug pause-boundary, unified Debug and timer-firmware use its bounded
resume-until-new-pause operation. These are three callers of the same observer
defect, not three production fixes. Worker shutdown precedes fixture cleanup.
No Shared code, production interface, thread or containment budget changes.

Pipeline 51829 was intentionally terminated as an owned process tree before
consolidating that fixture; its default x64 22/22 result describes the earlier
test-local implementation only and is not final current-fixture qualification.
Current focused x64 receiving passes all four tests (11.68 s); x86 and the
complete eight selected integration groups remain pending. DOS keyboard's
earlier failure remains unexplained; diagnostic and later passing evidence
must not be called a confirmed production repair.

Current focused x86 receiving also passes 4/4 (13.88 s). Test/Core manifest
and `git diff --check` pass. Pipeline 31923 rebuilds each original selected
tree with persistent ccache and jobs 8, then runs its integration selection
sequentially, once per width/profile. Its transcript is ignored local evidence
`build/t555-s3-current-receiving.log`; terminal results remain pending.

Pipeline 31923 completes current-fixture default x64 receiving at 22/22,
64.54 s, exit zero. It continues default x86 and the other six selected
groups. Four owner INI hashes still match the accepted S2 baseline, and
the Shared-six/MyNES path diff is empty. This is scoped receiving evidence,
not S3 acceptance or a declaration that DOS's earlier failure is explained.

Default x86 subsequently completes 22/22, 80.04 s, exit zero. Pipeline 31923
then selected 1,250 targets through XT's default build rather than the two
registered receivers. The owned process tree (PID 31664) was deliberately
stopped; its terminal handle returned exit 1 from cancellation, not a source
or test failure. Both completed default results remain valid.

Pipeline 97921 consumes each remaining tree's registered integration executable
names, builds only those targets in the same persistent directories, then runs
that complete selected group once. No new build directory, timeout change or
repeat of the two completed default groups is introduced. XT x64 passes 2/2,
23.04 s. The local transcript is `build/t555-s3-selected-receiving.log`; other
terminal results remain pending.

The same handle subsequently reports XT x86 2/2 (28.46 s), AT x64 4/4
(46.79 s, boot 45.99 s), and AT x86 4/4 (60.39 s, boot 59.76 s), all
exit zero. DeskPro x64 is confirmed live at its boot receiver; its first
three receivers pass. Both DeskPro terminal groups are still required.

### Complete Candidate Review

Executor actual-change review reconciles the complete production/test diff
with the 38-row disposition matrix, including the final AA initialization
and fixture-observer corrections. The bounded endpoint candidate commits
only after the full native sequence validates and fits; suppressed Set-3
breaks still clear repeat without consuming a serial slot. Rejected input
does not mutate prefix/repeat state. Endpoint stream-clear removes unsent
serial bytes and translation prefixes, not output already accepted by KBC.
The output-port latch and pulse mask have one publication callback; the
removed reset-pulse callback does not survive as another board route.

Source-conditioned test matrices cover all selected Set-3 keys and modes,
command preemption, partial-prefix rejection, full-queue repeat backpressure,
output pulse masks and selected board conversion. Actual KVM-to-KBC/PIC/60h,
RTC-to-IRQ8 and PIT2-to-PPI/speaker receivers remain distinct from chip-only
tests. Compaq speed/read/HOLD remains the complete named S6 repair class;
none of its missing behavior is accepted because a boot succeeds.

The fixture observer owns only test completion: it is registered before
worker start and is destroyed after shutdown. Three receivers wait for a
new PAUSED notification instead of reusing the pre-resume sample. DOS
keyboard's added diagnostics alter no input, assertion or timeout; the
earlier intermittent failure remains without a confirmed cause. Final
passing receiving evidence cannot erase that limitation.

Current `git diff --numstat` over tracked C/H/CMake paths, excluding
manifests, documents and EXEs: production 26 paths +683/-219, net +464;
tests 24 paths +1951/-93, net +1858. The increase implements missing chip
functions and source-conditioned downstream proof, not a framework or
parallel interpreter. The test increment includes the existing Core setup
fixture and four existing App integration receivers.

Both Core manifest verifiers, Core corpus gate, NXVM documentation governance
and `git diff --check` pass after the current fixture changes. Direct PE
inspection again verifies all eight final hashes listed above, machine 8664
for x64 and 014C for x86, with no `.debug` sections. All four owner INIs
match accepted S2 hashes; the Shared-six/MyNES diff is empty. No manual
desktop acceptance, statistical throughput guarantee or full-T closure is
claimed. The last DeskPro receiving result and pushed-P review remain pending.

### Terminal Current-Candidate Qualification

Handle 97921 is terminal, exit zero. DeskPro x64 completes 4/4 in 149.50 s
(boot 148.34 s); x86 completes 4/4 in 180.22 s (boot 179.28 s). Together
with the current-fixture default, XT and AT results above, the current
candidate passes all 32 selected integration registrations per architecture.
No budget or guest checkpoint changed. The x86 boot is close to its existing
180 s internal budget; a single passing observation is not a statistical
throughput or interactive-user qualification claim.

Complete current-production repository-only units remain 546/546 per width
from handle 9601. Subsequent edits are confined to the existing integration
observer/diagnostics and task evidence; they do not alter production or those
unit inputs. The four affected integration receivers were rebuilt/passed on
both widths before the complete current-fixture receiving selections above.
The eight artifacts retain the final recorded hashes and PE widths. Earlier
failures, source/model exclusions and the entire Compaq S6 repair class stay
visible; this evidence qualifies S3's assigned batch, not the whole T.

Executor self-review finds the complete S3 brief satisfied: every finite row
has a direct proof, an explicit selected-model/source boundary or the named
S6 receiver; confirmed owner-local repairs, similar-issue sweeps, manifests,
dual-width unit/receiving tests and artifacts are complete. Implementation P
push and coordinator review of that actual immutable delivery are still
required for S closure. No interactive desktop acceptance was performed.

### Coordinator Acceptance Of P1

Implementation `2ad955473` is pushed to `origin/master`; local and remote
tracking revisions match and the implementation worktree is clean. The
coordinator reviewed the immutable actual change, not only the executor
summary: production register/default/phase/sequence changes, all changed
Core/App test and build paths, manifest entries, eight deployed artifacts,
task packet and source/disposition record were compared against the S3
brief and applicable ownership, coding, source and execution rules.

The complete 38-row matrix is reconciled. Source-backed logical behavior
has direct receivers; electrical/undefined/non-selected behavior is explicitly
non-eligible rather than invented. The Compaq speed/read/HOLD class stays
required S6 work, with no T-level completion claim. Public Core changes
match the reported packet surface; no Shared or MyNES implementation or
owner configuration change entered P1. The test observer uses the existing
state sink and setup owner, not a product state mirror or new thread.

Final 546/546 units and 32/32 four-App integrations per width, both Core
manifest/corpus checks, documentation governance and verified current
artifact identities satisfy the assigned S3 gate. The earlier unexplained
DOS result and DeskPro narrow-margin timing remain truthful limitations,
not silently reclassified fixes. Coordinator accepts P1 and closes S3
through the separate pure-governance P2. T555 remains open for S4--S6.

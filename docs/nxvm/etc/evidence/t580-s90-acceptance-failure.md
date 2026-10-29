# T580 S90 — User acceptance failure: PC110 boot and cursor

## Timing-result reuse recheck

The earlier timing-result reuse candidate was temporarily applied for a
correctness and performance check. It passed 58 selected Core Machine,
manifest and PC110 construction checks on each architecture. A fresh
sequential warmup/measurement pair reported baseline 30,203,254 ticks in
10,055 ms and candidate 24,697,044 ticks in 10,031 ms; both ended at
3000:8000 but their individual guest instructions were not controlled. This
approximately 18% lower candidate throughput contradicts the earlier single
positive sample. Neither pair is a controlled whole-boot benchmark. The
candidate was removed, X86-Core source and manifest restored byte-for-byte,
both standard and observable Core libraries rebuilt, and the timing-preview
check passed on each width. No X86-Core source change remains. The new
baseline and candidate log hashes are
`A3359942809F0FA09DF1226C7804666FD6F4BFEEE4285E5113F3AA614759AFD8` and
`F7228AB5F823283553D29B18E1009C2DAF2C6720EA49A1F04A1A25A583BFDB31`.

The owner reported the released PC110 remains on the startup cursor for an
unacceptably long time and the cursor does not blink. The earlier S90
integration passes therefore do not establish the reported Product UX.
The owner INI selects Window, overlay mode and the two external PC110 media
images; the owner TFT CMOS has the same hash as the earlier Guest-generated
S87 state. The actual owner process was not modified or terminated.

A separate no-UI x64 Machine-runner probe used private copies of those
inputs and the committed implementation. At 15 seconds it captured an empty
80x25 text frame with enabled/visible cursor; by 30 seconds it was executing
the known firmware port-61 wait. Around 90 seconds it reached the previously
identified 3800:1333 firmware suspend point with the same static cursor.
Without input it stayed there through the 180-second observation, while guest
clock advanced. The public CPU `halted` sample was false at that point;
therefore that flag is not a reliable standalone detector for this boundary.

A second bounded run delivered one ordinary F12 make/break through the
Emulator input queue after observing that firmware suspend location. The
firmware left that location, continued its own code and switched to a
graphics frame around 255 seconds; the 300-second run did not establish
DOS or Personaware, or inspect the graphic frame contents. F12 delivery
through this no-UI probe does not prove native Window input or that the user
did not already press F12. The reproduced delay is a separate UX defect;
no host-side PC-specific execution shortcut or synthetic guest state was used.

The F65535 snapshot currently sets `cursor_visible = cursor_enabled` and the
Machine forwards it as `KVM_CURSOR_SOURCE`, so the existing host blinking
does not apply and the enabled cursor remains solid. The locally archived
C&T 65535 register reference revision 2.0, printed p.123, XR60, states that
bits 5:0 specify half the cursor blink period in VSync counts using
`programmed = actual/2 - 1`; the documented reset value is 0x83, i.e. four
VSyncs visible and four hidden. Its SHA-256 is
`ACB95E9BD56786227B6B9201B3BFF317ADBD224D1A587FBF8DB515FB5A648A91`.
Current F65535 reset clears the XR60 register and does not model a VSync
clock or cursor phase. The formula and reset are manual-backed; the actual
VSync rate remains to be established or explicitly marked L2. A guest-clock
chip implementation needs phase-change notification through the existing
frame publication route. No Lib change is implied.

The no-UI probe log SHA-256 is
`85209BBF341467ADAC19DE6107F749E3742EF5EE610176F228FF7BCB39E71`.
No production file or external source was modified by the audit. User
acceptance remains failed and S90/T580 remain open.

## Cursor repair under verification

The F65535 implementation now applies the XR60 reset value and computes the
cursor phase from its programmed VSync half-period. PC110 supplies a nominal
60 Hz VSync count derived from the existing guest CPU clock; this frequency is
an L2 model, not a measured PC110 display clock. Cursor phase changes use the
existing snapshot change signal, and the source-controlled KVM cursor remains
in use. No Lib or Emulator change is involved. The cursor phase stops advancing
when guest time stops. Chip-level tests cover the reset rate, alternate rate,
visible and hidden transitions, and disabled cursor. This does not by itself
establish the native Window result or resolve the long firmware startup.
The existing chip fixture also repeats a capture at the same guest VSync
count and verifies that neither the hidden phase nor its change flag advances;
the fixture and its test manifest pass on x64 and x86. This checks the chip's
fixed-time behavior, not a complete Session pause/resume integration.
The same original C&T reference, printed p.87, also specifies XR0E bit 2:
when set, the text cursor is non-blinking. The chip now honors that control
without changing cursor enable or adding an interface. The existing chip
fixture exercises disabling and re-enabling blinking as well as cursor
disable; the targeted fixture was rebuilt and passed on x64 and x86. Four
PC110 candidates were rebuilt afterwards; their new TFT x64, TFT x86, DSTN
x64 and DSTN x86 SHA-256 values are respectively
`6D5E1A84D1BE79828655B5437BD4872C3D0EDF3DDAA8BFE61CF0C7020B4FAF57`,
`36FC0BD086F7C53A51E1B873BCCDA02BCBEC0550982AEF154ECABEB3B1B3AF33`,
`7A1BEF07D52D5650F165F78D96CB7AA275043F0227DB412240BA8F7228EAD667`, and
`6495F307E1A034B15875232AB57EC03B8A2AD0A008B07E767A489F59187FCE86`.
These supersede the earlier candidate hashes below; they are not desktop or
unattended-start acceptance.
After the XR0E change, the PC110 machine-construction and both TFT/DSTN
product-composition fixtures were relinked and passed on both architectures
(three tests per architecture). The X86-NXVM source and test manifest checks
also passed on each architecture. These targeted checks do not replace an
unattended-start integration or a native desktop test.
The PC110 profile uses `DISPLAY_TOPOLOGY_NONE`; the generic video observation
therefore requests capture without relying on a separate video generation.
The F65535 snapshot marks a cursor change when its visible phase changes, so
the existing Machine publication path can forward that change.
After the affected test executables were relinked, the selected X86-NXVM and
MyPC110 suites passed on both x64 and x86: 117 tests per architecture,
including eight PC110 integration tests. These are not a native Window or
end-to-end Personaware startup acceptance.
The four currently deployed, uncommitted candidate artifacts have PE machine
types 8664h (x64) and 014Ch (x86). Their SHA-256 values in TFT x64, TFT x86,
DSTN x64, DSTN x86 order are
`901FC618CF0D46F73F5E18EEF0741475F060D932F83B18CAAAACA9971755770B`,
`D1A1C02AE20EE309EA8E45B03713C252283325466718169E2D965243CC87039B`,
`78464CCACA2FDF07E57CF3025A09F6B2A935FC6B0E2A7ECF4643B0ED4C2FD78D`, and
`3F0BF54A60FDC3A721915FC7250003FC9DC2C4942C356730C15011035363A67B`.

A separate no-UI x64 probe linked the updated production libraries and ran
the actual TFT firmware with private CMOS and overlay media. It periodically
paused the Machine and forced the existing display publication path: at about
12.2 seconds the 80x25 frame reported an enabled, visible cursor, at 14.3
seconds it reported enabled, hidden, and at 18.4 seconds visible again. The
probe exited successfully. This verifies the chip-to-Machine frame path under
those sampled conditions, not the native Window, RDP, or unattended boot.

A follow-up short isolated TFT probe also called the actual Machine driver
`copy_frame` after each forced publication. It observed KVM cursor mode 2
(`SOURCE`) and both phase 0 and phase 1, matching the corresponding F65535
snapshots. The run completed successfully; its log SHA-256 is
`26523D988E5B27C15172BFB3F9F65E29109929FCF4B61535016AA2981709BAAE`.
This covers the Machine-to-KVM frame handoff, not native window painting.
An additional no-UI x64 TFT firmware probe did not call forced display
publication. After each pause it copied the existing Emulator Machine's
published frame through `emulator_machine_copy_published_frame`; the normal
runner had produced source-controlled cursor phases 1, 0 and 1. The process
exited 0, its private disk inputs retained their hashes, and the output hash
is `AD3FE379555076DE00C682AA1EE39D4324D616B2166D100E6C5B48485080C5A7`.
This verifies normal Machine-to-Emulator publication, not native Window/RDP
painting or unattended boot.
The same probe relinked after the CRTC reset/readiness fix again observed
both `SOURCE` phases and completed successfully; its log SHA-256 is
`3FC9AEC26C240EE3DF7CEF6D813E26ECA551C8F4E11D0601FC3D6B2C5A294EC3`.
The existing Lib Win32 Window capture contract test also passed on x64 and
x86. Its mocked native paint and timer path checks `SOURCE` phase changes and
that host blinking does not override them. It is not a real desktop test.

A further short no-UI x64 real-firmware probe exercised the Emulator Machine
pause/resume path four times. During each 300 ms paused interval the Core
elapsed tick count stayed unchanged; after each resume it advanced. Published
KVM frames on the later samples contained SOURCE phases 1, 0 and 1. This
checks the actual Machine pause boundary with private media, not the native
Window or RDP. Its completed log SHA-256 is
`0CD0D4FAC81D6C9E564E2828070EC66D3539DCFA951F4E5EE93BE0E30FAEBB93`.

Cold-reset review found that PC110's profile reset did not call the existing
F65535 reset, so a programmed XR60 survived a full Machine reset. A temporary
extension of the existing PC110 construction fixture reproduced this on x64;
calling the existing chip reset made that fixture pass on both widths. All 11
PC110 units and eight registered integrations then passed on both widths.
However, the chip reset also clears the CRTC registers and the current capture
path treats their all-zero state as a valid 1x1 text frame. The Machine forces
publication after reset, which could shrink an existing Window. Those tests do
not cover the display result. A separate x64 chip probe directly returned
`create=0 capture=0 cols=1 rows=1 cell=1` for the reset state, and the initial
reset change was withdrawn while its display contract was investigated.

The C&T 65535 revision 2.0 register reference, printed p.62, specifies that
CR17 bit 7 is cleared by RESET and holds HSYNC and VSYNC low until released.
An isolated real TFT BIOS observation showed CR17=A3h during 40x25 and 80x25
text display; its log SHA-256 is
`899165D1097C92CB3851A60A53DD1FC62EDA9815DC6E0CDE1459DCBC0CCC91C7`.
The snapshot now reports `UNSUPPORTED` while sync is held, preserving the
caller's existing frame, and the App calls the existing chip reset during the
full board reset. Existing fixtures verify that an explicitly enabled 1x1
frame remains accepted, a disabled capture leaves the destination unchanged,
and a PC110 driver reset restores XR60 while retaining the last 80x25 frame.
These focused tests passed on x64 and x86. Core's processor-only reset remains
a separate path. On the corrected source, the selected X86-NXVM and MyPC110
suite ran 117 tests per architecture: 115 passed on each, and both manifest
checks failed because the two changed hashes were mistakenly written in lower
case. Correcting their required uppercase encoding and rerunning those two
checks passed on both architectures. The initial failures are retained here;
this combined result does not qualify unattended boot or native desktop UX.

## Suspend and port-lookup observations

A second no-UI TFT run sampled the configuration chip without injecting
input, using private CMOS and overlay disks. At 83.6 seconds it was executing
at 3800:653B with FF=00h; by 85.6 seconds it was at 3800:1333 with FF=82h.
At the latter point the chip's suspend-refresh request is asserted; B7, B8
and FB were zero in the sampled state. It remained there through the final
132.5-second sample. Guest elapsed ticks continued while the stopped CPU did
not progress. The completed log SHA-256 is
`37AB375286A6812A43FB2F93066F85DD005E0A8BB5E322BC6C89B7B23C7CC5E5`.
These samples identify the actual power-state boundary but do not establish
why the firmware requested it or justify injecting a resume event.

The locally archived VLSI VL82C480 December 1992 datasheet was additionally
reviewed as a related-family source. Its Table 4 identifies FBh as a
dedicated configuration-enable I/O port, not the PC110 Block-2 indexed FB
register. It does not establish the latter's bit-3 semantics or a suitable
power-on event. No register behavior was inferred from this name collision.
The archived PDF SHA-256 is
`DC7205BC3D05D327B7F33E7BA2EDC83846149DFF623A84414A8F426CE1A2E7BE`.
The Intel SL architecture reference, printed pp. 67--68, describes separate
auto-power-off, suspend-button and battery-low causes and suspend-refresh
control. It identifies its own suspend-status register as B8h, not the
PC110 FBh input under investigation. These cause categories constrain a
possible model but do not justify assigning Intel bit positions to the VLSI
part. The reference SHA-256 is
`3BFEA5A3C1B3BDC6A03BF2E622F24A4522123E3D0AB810386E02E3C72A94755B`.
The Open-Source-PC110 chipset notes initially describe FFh reads from the
24h/25h window, but their later sections document a successful four-read
unlock and a live Block-2 dump. The earlier locked-window FFh result therefore
is not evidence that the BIOS's unlocked FBh read should return FFh. The
local reference explicitly describes FB bit 3's exact meaning as inferred;
no open-bus fallback was introduced as a substitute for its power behavior.
The same external notes call the word write to port 22h a Block-2 relock.
That interpretation conflicts with the project's S47 actual-firmware replay:
coupling that CPU lock to Block 2 made subsequent BIOS initialization writes
disappear and prevented the observed ASMI setup. The existing configuration
route fixture covers the independent-lock behavior. This discrepancy does
not establish a reason to change the current lock or fabricate FB status.
Current-source sweep finds the only production hardware-side setter of FB bit
3 in `x86_vl82c420_set_resume_button`: it requires a new button edge while
FF requests suspend, then also latches the B8 resume cause and releases FF.
The normal cold-start path does not call that operation. The existing chip
fixture checks held-button, release and qualified-edge behavior. Thus the
current implementation has no independent cold-start producer for FB bit 3;
this observation identifies the missing model boundary, not which physical
signal or event should own it.

Further static inspection of the unchanged TFT firmware narrows the subsequent
SMM dispatch. The caller at F000:51C9 enters F000:3F9D; after FB bit 3 tests
clear, the firmware issues the vendor ASMI request. Its copied 3800:A4E7 path
then checks an internal status byte via 3800:A8B2 before selecting the indirect
power handler at 3800:A50F. The helper maps its F8 identifier to DS:0084 in
SMM-owned memory; it is not a direct read of chipset Block-2 register F8.
This is static control-flow evidence, not a measured value at the branch and
not a license to force either that byte or the earlier FB status. The next
observation must capture this internal state and its real producer before
choosing a power-model change.
The vendor call itself also has a traced static route: its saved AH:AL=5380h
and BH=82h select the third entry of the firmware's vendor table, 3800:2F36;
the observed CX=FF02h takes that routine's all-device, state-2 branch through
3800:24E7 and the previously observed indirect power-handler call. This
reconciles the apparently similar S47 rejected mask-80 experiment with the
current behavior: the current run enters the registered ASMI handler via its
normal source, and that handler can itself request the power handler. It does
not prove why the earlier FB predicate has the value it does on real power-on.
The Open-Source-PC110 Pluto investigation also identifies the keyboard
controller's P20/KBCSMI line as a system-management interrupt associated with
power, Fn and lid events. The current PC110 App routes its F12 power input to
the existing VL82C420 resume-button path; it does not model this independent
keyboard-controller SMI line. This is a modelling gap, not proof that an event
exists during the observed cold boot or that KBCSMI maps to FB bit 3. It does
not justify asserting either signal unconditionally.
The same source's Keyboard investigation describes the M38813 scan loop and
three standard scancode tables, but does not document a cold-start KBCSMI
assertion or map it to the Block-2 status bit. The PSU M38223 handshake uses
its own P20/P21 pins; those are a different controller from the M38813 P20
KBCSMI pin and must not be equated merely because the pin labels match.

The Open-Source-PC110 chipset analysis identifies programmed Block-2 slots
C8--CB and D0--D3 as possible I/O-trap descriptors for ports 03F2h and 0060h,
respectively, and describes B7 as a source dispatch register. Firmware writes
03F2h immediately before its vendor ASMI call. The current VL82C420 model
implements ASMI and button-related sources but has no general I/O-trap source.
This is a concrete modelling gap worth investigating, not yet a demonstrated
cause of the startup hang: descriptor attribute semantics and the trap enable
state at that precise access remain unverified. In particular, the same source
also identifies slot E8--EB as an I/O-trap source with mask 20h, whereas the
current ASMI model uses B7 bit 20h, so these mappings cannot be assumed
independent without reconciling the actual dispatcher and trigger rules. No
trap, synthetic interrupt or bypass was added by this observation.
The already archived Intel SL architecture reference, printed pp. 60--61,
describes a further constraint for its analogous device monitor: accesses
reload the device's idle timer; the trap SMI that turns a device back on is
associated with a device that was powered off after its local standby timer
expired. It does not support blindly generating an SMI on every access to
03F2h. This is an architectural analogy, not proof of VL82C420 register-bit
equivalence. In particular, neither a simple unconditional trap nor an SMI
source-mask mapping is justified as the fix.
S47's actual BIOS replay resolves part of the apparent source-mask conflict:
the selected firmware registers its vendor-call handler for mask 20h, and a
real 15EEh ASMI was observed dispatching that handler and returning via RSM.
Therefore the external I/O-slot labeling cannot by itself displace the
existing ASMI mask. Whether the same slot can generate that source under
other conditions still requires independent evidence.

An isolated TFT firmware replay installed the existing retirement observer and
captured the exact F000:4031 OUT to 03F2h. At sequence 31,966,305, it wrote
0Ch with B5=F5h, B6=90h, B7=00h and SMI armed. The observed slots were
C8--CB=`F2 03 21 02`, D0--D3=`60 00 24 00`, E8--EB=`F4 03 21 02`.
Unlike the live dump's `A1` byte, both FDC slot mask bytes were `21h` at
this access. In the Intel SL analogy bit 7 is the trap enable, so these would
be disabled; this remains an inference, not a proven VL82C420 bit definition.
The run subsequently reached the same suspend endpoint, stderr was empty,
and private disk hashes were unchanged; process exit code was not recorded.
The observer changes instrumentation cost and does not establish that all
other I/O-trap cases work. It narrows, rather than proves, the proposed cause.
Output SHA-256:
`CA1B5470B9C147C669AEB68E985347E1577EC50AA1310BC52D07AA482BCEB803`.

An isolated x64 TFT probe subsequently read the two bytes through the
existing Core memory interface while periodically paused. During the SMM
samples before suspend it read DS:0003=01h and DS:0084=0Fh; at the observed
FF=82h endpoint it read 81h and 0Fh, respectively. The latter value's high
nibble is clear, consistent with the static branch predicate, but these are
periodic samples, not an instruction-boundary observation of the predicate.
Reads outside SMM can see a different memory mapping and must not be treated
as the saved SMM value. The BIOS helper at 3800:A8BB explicitly clears the
high nibble and sets bit 1 in this internal byte; other firmware routines can
set high bits, but this static inspection does not establish which of them
should run in this cold-start case. The probe recorded the suspend endpoint,
its stderr was empty, and both private disk copies retained their input hashes;
its process exit code was not recorded. Output SHA-256:
`53F8EB7EB89732CA79ECA7B57B4ECEDCD2EF25C17642C1A32519F2679E04120C`.

A separate x64 TFT no-UI probe used a private, explicitly zero-filled 128-byte
CMOS image rather than the owner's retained configuration, and private overlay
disk copies. It performed reset then resume and injected no keys. By 83.7 s it
had reached the same 3800:1333 location with FF=82h, B8=00h and FB=00h; it
remained there at the final 132.6 s sample. All 65 planned samples were
recorded and stderr was empty; the process exit code was not captured, so this
is an observation, not a passing integration test. Both private disk images
retained the same hashes as their input copies. This narrows the failure beyond
the retained owner CMOS but does not establish the cause of the firmware's
power request or test the native Window. The zero CMOS and output hashes are
`38723A2E5E8A17AA7950DC008209944E898F69A7BD10A23C839D341E935FD5CA`
and `A04AD331C722BE9979814B314745583F03C3FD7F2024296E6337595B962CCF38`.

An independent x64 TFT replay used the existing retirement observer to record
all accessible Block-2 FBh data-port accesses until suspend, without injecting
input. It observed exactly three: firmware wrote 00h at physical 3E3D7h,
later read 00h at FDBCDh immediately before the POST predicate, and wrote
00h at 392D7h during the suspend sequence. The bank was unlocked in all three
observations. No other FBh data-port access was observed between the initial
clear and the tested read. This observer does not exclude every possible
multi-byte access or a missing hardware input, and does not define its
semantics. The endpoint was reached, stderr was empty,
both private disk hashes matched inputs, and the process exit code was not
captured. Output SHA-256:
`5F76C314950E3111CEBE84D59622C338BFCDDF2A11AAF4E5C30F2A37FAA78800`.

The corresponding x64 DSTN firmware was also tested without a pre-existing
CMOS file, using isolated overlay media and no key input. Its product asset
therefore supplied the normal DSTN first-run CMOS behavior, rather than the
TFT CMOS. At 77.5 s it reached the same 3800:1333, FF=82h endpoint; the
sampled SMM internal bytes were 81h and 0Fh. The diagnostic stopped at that
endpoint as designed; stderr was empty and both private disk copies retained
their input hashes, but its process exit code was not captured. This is a
first-run endpoint observation, not an end-to-end desktop or DOS test. It
shows the issue is not confined to the TFT firmware. Output SHA-256:
`3E7D48646E320FF225982F3D6BB185B87F1E5F6944C354BD1E13A7E83E4A062D`.

The PC110-QEMU reference's `qemu/hw-misc/pc110-chipset.c` initializes its
Block-2 registers from the same published hardware dump: the complete 256-byte
image, including FB=18h and B8=00h, matches `mypc110_cold_block2`. Both
models permit subsequent software writes to that bank. QEMU explicitly models
its CPU clock-stop ports as read/write latches and does not stop the clock;
therefore its boot behavior is not an independent demonstration that our
current stopped-CPU path or the meaning of FB bit 3 is correct. Its product
also contains optional BIOS-specific resume hooks. Neither difference justifies
changing the cold register image or claiming an automatic wake has been fixed.
An additional reference-code check found that PC110-QEMU's custom 15E8h I/O
region is five bytes long, covering 15E8h--15ECh, and no 15EEh/ASMI handler
is named in its checked QEMU C/H sources. The BIOS vendor request in our
observed path writes 15EEh. Thus that reference does not show an equivalent
ASMI-to-SMM device route in its PC110-specific implementation; its ability
to boot is not evidence that the unchanged firmware's same power handler
returns there. This is a source comparison, not an instrumented QEMU run or
proof about any generic QEMU device registered elsewhere.

To test that distinction, a scratch-only x64 TFT candidate retained the guest's
register writes but did not assert the CPU stop-clock input, resembling the
reference's latch-only clock treatment. With the same private CMOS and overlay
disk inputs and no key injection, the real firmware still reached its HLT: at
80.3, 90.4, 100.4 and 110.4 seconds the CPU remained halted at 3800:1334 while
guest time advanced. The owned diagnostic process was terminated after these
samples; this is not a full-boot test. Both disk copies matched their input
hashes. Therefore omitting stop-clock alone does not provide unattended boot
and would also discard implemented suspend behavior; the candidate is not a
production change. Output SHA-256:
`7088DC6D32718177F42A7530767FE95D4DAD3A98901DF915651AC556F34E2ED0`.
The actual PC110 input route consumes F12 before ordinary keyboard delivery,
calls the existing resume-button operation and requests processor reset only
when the chip reports a qualifying edge. The existing registered
`real-firmware-boot-handoff` test uses the firmware resume path, and passed in
the corrected-source dual-width runs above. This establishes the existing
explicit wake route at its test checkpoint; it does not turn an unattended
cold start into a wake event or establish the user's desired startup time.
The existing test is now registered as
`mypc110.integration/real-firmware-resume-boot-handoff` to make that input
requirement visible. Its executable, argument, assertions and timeout are
unchanged. Both existing Ninja trees were reconfigured and each lists exactly
the new test name; this name-only check does not rerun the long guest test.

A follow-up run stopped at the first FF=82h observation and captured the
paused guest stack. The first return was 3800:2512; this agrees with the
earlier S51 traced call at 3800:250F into the power handler. S51 traced the
preceding F000:3FF8 FB-bit-3 test and its ASMI request; neither this stack
nor that trace proves why a physical PC110 would report the necessary input
at that point. The follow-up log SHA-256 is
`9953DEE52C7C03AD1DDEE61F987C101572184513E427B315BE365A479BECA5DC`.
Read-only inspection of the selected TFT BIOS also finds that its POST path
calls the FB-checking routine at F000:51C9 before continuing with later POST
work. This is consistent with S51's earlier execution trace and explains why
this is a cold-start boundary, rather than evidence that a host key was sent.
The chipset research still labels FB bit 3's precise physical meaning as a
hypothesis; neither a fixed FB value nor an automatic button press follows
from this observation.

One isolated x64 candidate changed port read lookup to scan the route list
once when no wired-OR source is present, without changing production files or
device callbacks. Sequential ten-second early-firmware runs measured the
baseline at 30,868,894 guest ticks in 10,034 ms and the candidate at 29,985,448
in 10,045 ms. This one comparison does not show a useful speedup, so the
candidate is not adopted. It does not measure the complete boot or establish
the relative cost of every port. Baseline and candidate log SHA-256 values:
`180F89884A042FE451EC2EF5F5E438593A21E19010AD7462A19A5360AE47FB4F`
and `5EBAC968196224C4FE9E62A6771E719ECC180BFAD2D529BF081C30C8A9320EB6`.

A later comparison used the same no-UI guest and a 30-second warmup before
each ten-second measurement. The baseline advanced 27,110,955 ticks in
10,033 ms; the single-scan candidate advanced 28,876,482 in 10,038 ms. Both
ended at the same sampled SMM entry, but the individual instructions and
interrupt histories during the measured windows were not controlled. The
approximately 6.5% difference in this single pair does not establish a
reliable speedup, whole-boot improvement, or correctness for wired-OR
providers. No production change is adopted. Baseline/candidate log hashes:
`ECE93D7DE58C4A68262E3FBA9D88E3C93B9083B6071093DC491562DEBECDB738` /
`341BFFC78B4254A1DF5F194F1CD5D479A2E5575CAAE2E31D503085321441676D`.

The CPU timing selector also scans instruction prefixes twice when building
its formula-input metadata. A scratch-only candidate combined those scans
without changing the prefix classifications. In one sequential early-guest
comparison, baseline produced 33,151,237 ticks / 10,043 ms and the candidate
29,911,935 / 10,032 ms. This is a noisy single pair, not a demonstrated
regression or a representative later workload; it does not justify a timing
implementation change as a performance fix. Candidate is not adopted.
Baseline/candidate log hashes:
`95D75330DE84EC2364824DCEF1C2F010B930D6759866A5A0615ED073D766C7E4` /
`1DBBB4FD0052C0F848B7BC587AEFBC8E49DB31318E82C7D2F495BE37A4E02C9F`.

Another scratch candidate captured the CPU timing result once per execution
round rather than reading the same result again after prefetch reservation.
It produced 29,156,431 ticks / 10,031 ms in the 30-second-warmup workload,
against the above baseline's 27,110,955 / 10,033 ms; both ended at the same
sampled SMM entry. The measured windows still contain uncontrolled guest
execution, so this one result does not establish a reliable improvement or
qualify the candidate across CPU, fault, and prefetch paths. It has not been
applied to production. Candidate log SHA-256:
`63321322675F94B8129EB4BC079323355F3F1AD8571DF40BD3AD2BFA49C07A02`.

The 486 lexical preview also copies the CPU cache and TLB into a temporary
execution context. A scratch-only experiment avoided that bulk copy and read
the original cache/TLB during preview. Its diagnostic-only pointer reuse was
not a production design and has not been applied. After the same 30-second
warmup it advanced 27,989,432 ticks / 10,033 ms, versus the earlier baseline's
27,110,955 / 10,033 ms; both ended at the same sampled SMM entry. This one
uncontrolled pair shows no compelling improvement and provides no cache/TLB
correctness qualification. Its log SHA-256 is
`C425A5D7F6EC6047EB19FB04DE041D2C860C5666CB90295539C27786E2820E4B`.

The existing x64 LTO configuration was rebuilt against current source and
compared with current Release using the same private TFT inputs. A preliminary
30-second warmup comparison ended at different guest locations and is not an
isolated throughput result. A second sequential comparison waited until each
run reached the F000:EA86--EA90 BIOS port-61 wait before timing ten seconds.
Release began at EA88 and advanced 30,385,780 guest ticks in 10,033 ms; LTO
began at EA8C and advanced 34,176,448 ticks in 10,046 ms. The latter ended in
SMM while the former ended in the wait loop. The approximately 12.3% observed
difference is consistent with, but does not by itself prove, an LTO speedup or
establish an end-to-end benefit. No release compiler policy was changed.
The matched baseline and LTO log SHA-256 values are
`965ADE5EA44DB9842D038C45B840A03D2393F719122FD68C677FC274DFF805E7` and
`2070F69E3C0365DD838C8E65D8A87369231295A90A618C10C124C49E95AD7252`.

A more narrowly linked diagnostic used LTO only for the X86 Core CPU archive
and the probe translation unit; the remaining archives came from normal
Release. It began at F000:EA88 and advanced 33,122,146 ticks in 10,035 ms,
ending again in the same wait loop. The earlier matched normal Release began
at EA88 and advanced 30,385,780 ticks in 10,033 ms, also ending in that loop.
This is approximately 9% higher in one pair, not controlled instruction-for-
instruction timing or a complete boot measurement. It has not been tested on
x86 or against the full CPU correctness suite, and is not adopted as a release
policy. Diagnostic log SHA-256:
`C49C504904B7467B2EAAC9CF141BF593A5D1B11735351E1FC51FF44BFBFEB63D`.

An additional no-UI x64 experiment linked a scratch-only chip candidate
that forces FB bit 3 high on indexed reads. It used private copies of the
same CMOS and disk inputs and sent no F12 or Enter. At 180.7 seconds the
guest was still executing firmware, with an 80x25 text frame; the tested
instance did not enter the earlier 3800:1333 suspend checkpoint. This
supports the traced branch's relevance but is neither a DOS boot nor proof
that forcing the bit models PC110 hardware. The unsupported candidate is
not adopted. Both disk copies retained their input hashes; the completed
probe log SHA-256 is
`09BC57FB47661EE672C5D29BDA46B9E5794704BC10570FB41AECCFDA29BD5A02`.

A longer separate run of the same unsupported forced-FB candidate, again
without injected keys and with private overlay media, produced a partially
drawn Personaware launcher at 281 seconds and a complete launcher frame at
301 seconds. The probe was deliberately terminated after the checkpoint; its
nonzero exit is not a complete test pass or an interactive-input result.
Other task-owned dual-architecture tests ran concurrently during part of the
observation, so this elapsed time is not a controlled performance benchmark.
It establishes a later guest display checkpoint under the artificial FB
assumption, not the hardware meaning of the bit. The log and complete-frame
hashes are `7340495A169763F297F140652C5041B4DFB5153CD26F6752AFDFD0BA64458BD6`
and `85A35680232CA0AD623DB7DD18D23A79396931ACB133791581D40C37048AEF41`;
both disk copies retained their input hashes.

## Longer isolated Guest observation

A subsequent no-UI x64 Machine-runner observation used the same private
CMOS and overlay media. After the firmware entered its previously observed
suspend boundary, the probe queued one F12 make/break. The first captured
graphics frame at approximately 270 seconds displayed `Starting PC DOS...`;
the probe also queued two Enter make/break pairs after graphics appeared. At
421 seconds the captured frame displayed Personaware World Clock. This proves
that this isolated run reached that screen with those inputs, not that the
released Window path did so without input, nor that seven minutes is acceptable.
It does not identify which of the two Enter pairs was consumed.

A five-second process-local call-stack sample during post-wake firmware
execution showed inclusive CPU execution refresh and timing selection as
well as Machine scheduler and port reads; the sample was perturbative and
does not establish precise CPU attribution or justify skipping instructions.
Log SHA-256
`6D54CA9DB263B8FDF5399EDC62ECD22B4844DD5B40CCA2732C6E281C7590FC3A`;
DOS and World Clock raw-frame SHA-256 values
`144841A5852CF83B589CB9E03D7A77A0ACDFC22AB07787ADCB314499C9168FDE` and
`15273A877DD18D68BBC48FCB6E40F380D217F1395E1DB4FEE3F2E68FF9A654B5`;
call-stack sample log SHA-256
`AF0E99E7B95C2E9110C459EABB75CB103F24EC83BFF46029A019841D0B668D55`.

## Narrow text snapshot row stride

The F65535 text capture previously packed rows at the current column count,
while the existing Machine frame converter reads each snapshot row at
`X86_VIDEO_MAX_COLUMNS`. The two layouts agree at 80 columns but disagree at
40 columns. Capture now writes each row at the snapshot's fixed stride while
retaining the existing guest VRAM addressing. The existing chip contract test
checks distinct content and attributes on the second row of a 40-column frame
and the unused first-row tail. This is a separately confirmed display defect;
it does not establish the cause of firmware suspend or boot latency.

After this correction, the F65535 contract and eleven MyPC110 unit tests passed
on both x64 and x86. Both TFT and DSTN candidate executables were rebuilt for
both architectures and their artifact architecture checks passed. These
results do not establish unattended DOS/Personaware boot or RDP validation.
The existing Machine frame fixture additionally verifies the second row and
its colours for a 40-column snapshot, while retaining the padding check. The
Machine frame test passed on x64 and x86; the X86-NXVM source and test manifest
checks passed on x64. These two fixture tests exercise the producer and
consumer independently, not an interactive PC110 screen.
A scratch x64 negative check compiled the current chip with only its prior
packed-row loop restored and linked it to the unchanged regression fixture.
It exited 3 at the new narrow-row assertion; the fixed implementation passes.
This confirms that the new producer test detects the original stride defect,
without modifying the production tree or treating the negative run as a pass
of the complete guest path.

## Verification freshness correction

The first broad integration rerun after the stride change used a stale
real-firmware test executable: Ninja's dry run showed that the executable
still needed relinking against the new F65535 archive. Its reported pass is
therefore not used as evidence for the latest implementation. The same check
identified the Machine construction unit executable as stale. Both widths
were explicitly relinked, and the Machine construction unit passed again on
x64 and x86. The true-firmware tests are being rerun after relinking; their
results must be recorded separately. The six other PC110 integration checks
from the broad run passed on both widths, but do not establish a guest boot.
After explicit relinking, the current real-firmware FDC-reset and
resume-boot-handoff tests both passed: x64 2/2 (128.21 s) and x86 2/2
(152.63 s). The latter test injects F12; it does not verify unattended cold
boot. The desktop/RDP visual behavior was not tested in these runs.

## Cursor publication at the reproduced suspend point

An isolated x64 Machine-runner observation used the current F65535 and frame
libraries, the real TFT firmware, copied CMOS and overlay disk inputs. It sent
no key events. The probe periodically paused the Machine to read CPU state,
then sampled the already-published frame; it did not force frame publication.
It saw the real initial 40x25 text mode and subsequent 80x25 text mode. At
104.2--132.8 host seconds the CPU remained at 3800:1333 while the running
Machine's elapsed ticks advanced. Published source-controlled cursor phases
included both 0 and 1 at that endpoint, and agreed with the captured F65535
snapshot at the recorded samples. The probe exited zero; stderr was empty and
both private disk copies retained their input SHA-256 hashes.

This confirms that the Machine publication path carries a changing cursor
phase at the reproduced suspend checkpoint, not that the native Window or RDP
actually displays that change. It also confirms that the corrected 40-column
snapshot mode occurs during a real firmware boot, but this observation does
not compare every displayed character against guest video memory. It does not
fix or excuse the unattended suspend. Output SHA-256:
`C9EAAB435E06CB90A3EEF7FB5CFC2651B25F02FEDADD8BEC8DD4373E87E47626`.

## Matched real-firmware timer checkpoint: Release and LTO

The current x64 Release and existing x64 LTO-perf graphs were each rebuilt
for the real-firmware integration executable. Sequential `--timer` runs both
exited zero at the same 8,094,093 executed-instruction checkpoint with timer
delta 1675h, and their output files were byte-identical (SHA-256
`FD51DEF9422656E961516692B1E8F37990465A20280D70F7A33E075C5EB73902`).
Elapsed wall times were 21,278 ms for Release and 19,773 ms for LTO, about
7.1% less in this single pair. This is a controlled guest checkpoint but
not a repeated statistical measurement, a complete boot, a desktop test,
or x86 qualification. It does not justify changing release compiler policy
or claim to fix the much larger startup and suspend problems. No production
source or release configuration was changed for this comparison.

## Real 40-column snapshot against guest video memory

A separate current-code x64 TFT firmware observation paused at its actual
40x25 text mode and forced a diagnostic snapshot while paused. The existing
Core memory interface then read 2,000 bytes at guest B8000h. Comparing all
1,000 character/attribute pairs against the fixed-stride F65535 snapshot
reported zero mismatches; all 1,000 character bytes were nonzero. The same
run later reached 80x25, exited zero, and left its isolated input disk hashes
unchanged. This validates the captured guest text content in this sampled
mode, not the native Window renderer or all possible CRTC start/stride modes.
Output SHA-256:
`E8B209CD05F9E92272D9BCF945B20F35EC7F8E987D4474FDFE818E9083E4EB14`.
The equivalent isolated x64 original-DSTN firmware run, without a retained
CMOS image, also reached 40x25 and matched all 1,000 B8000h character and
attribute pairs to the paused diagnostic snapshot with zero mismatches. It
then reached 80x25 and exited zero; isolated media hashes were unchanged.
The diagnostic forced a capture while paused; it does not validate native
Window/RDP presentation or unattended boot. Output SHA-256:
`3D01E432FC63A24C32678682F96CFAC36F84338FD38FFB32A9EC38020AC26679`.

## F8 state writer observation during cold POST

A current-source, isolated x64 TFT firmware run used the existing retirement
observer to watch the BIOS FB test, the firmware's F8-indexed write helper,
its known high-nibble set/clear call sites, and the indirect power-handler
call. It injected no input and stopped after the existing FF=82h endpoint.
The log captured F000:3FF8 with AL=00h, then the indirect call at 3800:250F,
before reaching 3800:1333. None of the watched F8 helper or named high-nibble
writer sites was observed in that run. This bounds those particular dynamic
paths; it does not rule out writes through other instructions, prove the
physical origin of FB bit 3, or establish what a real cold start should do.
The observer perturbs performance. The process exited zero, stderr was empty,
and both isolated disk inputs retained their hashes. Output SHA-256:
`C147C251279DE1EB203986B90E9B0C4B5E6D42CEC0461A729AD6A4AD54CA95C7`.

## F8 initialization and branch correction

A follow-up retirement observer watched the actual PC110 private SMRAM backing
byte at offset 84h. In the isolated cold TFT run, it changed once from 00h to
0Fh at physical EED67h, sequence 2,289,156, with AX=0FF8h and BX low=0078h.
That address is the Flash alias of the indexed firmware helper's store; the
previous observer watched only its copied 3DD5Bh entry, so its absence did not
exclude this earlier initialization. The firmware's initialization loop calls
the SCAMP read helper for indices 80h--FFh; that helper masks bit 7 and reads
74h/76h. The current cold SCAMP image has index 78h = 0Fh. This is consistent
with the observed F8 byte, but the single retirement observation alone does
not capture the preceding read or establish hardware semantics.

An independent execution-boundary trace recorded the later 3A8B6h test with
AX=0FF8h, DS=3000h/base 30000h and SMM active, followed by the zero branch and
indirect 3A50Fh call. Attempts to call the public memory-inspect API from the
running retirement callback returned INVALID_STATE (2); those returned bytes
are invalid and are not evidence of the memory contents. Both runs exited zero
at the suspend endpoint, produced empty stderr and retained their isolated disk
hashes. These observations locate initialization and consumption more
precisely; they do not justify altering SCAMP 78h or FBh. Output SHA-256:
`D7EE55F3B9FC14E2C40A8ED310914607F122602842E3762D7E89D34536733D46` and
`2FA211746299C55B53341DA414C9E9DEC2724BCF87F8FA366F31BD0BE73D62CD`.
A subsequent short isolated retirement trace directly confirmed that data
flow: at sequence 2,289,144 the guest read SCAMP 78h from port 76h as 0Fh;
at sequence 2,289,156 it stored AH=0Fh for indexed state F8h. It exited zero
with empty stderr and unchanged private disk hashes. This confirms the value's
source in the current emulation; it does not establish whether a physical
cold-start SCAMP 78h should differ from the available live dump. Output
SHA-256 `18078A4C9058CB951B9C9347A202B899C014C7C4E43DBE8A0843B6C046222EE7`.
The `--timer` fixture installs a retirement observer, whereas the published
PC110 executable links the normal Release Core without an installed observer.
Thus the matched 21,278/19,773 ms comparison includes per-instruction
observation overhead and does not measure end-user product speed. The result
is retained only as a same-instrumentation comparison, not a basis for an LTO
release policy or a percentage claim about the actual product.

## BIOS keyboard wait and throughput boundary

A current-source x64 retirement trace recorded the firmware's KBC accesses
and wait entry/exit using isolated media and no input. A wait at F000:6436
began at retirement 23,456,194 and returned at 31,789,738, approximately
8.33 million retired instructions later, with the controller's output-buffer
status clear and CX/BL exhausted. The next observed PC was F000:651C; the
firmware listing identifies the corresponding optional wait at F000:64E8 and
its zero-result branch. Earlier reset-command ACKs and subsequent auxiliary
reset responses were observed, so the trace does not establish a missing
keyboard command reply as the cause of this particular wait. No synthetic key,
shortened delay or skipped instruction was added. The observer itself adds
overhead, and the count is not an end-to-end product benchmark.

The diagnostic reached the known suspend endpoint and exited zero, with empty
stderr and unchanged copied-media hashes. Output SHA-256
`83767D3BCB8202DD9F67671F3E21093D78D2387D1EF418CBABD2D555886DCED9`.

## Condition for the observed keyboard wait

The real firmware listing traces the F000:64E8 wait's preceding condition
through F000:DF1E and F000:DF41 to indexed ports 35EAh/35EBh, index 04h.
The current PC110 EC block returns 84h for this index, matching the recorded
Open-Source-PC110 live 35EA/35EB dump. Bits 3 and 4 of 84h are clear, so the
DF41 carry path is consistent with the optional wait. The earlier retirement
trace observed the wait's entry and its timeout/exit, but did not log this
particular indexed read; the condition-to-value link here is static analysis
of the code and configured value, not a direct observation of that read.
There is no evidence to change the live-derived value just to avoid the wait.

## Actual firmware cursor rate programming

A current-source isolated x64 TFT run observed the initialized F65535 state
while paused at five early boot samples. The firmware programmed XR60=88h and
XR0E=80h; the latter does not disable cursor blinking. Per the chip's XR60
formula this requests nine VSyncs per phase, nominally 150 ms at the selected
L2 60 Hz guest VSync, not the reset default of four VSyncs. Published frames
showed both phase values. Because VSync uses guest ticks, host-visible timing
also depends on guest execution speed and need not be 150 ms of wall time.
The probe exited zero with empty stderr; it used copied overlay media. This
checks register programming and Machine frames, not native Window or RDP.
Output SHA-256
`1D43BE45CD12DC3AD6039854592B6D511540CD621A5CD0825BC2F5880DDF5ADA`.

## Text CRTC offset audit

The current F65535 text snapshot advances guest source rows by visible columns
and also derives the cursor row from that same value. The F65535 CR13 manual
instead defines the programmed display-buffer row offset, with CR14, CR17 and
XR0D influencing address interpretation. A short current-source x64 TFT
firmware sample read CR13=14h at 40 columns and CR13=28h at 80 columns, with
CR14=1Fh, CR17=A3h, XR0D=00h and start=0000h; both observed modes therefore
agree with the current columns-based row spacing. The earlier direct B8000h
comparison also passed for the sampled 40-column frame.

The former columns-based text row handling ignored programmed row pitch.
The subsequent fix below covers the PC110 word-addressed text mode; byte and
doubleword text addressing are not qualified by this work. The sample exited
zero with empty stderr. Output SHA-256
`4D814FAF9E9905E1B6FDD8C77ADDC9E86F839DBA9F5871DE5B0408D8C011043F`.

## Word-addressed text row pitch correction

The existing F65535 contract fixture was extended with a 40-column frame,
nonzero start, CR13=25 and XR0D bit 0 set, producing a 51-cell row interval.
It places distinct data on the second row and a cursor two cells into that
row; it also checks that a cursor in the inter-row gap is hidden. With the
former columns-based capture the x64 fixture failed at the new assertion.
The current implementation derives row starts from CR13/XR0D for the PC110
word-addressed mode and uses the same row starts for text and cursor. Source
bounds are checked against the last visible row before the destination is
modified. No interface or Lib/Emulator change was added.

The affected F65535, Machine frame, PC110 construction and X86-NXVM source
and test manifest checks passed on both x64 and x86; the gap extension and
manifest were rerun after their final edit. Both PC110 firmware variants were
rebuilt on both widths and passed artifact architecture checks. Short x64
real-firmware paused observations, using copied media, each captured an actual
40x25 frame with zero mismatches across 1,000 B8000h character/attribute pairs.
Their output hashes are `3CD3EBE7E955A63D1907399EEE63C2E79E1C1E72C3CE2E86D5F84A5FF0F038A2`
(TFT) and `DAD30E539B13DB1EC226D19F3D92E6145A9D0FB4376ED0AC80C63C596FB8A03E`
(DSTN). These are not full boot or native desktop tests. Other text address
modes are not qualified. Unattended startup remains unresolved.

## Text addressing-mode containment

CR13/XR0D row-spacing semantics applied above are verified for the actual
PC110 word-addressed text mode. CR14 and CR17 also select byte and doubleword
modes, whose logical-to-physical address mapping is different. The source now
limits the new pitch computation to word mode and preserves the previous
columns-based behavior for those other modes; it does not claim that legacy
behavior is a correct implementation of their address translation. The
existing fixture checks both retained modes so this repair does not silently
change them. Both widths rebuilt the F65535 fixture, PC110 construction and
four PC110 candidate EXEs; the five targeted checks passed on each width.
This containment is not a completed implementation of byte/doubleword text
scanout, and it does not resolve unattended boot.

## Owner-authorized L1 unattended-start compatibility

On 2026-10-29 the owner authorized an L1 model or independent firmware
compatibility interception without waiting for a physical-hardware source.
Both selected firmware inputs contain the same `test al,08h; je` sequence
at F000:3FF8. The App creates a private copy of the selected immutable ROM
and, only when the expected sequence matches, replaces that conditional branch
with two NOPs. The input ROM and shared chipset are not modified. This is a
compatibility workaround for the observed startup path, not a claim about
physical PC110 power-on semantics. The chipset's existing explicit suspend
and resume implementation is unchanged; full guest suspend/resume behavior
after this intervention requires separate end-to-end verification.
The existing real-firmware resume integration receiver explicitly restores
the original two bytes in its private fixture: its purpose is to exercise
the pre-existing firmware suspend/button/resume sequence, not the released
cold-start route. A separate cold-boot receiver uses the unmodified production
construction path and checks the actual firmware's INT 19h handoff without
injecting a key. Neither receiver alone establishes interactive guest power
management after starting through the compatibility path.

The existing firmware-binding fixture checks the matching and nonmatching
sequences, mapped bytes and immutability of its source. The fixture and
machine-construction checks pass on x64 and x86. A separate x64 Machine
runner used the actual TFT firmware, copied CMOS, copied overlay media and
no key injection. It displayed `Starting PC DOS...` by 181 seconds and a
complete Personaware launcher by 291 seconds; the image SHA-256 was
`982B23B496A37B5F4DC23E6FE0625294A1463116670CFC21EB229810368CA7BF`.
The run observed both cursor phases. Other builds/tests ran concurrently,
so these times are not a controlled benchmark. The original media copies
remained unchanged. The probe log SHA-256 was
`F272F1E7C2FA5C033196244BF8DE09ACBEB84CB9557C7B999404DB9822A598FC`.
This is not a native Window/RDP or interactive input acceptance, and the
long startup time remains visible.

The derived DSTN firmware was also run with an independently copied, already
configured CMOS image and copied overlay media. It reached PC DOS without
input and displayed the same complete Personaware frame by 291 seconds; the
frame's SHA-256 matches the TFT image above. This does not by itself establish
first-run behavior with an empty CMOS or a native desktop/RDP result.
The DSTN probe completed with unchanged copied-media hashes; its log SHA-256
is `F4753832ECE4BFF62A457E32835A095F1DC70F461C4E035BCC5CC34211EADEA8`.

An additional derived-DSTN run used empty CMOS, private media and no input.
It displayed firmware ERROR `00173 / 00163` with OK and Cancel rather than
automatically reaching DOS. Three successive frames at 200--220 seconds were
identical; the probe was then stopped deliberately. The unchanged frame's
SHA-256 was `B9EA224DFEB96227BC89CF2B1FD9C5147324F85767C21B9FE8259DBAD9159B96`;
the log SHA-256 was
`6F52187483000E095EA031409D97D181B9601A9CB5379E83682914E091199CC6`.
This is a first-configuration boundary, not evidence of a cold-start DOS
pass. No configured CMOS is manufactured or committed by this task.

The complete registered unit suite passed 713/713 on each host architecture.
All four PC profiles and both PC110 firmware variants were rebuilt on x64
and x86 in the existing cached trees. PC110 FDC reset and the six other
product/firmware integration checks passed on each width. The original
resume-handoff test initially failed on the new production cold path because
it required the removed startup suspend. The separate test paths above retain
that assertion instead of treating this failure as a pass.
The original-firmware suspend/resume receiver subsequently passed on x64
(100.74 s) and x86 (124.51 s). The separate production-path cold-start
receiver passed without injected input on x64 (86.38 s) and x86 (103.79 s).
These tests check BIOS handoff, not the later Personaware screen or RDP.

Final rebuilt artifact SHA-256 values (PE architecture checked separately):

| Artifact | SHA-256 |
| --- | --- |
| nxvm x64 | `27e9cd45543bff554e386665aa2da44df6e44c36bb386fc045738730080d493b` |
| nxvm x86 | `079c99334dfd0cc9575e8e5a3d9a559a69b6f22beb5ecc411dcbe64a65ee430c` |
| my5160 x64 | `77c834aa5231d3dbfe37693394f578b11429a7422e7fbdf2d46641fa7a5c2c8e` |
| my5160 x86 | `6f83eadbdef7e8c1453b94417948ac5f05a842a6555aff037c1a8df8d34cf749` |
| my5170 x64 | `853ba1af6af718de678cc489a15713471b3d075997202a187074eb26ce86a444` |
| my5170 x86 | `225196a8593a7779495214ef7e79284ef091511e8b82fc7a57e6d8c98df42589` |
| mydeskpro386 x64 | `0facd15cab8db47c9db56da7d8e9a9533d3cb63f60b1568a44d8277a0497c41b` |
| mydeskpro386 x86 | `6311d0bca04c43e485453850068ca6e9a1665537ad186282dff5d68a68ab0e1f` |
| mypc110 TFT x64 | `fa6392b2b66c4655e1ae3548f66c8944cb2ec78c675a55f965f08d92c22c1b75` |
| mypc110 TFT x86 | `8cefff95a2d21045d2a318da4e79122edfa1d1c779363dd6e2c9ed4023e95627` |
| mypc110 DSTN x64 | `eab55818b05b7ba2de9acd6ce4992aa9f3861f92741da85d57c9d3c4e2d9deef` |
| mypc110 DSTN x86 | `37d1d6701173b6187793fb59b684d34ab99dd9f13a5aa80c73616e1c336ffc4f` |

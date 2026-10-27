# T580 S73 — PC110 ATA slave boot observation

## Input and scope

The local reference media is 4,194,304 bytes, SHA-256
`5149DB391D13CFEAB330016FCF0EDBE6B0D379CBB66A3AED91DBF7684142D52E`.
It contains an MBR `55AA` signature and an active partition starting at
LBA 32 with 8,160 sectors. The external PC110-QEMU disk description identifies
the Personaware media as user-supplied IBM software; it is used here only as
the owner's local BYOB input. No disk bytes enter the repository. The external
QEMU runner describes a 128×2×32 geometry but uses host POST/INT13 assistance;
its end-to-end boot claim does not prove our real BIOS selects this device.
The partition boot sector independently declares 512-byte sectors, 32 sectors
per track, two heads and 32 hidden sectors, with its own `55AA` signature.
Another local PC110-EMU `Disk1.img` is 8,011,776 bytes (SHA-256
`990EB1727F5F3F542C4D1E124A8CA37A1480B1C4A0B59FCCE0029407EC6AB0A1`),
not the documented 4,030,464-byte soldered SunDisk; a `55AA` signature alone
does not make its partition-table interpretation valid. It is not substituted
for the missing master medium in this observation.

Open-Source-PC110's live ATA record identifies the real soldered 123×2×32
SunDisk as primary master and a removable CF as primary slave. The current
PC110 plan explicitly fixes the master geometry and connects a second ATA
slot. The existing Machine media owner defaults an image in that second slot
to 16 heads and 63 sectors per track. This difference is a testable geometry
question, not yet proof that it causes the observed BIOS boot outcome.
The existing ATA controller also implements guest command 91h to set the
selected unit's heads and sectors per track; the actual BIOS command trace
must be checked before attributing any mismatch to the initial geometry.

The existing real-firmware harness now accepts `--ata-slave-boot` and assigns
the supplied image to slot 1 with the existing overlay mode. Slot 0 stays
empty; both slots have explicit overlay modes. The expected sector is read
independently from the same file through a read-only Storage lease. Success
requires genuine BIOS execution at 7C00h and an exact 512-byte match; the
host does not seed guest memory or replace the BIOS disk service. The x64
observation uses an 1,800-second wall limit, 300-second no-progress limit and
64 MiB output limit. The first x64 attempt reached real INT 19h and guest
floppy retries, then observed a BIOS drive-80h call immediately before its
old absolute 100-million-instruction containment. It exited nonzero at
100,051,694 instructions, without a Core fault and before observing ATA I/O
or sector entry. This is neither a disk failure nor a pass. Its log SHA-256
is `63A6C24B46426A292D6F1024DB7FED354A649D5DDC209A3E9C27D2C17DEED040`;
the source image retained its original hash.

For the new slave mode only, the diagnostic now allows 100 million
instructions after the observed INT 19h rather than counting the cold prefix
against that phase. This follows actual forward progress and preserves the
older master-mode bound. The new run remains bounded; no timeout is a success
condition and no guest timing or behavior is changed.

That continuation observed actual PCIC index/data operations, which exhausted
the existing shared 128-line boot-trace budget. After exhaustion, lack of
printed ATA operations is not evidence of their absence. The existing
diagnostic is therefore being amended so ATA I/O has its own bounded count,
separate from INT15/PCIC/IPL logging. This is observation-only and does not
change execution or add production state.

The x64 continuation exited nonzero after 164,980,303 instructions without a
Core fault or verified boot sector; its log SHA-256 is
`2B9D6C58034CF74022168E2324D0A9102F45D063415FA3C78CDC78140A80D3DC`.
Execution subsequently entered `5000:` code, the address range where S66
proved the real Easy-Setup program is loaded. This observation is consistent
with Setup being needed, but the bounded trace alone does not establish the
displayed page or the eventual disk-selection outcome. Further merely longer
waiting would not provide an input decision. The existing Setup diagnostic
will instead be combined with the slave-media diagnostic: validate the
actual guest-loaded Setup bytes, deliver the real Enter/F12 interactions,
and follow the post-Setup INT 19h to the actual sector checkpoint. No seed
rewrite or automatic production input is introduced.

The earlier x86 `--ata-slave-boot` run was deliberately stopped after the
x64 continuation identified the Setup path; it had not reached a terminal
checkpoint and is neither a pass nor a failure. Its partial log SHA-256 is
`C4D49584118B24069431DF8FB035B2E0258BF1B6C0D7AFA1D18AA0B451FC5C36`.
The owned process and parent were confirmed before stopping it. Both widths
now run the combined source in hidden, separately budgeted processes.
After those processes were launched, the diagnostic source was extended to
save the final guest frame in future combined runs. The in-flight binaries
do not contain that later observation change; their existing boot and ATA
checkpoints remain valid, but they must not be described as having saved a
final frame.

The existing X86-NXVM Machine construction test now also configures the
ordinary ATA board route, leaves the master empty, inserts one synthetic
in-memory sector in the slave's existing overlay, and reads its `1234h` first
word through task-file ports. Its first version incorrectly expected DRQ
before advancing Core time and observed status `80h` (busy) on both widths.
After one normal Core instruction services the pending command, the test
passes on x64 and x86. This proves the selected synthetic slave route, not
that the BIOS selects the external disk or uses its geometry. No host file is
created by this added check and no production receiver was changed.
The combined x64 run completed at 310,254,570 instructions without a Core
fault, but did not reach the independently checked boot sector (exit 1).
The actual Setup and post-reset INT 19h checkpoints were reached. The trace
after that INT 19h shows floppy attempts and PC Card register accesses, but
no accesses to the separately counted ATA task-file ports before the finite
observation ended. This trace does not cover all earlier POST ATA accesses.
Subsequent execution returned to the Setup code range; this does not by
itself prove a particular screen or the reason for the boot choice. The log
SHA-256 is `F5D77240CF072FFD2656629D3241951EC591B4516444DE14C2917AE978F09EAB`;
the external image still hashes to
`5149DB391D13CFEAB330016FCF0EDBE6B0D379CBB66A3AED91DBF7684142D52E`.
The x86 run has the same 310,254,570-instruction endpoint, observed INT 19h,
no ATA task-file access, and exit 1; its log SHA-256 is
`2C5DCEA95AD86EFFA031AA2951846B6755432E76222A234837846B97312A4E3B`.
These observations rule out neither an
unimplemented card/detection path nor a guest setup choice; they do not
justify changing disk geometry or reporting a successful guest boot.

The Personaware image is 128×2×32 and cannot be attached as the fixed
123×2×32 master: an actual x64 construction attempt returns
`LIB_STATUS_INVALID_ARGUMENT` before CPU execution. This is a media-size
contract result, not a firmware boot experiment. To isolate whether master
presence changes firmware discovery, a follow-up diagnostic attaches a
task-local, newly created blank 4,030,464-byte master and the unchanged
Personaware image as slave, both as overlays. The blank file's SHA-256 is
`1AB771116DF9FD1572AB44D2A8E18BC81A65C16DD2E58AD01A89545B93A70763`.
It is explicitly synthetic and cannot stand in as the factory boot disk;
the only question is whether its presence causes ATA access. The x64
comparison is running within the existing finite budget; its first ERROR
frame and date/time frame are byte-identical to the slave-only frames
(SHA-256 `02546EFBDCDF7D56C4668196232C5D9F29919E28F1EFF759D2C61C36450625EB`
and `FC147A26D8D1211E2C519A19E83D799B4227B716F44DF706EC9A547F6C253C0C`).
An x86 comparison uses a
separate blank master with the same hash and adds bounded ATA-register
observations during POST as well as after INT 19h. The x64 binary was built
before this additional POST observation and cannot provide that evidence.
The x86 observation recorded the BIOS identifying both units, issuing ATA
91h and then issuing Recalibrate 10h to each; each 10h returned status 41h
and error 04h (ABRT). The existing ATA-PIO command dispatcher supported
Recalibrate only for the WD-specific controllers. Both pre-fix comparison
processes were deliberately stopped after identifying this reproducible
command failure, not counted as complete test results. Their partial logs
hash to `33EB00B46A9F8FC5E2EFA907CC468D5F0700C3A5EF507C5C5078A7CD68AB41B7`
(x64) and `350AA5E3390C86374773B7C1D8F44C9C3F2AA4EC7E1FFFF7B3FC9AE930FBDFAA`
(x86). The negative post-INT-19h observation must not be described as
absence of all BIOS ATA probing.

The existing ATA-PIO path now handles 10h–1Fh as non-data Recalibrate for
a present unit, updates CHS/LBA task-file outputs and completes through the
existing command scheduler and IRQ. An absent unit follows the existing
abort path. ATA-2 revision 4c section 7.27 specifies these register outputs
and identifies the device operation as vendor-specific:
https://arxv.wirehaze.ovh/ATA-2_X3T10_0948D_R4C.PDF. This does not claim
physical timing or a specific flash device's internal mechanics. The
existing HDC contract test now exercises master and slave, CHS and LBA,
command endpoints 10h and 1Fh, absent media, IRQ and no media I/O.
The targeted HDC contract and Machine construction tests pass 2/2 on both
host widths; both X86-NXVM manifests verify. Expanded HDC, board and Machine
regressions pass 7/7 on each width. The rebuilt x64 X86-NXVM test group passes
235/235 (233 unit, two integration checks); its log SHA-256 is
`67283D880C22C84E4E8E81D2DCEC38F2B30AE387D07231F917592DCF980E7D14`.
The rebuilt x86 group also passes 235/235, log SHA-256
`2A77793B8A5906ADF1668FE8E7B874BD10F9AA342C14A76DA37E77C38FA7EA85`.
A real-firmware x64 pair run with the
fixed controller has observed the same two BIOS 10h commands now each
complete with status 50h instead of the old 41h/ABRT, on both host widths.
The fixed x64 pre-Enter ERROR frame displays 00173 and 00163; 01781 and
00174 from the corresponding pre-fix frame are absent. This validates the
command receiver and the observed page difference, not the disk boot. In the
x64 post-INT-19h trace the BIOS subsequently
selects the blank master and issues READ SECTORS 20h, receiving DRQ status
58h; earlier without Recalibrate it did not reach this observed ATA access.
The x64 comparison stopped at its 100-million-instruction post-INT-19h
bound without a verified boot sector (304,650,223 total instructions,
`PC110:POWER:result=0`, no Core fault); it did not observe selection of
the slave. Its log SHA-256 is
`CCC727AA320A42AA6960EDE1CA46F2FE8485AB8915B76B5D3AA4F0C459032494`.
The harness runner's process exit-code capture produced an empty value,
so the recorded diagnostic result rather than that wrapper is authoritative.
The requested final graphics-frame capture returned INVALID_STATE because
the current output was text; no final picture is claimed. The x86 comparison
likewise reads the master and terminates at the same instruction count and
unverified boot result without a Core fault; its log SHA-256 is
`A795F2AEB234860A58B0590A506F36C0848359502E5D757F73615EFA87939818`.
A fixed-controller x64 slave-only run has been started separately
to distinguish an unbootable blank master from the BIOS's slave policy.
Its early POST trace selects absent master A0h and reads status 50h, then
IDENTIFY returns 41h/ABRT; selected slave B1h returns 58h to IDENTIFY. The
ATA implementation holds a controller-wide status and only the separate
Compaq WD protocol refreshes idle status on device selection. Whether and how
ATA selection should represent an absent device requires further contract
and guest-path verification; the observed shared status alone does not prove
the root cause of the boot outcome.
An independently bounded early-POST trace (`TRACE_CAPTURED_NOT_BOOT_TEST`,
log SHA-256 `A9F17F11E006AC30F6EE33D06F308904B75556A599A4A7B3EF20AB424D8F0A00`)
shows the absent master IDENTIFY abort reaching the BIOS error return, while
the slave's IDENTIFY reaches the BIOS `REP INSW` completion and returns with
zero status. Thus the failure is later than merely receiving data-ready; it
does not yet establish whether the BIOS rejects the identification fields,
requires a master, or makes another configuration decision.
The follow-up 128-instruction trace after the slave IDENTIFY
(`TRACE_CAPTURED_NOT_BOOT_TEST`, SHA-256
`9EA071A94D8A6E1D95980251EBFE7AF7CE6E66EA1CFF2BCA345BCAC0D178FC70`)
shows the BIOS copying the returned 9-cylinder/16-head/63-sector geometry,
then requesting BIOS disk 81h while BDA disk count is 1. At F000:D3B2 the
BIOS compares that count with the zero-based drive index 1, and branches to
its invalid-drive return. Thus, with only an ATA slave attached, this BIOS
has accepted IDENTIFY data but rejects the corresponding BIOS disk number;
the observation does not justify rewriting the ROM or fabricating disk
count. Real-hardware reference has a permanent ATA master, so a realistic
two-unit boot medium and the firmware's actual boot selection remain the
next receiver to investigate.

The local Personaware image is a rebuilt 128×2×32 FAT12 disk, rather than
the original 123×2×32 physical master. A separate task-local test copy is
created at the physical capacity, preserving all boot code and file data:
the standard partition length and BPB sector count are changed from 8160 to
7840, the ending CHS is updated, and the unused tail is removed. Both FAT
copies were identical before conversion; every FAT entry beyond the last
cluster of the reduced volume was verified free. The source image remains
read-only and unchanged. The resulting 4,030,464-byte test disk has SHA-256
`3CD463CC5D05D17F4709DA98F9E18F457BF7CCE279A78BDACC12ABC2DA49598A`.
This is a clearly identified derived test disk, not an original factory
dump; it is neither committed nor distributed. Both host widths are now
running the original BIOS with this image as the master and comparing the
guest's 7C00h bytes to its own independently read MBR. Reaching the MBR alone
will not be labelled a DOS or Personaware desktop success. These in-flight
diagnostic binaries include the temporary IDENTIFY-follow tracing used above;
that observation-only instrumentation was removed from the source after its
evidence was recorded. Their binary boundary must be distinguished from the
subsequent final source when reporting validation.
The x64 single-master run completed its bounded observation at 310,217,303
instructions without a Core fault or ATA boot-sector handoff; its log hashes
to `1F30B14CA2E456E399B91A36FA92025FB18B5ABA5AE9C2AA5F971EBC96DFCFDB`.
Its final real guest frame displays configuration error 00174; the initial
frame displayed 00173 and 00163. The x86 counterpart reaches the same
310,217,303-instruction endpoint with the same diagnostic result and no
Core fault; its log hashes to
`9D22A7D5FE88C21FAB695EC88DD8196D4F2ECA41D53B0EE210A1958094FE7A54`.
The final x64/x86 guest frames are byte-identical, SHA-256
`BD4E20357777794AB532A591932845F96D16438B96491ED25D5FCA8CCC6FE14B`.
For comparison, a new x64 pair run uses this derived test image as primary,
the unchanged source image as secondary, and reads the expected MBR from the
primary; both are overlays. This represents a two-device test configuration,
not a claim that either is a physical factory dump.
That x64 pair run now passes the exact sector checkpoint: the unchanged BIOS
executes the primary's MBR at 0000:7C00, the captured 512 bytes equal the
independently read primary sector, and the diagnostic reports result 1 with
no Core fault at 256,579,379 instructions. Its log SHA-256 is
`745B19C518EC6EFCD20E7A773FE2BDD6CC8F89DA8635D300BE56E364893324E3`.
The x86 run with the retained diagnostic source also reaches and compares
the full primary MBR successfully at the same instruction count, reports
result 1 and no Core fault. Its log SHA-256 is
`9B194E3C185A01DFD614894D8791FCD358EA4B201AE9F31CD3BE051EC4A340FD`.
Both the original source image and the task-local derived image retained
their documented hashes after the runs.
The x64 diagnostic binary included an unused optional acknowledgement branch
and preceded a later reset-watchpoint correction; neither path was taken in
this run. The retained source removes the unused branch, and the x86
verification uses the retained diagnostic source. This is BIOS-to-MBR
handoff, not a verified DOS desktop. On this evidence the
separate single-master acknowledgement experiment was deliberately stopped
before its acknowledgement, to prioritize x86 validation of the successful
pair; its partial log SHA-256 is
`D4F861FCFCDC6D84E3805A3B02314B4EBD9B88C69CD04A12DCF0DCEBE1A4C045`.
The stopped x86 single-master experiment was intended to record and
acknowledge the 00174 page using an ordinary Enter make/break, without
changing firmware or CMOS. It was stopped before that point and provides no
acknowledgement result; its experimental input branch has been removed from
the retained diagnostic.
The fixed-controller x64 slave-only run terminated at the same bounded
310,254,570-instruction endpoint with no boot-sector match or Core fault;
its log SHA-256 is
`90D767D7F92F70E227A8FEDB4F356E0CD4E3E2DEEC327D0A746F4037FBBD01E5`.
The final guest frame is a real graphics capture, showing error codes 01781
and 00174. No ATA task-file operations were observed after INT 19h; early
POST identified the slave but did not issue its 91h or 10h commands. The
unmodified media and fixed geometry constraints remain as above. This
isolates a firmware discovery/acceptance difference, not yet its owner.

The eight existing PC App artifacts have been rebuilt in the original x64
and x86 Ninja trees, with four profile selections per width. PE architecture
checks and each profile's product/INI identity integration pass (eight
rows). The identity log hashes are
`20D712FD8F196965DC513C4E981ED4EC9042BB79CD75319B29CCD56A0E5DF228`
(x64) and `B94C84B48E961FF4680542AC38AD3524A241079B463E496F89DCDEA882EAAC44`
(x86). Their paths, lengths, architecture and hashes are recorded in
[the S73 inventory](t580-s73-artifacts.json). No user INI or external disk
was changed. These are receiver checks, not full App or PC110 qualification.
After relinking, the default NXVM ATA DOS and hard-disk boot integration
receivers each also pass on both host widths (2/2 per width). Their log
SHA-256 values are
`588C485FCE50DFFA9439B8603AEF3496ACB32BE542A60CC2BDB357A075C6FBA0`
(x64) and `1A18CFF8F3D53032625CB8342E8F5C129B0ED2F36D3C7F7B8F137DE894AFD30E`
(x86). These do not replace the PC110 real-firmware boot check.
Both PC110 preview variants also rebuild for x64 and x86. The rebuilt PC110
group passes 14/14 on each width (12 unit and two CMOS Product integration
receivers). Its log hashes are
`6DD32B30B98D871790C79247D1335E5523FFB31B37C1AA0E94F57FC89F0AA29E`
(x64) and `AD4ADA5A04F4F519854312FE2710375E998778639398CDDA559D30827D9CC680`
(x86). Previews are not qualified or published PC110 deliverables.

Actual tracked C-source/test delta measured with `git diff --numstat`,
excluding manifests, documents and generated binaries: production HDC
+15/-0 (net +15); the existing HDC contract, Machine construction and real
firmware diagnostic tests together +154/-21 (net +133). The increase is the
device-mode regression and genuine BIOS/sector/observation path, not a new
production route or framework. The existing scheduled ATA command path,
media owner and interrupt owner remain unchanged. Temporary diagnostic-only
instruction-follow and UI-acknowledgement branches were removed after use.
Lib, Emulator, X86-Core, MyNES and external firmware/media are unchanged.

The actual pre-Enter Setup image contains errors 00173, 01781, 00163 and
00174, followed by a date/time page; the two Enter events do not demonstrate
that all Setup configuration has been completed. IBM's general
[POST error reference](https://www.ibm.com/support/pages/troubleshooting-post-numeric-error-codes)
maps 1781 to disk drive 1. This is a useful cross-model lead, not proof that
the PC110 firmware uses precisely the same diagnostic semantics or the cause
of this particular error. Pre-INT-19h ATA observation and the comparison run
are required to distinguish firmware discovery from subsequent boot selection.
The x64 initial busy-state diagnostic log SHA-256 is
`0CE1D23BDF07F060C7CD34A27AA9A3F2BFC9C242B937C186B6E11A4953558930`;
the passing registered-test logs hash to
`0DA68F2BEDFB9D65FB4F351EB4455EBA180E836CD35FEFE72D3197639601C72A`
(x64) and `A525DFB75B710373D766B8930A975F0C481F297FE2AA076CF5DAC5C5AF6BBF25`
(x86). The changed test manifest also verifies.

## Related display observation, not a storage conclusion

PC110-EMU commit `81235b5fdf7d17ae180fc5cb6148e44bcbb87b4c` labels
`Docs/images/easy-setup-bios.png` as its ROM-backed Easy-Setup screenshot;
that image (SHA-256
`86C600C4776F384749F5375BFF7C2315FA4101709897548D7A763B4B312AFF02`)
shows a rose/brown main area, consistent with the owner's color report.
PC110-QEMU commit `bae390ebf21665ec97860d28e26ec46f1fe8f4e9` separately
describes a blue-ish initial theme when it bypasses BIOS pre-entry initialization
and enters a host-extracted Setup image. Its workaround edits that extracted
image; that workaround is not adopted or evidence that our genuine BIOS path
has the same cause. The two references have different entry and rendering
conditions. Real product Window pixels and the firmware's theme initialization
remain an open display receiver, independent of this storage test.
The combined diagnostic's two pre-Enter guest frames have identical color
histograms across host widths. In each 640×480 frame, `BE9E96` occurs
113,591 times in the first and 177,692 times in the second; other dominant
colors are black, white and `710820`. These are raw guest-frame pixels before
native Window presentation, not evidence of the displayed Window's colors or
its physical panel. The owner's observed blue Window therefore remains open.

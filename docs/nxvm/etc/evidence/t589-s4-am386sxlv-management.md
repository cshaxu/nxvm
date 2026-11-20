# T589 S4: Am386SXLV system management

## Machine boundary

AMD 21020 Rev A, printed pages 5–6, distinguishes management memory cycles
from ordinary memory and I/O. The management fetch begins at FFFFF0h.
The two published save spans cover 244 bytes. The text reports 114
transfers, while walking both spans without internal holes takes 122 words.
The current L2 model follows the published ranges and leaves the gap
between the ranges untouched. The management
handler accesses ordinary memory with UMOV and ordinary ports with the
existing I/O instructions.
[AMD 21020 Rev A](https://www.amd.com/content/dam/amd/en/documents/archived-tech-docs/datasheets/21020.pdf)

The CPU bus now accepts an SMM bit alongside the existing memory access
kind. Machine uses that bit to call the board attachment's management-memory
pair, through the same memory transaction begin, commit and cancel path.
Observation reads use the same address space without beginning a
transaction. An unbound or declined management read or write terminates
with its own status, with no fallback into regular RAM or ROM. Ordinary
memory A20, reset overlays, cache controls and wait windows do not qualify
the management decoder or its delays.

The cached x64 and x86 builds of the existing Machine executor smoke passed
the new route case and the previously registered Intel SMM cases. The new
case sends a tagged access through the actual Machine CPU bus and checks
that normal RAM and managed storage return different bytes at 60000h.
It also covers observational reading, reset-overlay precedence, unbound
or rejected routes and transaction release.

## CPU entry and restore work

The existing interrupt arbitration accepts the AMD profile. It writes the
save spans before setting up the management entry registers. RES3 reads
ES:EDI into a candidate state and publishes it only after successful reads.
UMOV retains the management instruction fetch while the operand uses
ordinary memory. The software SMI opcode queues the same interrupt when
DR7 bit 12 is set. The successful RES3 transfer time is charged through
the existing event ledger; UMOV borrows the ordinary MOV cost. Both use
nominal timing.

The executor smoke now raises a real SMI input, fetches a synthetic ROM
vector at FFFFF0h, executes a handler in the management space, reads an
ordinary RAM byte with UMOV, modifies the saved AL and restores it with
RES3. The test checks that the gap is unwritten and execution returns to
the original CS:EIP. A second guest program uses MOV DR6/DR7 and F1 to
enter SMM, clears bit 12 in both firmware-observed save slots and reads
the restored debug registers back through guest instructions. Both
handlers send a byte through the ordinary port provider. The failure cases
reject one write during entry and one read during RES3, check the reported
address, cancelled transaction and unchanged resume point, then reset or
destroy the CPU. The RES3 failure case requests NMI with SMI and requires
the management entry vector to win.
[System ROM inspection](t589-s3-am386sxlv-word-bus.md#s4-firmware-handoff)

The public AMD sheets establish the spans, entry vector, instruction
encodings and I/O routing. They do not establish the complete field
layout. Keep unverified register slots and instruction timing at the
appropriate model level, with exact provenance recorded when implemented.
AMD documents that a related Élan part can accept a still-asserted external
SMI immediately after RES3. This supports checking held-level behavior
as part of the AMD restore work, with OB300 board acknowledgement and
IIBEN wiring deferred to the next T.
[AMD ÉlanSC310 programmer reference, §1.9.2](https://www.amd.com/content/dam/amd/en/documents/archived-tech-docs/programmer-references/20665.pdf)

The L2 save image uses reported legacy register positions and the ROM's
confirmed debug offsets. Extended preservation fields for CR2/CR3,
DR0–DR3, NMI and debug state are modeled in the second span. The exact
silicon offsets, complete undocumented RES3 layout and electrical order
within each span remain unverified. The related DXLV data sheet documents
the same ranges and a consistent 61 doubleword transfers.
[AMD DXLV data sheet](https://www.ardent-tool.com/CPU/docs/AMD/386/386DX/Am386DXLV_final.pdf)

## Related path review

Searched `read_memory\(|write_memory\(|provenance == X86_CORE_CPU_MEMORY_ACCESS`
in `src/x86-core/cpu` and `src/x86-core/machine`. Data, instruction fetch
and page walks share the same CPU bus tag selection. The three direct
descriptor timing observations now use that tag for side-effect-free
reads. UMOV overrides the operand space while its fetch and page walk
retain the management route. The Machine debug physical-memory API still
inspects normal system physical addresses.

Searched `_smm_enter|smm_active|set_smi|case 0xf1|_amd_umov_opcode` across
CPU execution and the executor test. Hardware and software entry use the
same arbitration. Opcode admission is shared by execution and lexeme
preview. Intel RSM retains its own state format on the same bus.

## Qualification

The final executor smoke, including provenance and a concurrent NMI
assertion, passed in both cached trees. All fourteen receiving builds
completed and matched the recorded PE and SHA-256 identities below.

The implementation changes nine X86 Core source files (+307/-59),
the existing executor smoke (+228/-1), and the source/test manifests
(+10/-10). AMD management operations retain modeled timing. The T589
hardware tests do not exercise the original system ROM end to end.

The x64 aggregate found two stale x86-nxvm fixture expectations, each
registered twice. The profile closure test assumed the last valid
processor was 80486SX. The opcode matrix expected 0F 07 to be
unallocated on every profile. The fixtures now recognize the exact
Am386SXLV allocation and also execute 0F 07 on the Intel 80386 protected
path to prove that it still delivers #UD. The follow-up changes affect
only the two tests and their corpus manifest. Core code and the verified
receiver binaries are unchanged by this follow-up.

Final unit coverage was **716/716 on x64 and 716/716 on x86**. The x64
aggregate hit its 300-second deadline after reporting 712 outcomes.
The x86 aggregate completed in 237.53 seconds. On each architecture the
same four registrations failed due to the two fixtures above. The
follow-up kept all passing outcomes whose inputs were unchanged, rebuilt
the four affected test targets, and ran the exact complement of the
updated inventory with corpus and line-ending checks. The x64 result
combines 704 unchanged passing results with 12 passing follow-up tests.
The x86 result combines 708 with 8. Individual timeout budgets were
unchanged. Source and both affected test corpus manifests verified.

Test-only follow-up delta: fixture sources +8/-4, manifest +2/-2.
X86-NXVM test manifest SHA-256:
`e6aff8895e8d1e3721a42c55b4c2bf45b65e661ab1ebc11c3c2b76aae53e909d`.

## Display handoff

Inspection of the admitted System ROM 1.0 ABA common-memory dump confirms
a 32 KiB VGA option ROM at dump offset 7E0000h. Its 55 AA header, declared
length and byte checksum agree. The 65510 text at 7E0091h belongs to
that option ROM rather than an unrelated file in the card image.
Disassembly shows real indexed I/O in the image: an OUT AX at option-ROM
offset 0ABFh uses port 3D6h, and the routine at 0F17h selects XR0F and
reads an index/data word through that same port. The 65510 preliminary
data sheet, rev 0.7, page 74, defines the matching index/data ports
and the 90h-family XR00 ID. The PC110's current 65535 model returns C1h
and belongs to a different controller. The option ROM and programming
manual support a 65510-compatible receiver, with the board chip marking
and stepping still to be established. The board receiver will need
correct lane order for paired index/data word accesses.
[C&T 65510 data sheet](https://www.xn-www-sd4eq5d.vgamuseum.info/images/doc/chips/f65510.pdf)

The ROM also performs a read/modify/write of XR52 bit 3 at 0F59h, matching
the documented panel-off control. A nearby path reads XR59, the documented
vertical line insertion register. XR0F is software flag storage, so it
needs read/write persistence without a synthetic hardware action. HP's
product comparison specifies a 640×480, 16-gray LCD and no external VGA
output for OB300. These give separate register, panel and guest-frame
criteria for S5.
[HP service manual 5965-0244](https://www.elhvb.com/mobokive/edwin/laptops/hp/omnibook%20300%20425%20430%20530.pdf)

The code at 0ED3h also writes XR0B with a value whose extended mapping and
divide-by-four bits depend on the firmware path. The data sheet describes
the XR10/XR11 bank bases and XR0C's upper display-start bit when 512 KiB is
installed. The reusable VGA core already owns the standard latches, DAC,
text and planar capture, but its planar backing assumes four 64 KiB
planes. S5 must prove any needed extended mapping against the same backing
and rendering path. A register-only shim that reports XR0B without changing
guest addressing would be insufficient for that firmware path. Board
decode of a linear aperture and the physical panel wiring remain separate.
[C&T 65510 memory mapping registers](https://www.nrjfwax.vgamuseum.info/images/doc/chips/f65510.pdf)

Two scratch probes linked the existing libraries on x64 and x86. In the
general VGA core, standard write mode 0 succeeds and GR05 reads back 03h
after selecting mode 3, but the aperture write then returns UNSUPPORTED.
The 65510 data sheet, page 62, defines mode 3 as a masked Set/Reset
write. The existing C&T 65535 VGA implementation passed the same
mechanism on both host widths: latch 35h, effective mask A0h, set plane
color one, and read back B5h. It also already uses 512 KiB and captures
characters and fonts from the live planes. The 65535 standard VGA
mechanics are therefore a stronger sharing candidate than the older
general VGA core. The 65510 still needs its own register map, XR0B/XR10/
XR11 CPU banking, XR0C display start and panel behavior. The 65535
PC110 identity and extension semantics are separate.
[C&T 65510 data sheet, GR05](https://www.nrjfwax.vgamuseum.info/images/doc/chips/f65510.pdf)

An independent x64/x86 decoder prototype matched seven aperture cases
and two decline cases from the XR10/XR11 mapping table. In dual map zero,
the boundary is AFFFFh/B0000h. In map one it is A7FFFh/A8000h. Maps two
and three retain a single bank. Bank 40h represents 10000h bytes per
plane and 40000h packed bytes. The implementation needs to resolve the
bank boundary before applying the common lane and latch logic. The
prototype does not yet exercise shared backing or board linear decode.

## Board handoff

The original reset entry at F000:E05Bh accesses FBh first, then selects
ECh/EDh index 18h. It preserves the data value's bit 3 when setting
bits 5, 2, 1 and 0, then branches on bit 3. When that bit is clear it
continues to the first write to the standard POST port 80h. When it is
set, the code checks a signature at linear 60210h and far-jumps to
6080:0000 on a match. This narrows the proof needed by the board task:
the same reset-vector image can select the cold or management path
from chipset state, and the management window must resolve correctly
before the handler's first ordinary instruction fetch.

VLSI's October 1992 VL82C316 manual uses the same ECh/EDh index/data
ports. Table 40 at printed page 79 assigns index 18h bit 3 to the
read-only SMM active signal, bit 5 to translation, and bits 2–0 to
the protected range. Figure 26 describes translated management cycles
in the 60000h region and normal CPU accesses to the same address.
This is a concrete decoder candidate. Exact OB300 chip markings,
reset straps and the purpose of FBh remain open.
[VLSI SCAMP II data manual](https://bitsavers.computerhistory.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf)

## Receiver artifacts

Source manifest SHA-256: `c1d54d47f65a127e0f279b5a95aad1cc371983a34ae4dcb2a8435e76b5ebbfb8`.

Test manifest SHA-256: `5bc35b992847d36b28096eb8a444e4e7621cc224b4c5b6ac1b07e9a72bd3b2ce`.

All PE machine values and file hashes were independently verified against the publication log. Existing receiver versions were retained.

| App | Host | File | SHA-256 |
| --- | --- | --- | --- |
| my5160 | x64 | my5160_0_5_0582_x64.exe | dda572d8694b9282fe0bc94f126383b1b534f7889c2fce8274b404348710aeb8 |
| my5160 | x86 | my5160_0_5_0582_x86.exe | 007e74d05fa3c894711f58ac8fa43e55f2705b4adb8ae0a848c19f660bbde0ad |
| my5170 | x64 | my5170_0_5_0582_x64.exe | 3a96fcaa86659f0816da44efeb7a938d175d2c6f56bd93fd559a656a65c5c21a |
| my5170 | x86 | my5170_0_5_0582_x86.exe | a36ced0beb4740870c4ec722135f43456f54a29841ef09d9f8a4dfb54f2ba06b |
| mydeskpro386 | x64 | mydeskpro386_0_5_0582_x64.exe | 313a6595440af4bcc586f51af50a43f766b79a62a7984715bff0c85103a0b4f4 |
| mydeskpro386 | x86 | mydeskpro386_0_5_0582_x86.exe | dfa8ba5dffc11cddb1b883082c3f969d618d0f112ea323ab27b756c88686c13a |
| mynes | x64 | mynes_0_0_0044_x64.exe | 205262e9ce302e7a3de378acff7b0fd900a952a53dd04ba951d63dfcf36d3d4c |
| mynes | x86 | mynes_0_0_0044_x86.exe | 559d5cee6b80c790b7f207a275c5fb20c053aee7bb83d7a5f98958ac9a18f55d |
| mypc110 | x64 | mypc110_x64.exe | 7f146f5b5f6c9ee73e991e221682454f6ce72f162172740e3e60498bca5cb080 |
| mypc110 | x86 | mypc110_x86.exe | 3cd6897b2bafd96806af4bdd4b927c44ba5b06a1ea14ada4fe4d83f59efd4d66 |
| myraidenii | x64 | myraidenii_0_0_0007_x64.exe | 048ba2180da43f65d8474d16b8d13abae6dc1e075f5ad7167e5f09e05747589d |
| myraidenii | x86 | myraidenii_0_0_0007_x86.exe | 0bf3dc9b5f33994ca2c9e8d1fa14a808bd6e0284c74d2f8bb17599e8f936c584 |
| nxvm | x64 | nxvm_0_5_0582_x64.exe | 814dfe50dd4b5325ec2818b3b0b332967b95c5c4dc403aa01e7f699c50c82dbf |
| nxvm | x86 | nxvm_0_5_0582_x86.exe | 6a1117a5563615e19ccde99ac16fccad119d288fa2ed53c8d619fc046996c93e |

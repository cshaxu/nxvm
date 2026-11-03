# M5 T585 S21 — PC110 COM1 design

The Open-Source-PC110 live dump and Pluto notes identify a FIFO-capable
16550-compatible COM1 inside the FDC37C665IR at 3F8h, with IRQ4 gated by
OUT2. This establishes a guest-visible receiver, not a host serial endpoint
or every UART timing detail inside the PC110 Super-I/O.
The register and FIFO model is independently derived from the standard
16550-compatible [TI TL16C550C documentation](https://www.ti.com/product/TL16C550C).
That later compatible device is not identified as the PC110's physical chip;
device-specific equivalence is L2, and nominal board-clock choices are L1.

The reusable X86-NXVM chip owns its register bank, FIFOs, interrupt priority,
loopback and guest-time serial scheduling. The PC110 board binds the existing
Core port routes and PIC IRQ4 and supplies its clock. No App identifier, port
base, host serial object or second clock belongs in the chip. Reset, read
acknowledgements, DLAB, FIFO enable/clear, transmitter completion, receiver
threshold and timeout must use one state owner. Unsupported or unconnected
physical inputs are represented as disconnected, not as successfully received
host data. Tests need both the chip and actual PC110 route, and the limits on
external serial, IR and modem remain explicit.

The nominal baud clock, transmit completion and receive-timeout timing are
bounded L1 estimates. In particular, the precise FIFO transmit-empty interrupt
delay is not asserted to match silicon. The chip has no external receive or
transmit delivery API in this batch; internal loopback exercises the guest
register and scheduling path without inventing a host connection.

The existing Board profile deadline and advance callbacks already share Core's
guest timeline, so no scheduler or worker is introduced. A complete S21
requires both widths, the Shared manifest and dependency checks, affected App
tests, full cached qualification and the fourteen current release artifacts.

## Implementation and in-progress verification

The X86-NXVM chip owns 16-byte FIFOs, divisor and line registers, interrupt
priority/acknowledgement, loopback and guest-tick deadlines. The board supplies
both the guest tick rate and UART input oscillator; PC110 binds 3F8h–3FFh and
gates IRQ4 through OUT2. No Lib or Emulator changes were needed. Existing
PC110 product composition executes guest OUT instructions and verifies the
received byte and the actual PIC IRQ4 request; chip tests cover FIFO threshold,
timeout, overflow, DLAB, transmit-empty acknowledgement, reset and different
input oscillator rates. Targeted tests passed on both widths.

The first full x64 and x86 unit runs passed 714/714, but preceded the final
change moving the UART oscillator from the reusable chip into the board
configuration. They are not final qualification. A pending x64 integration
run was intentionally stopped when that boundary defect was identified;
it is not counted as a passing or failing suite. The updated source is being
rebuilt and requalified. Native audio endpoints were unavailable in the earlier
isolated desktop probe, so that probe did not validate physical audio output.

The first final-source x64 unit run passed 713/714; the unrelated
`emulator.verifier-negative` timed out at its existing 60-second limit while
the suite ran eight-way. It passed separately in 20.49 seconds. This is not a
clean aggregate pass; reduce test parallelism for the repeat rather than
altering the established timeout.

With four-way test parallelism the final-source cached unit suites passed
714/714 on x86 and 714/714 on x64. The earlier timed-out aggregate and its
individual retry remain recorded above; the timeout was not increased.

The final x64 registered integration suite passed 47/48, including actual
PC110 firmware cold boot, resume boot and the guest UART/IRQ receiver. The
one failure was `lib.integration/native_midi_output`: the native default MIDI
output could not be opened on the isolated desktop. The same desktop reported
no native PCM endpoint. This is not counted as a MIDI pass or as proof of an
UART fault; the owner has directed that unavailable MIDI devices not block
the PC110 work. The x86 registered integration run also passed 47/48 with the
same native-MIDI creation assertion as its sole failure; the x86 process exited
with Windows code `0xc0000409` after that assertion.

## Release and profile verification

The cached product targets were built for both architectures. XT, AT and
DeskPro profile-specific integration rows passed 3/3, 5/5 and 6/6 on each
architecture. All fourteen assets have the expected PE machine width. The
deployed PC110 and default NXVM files match their current build files; MyNES
matches byte for byte. MyRaidenII matches the stripped cached build apart from
the PE timestamp and checksum and was not changed by X86-NXVM. The release
hashes below capture the reviewed files.

| Asset | SHA-256 |
| --- | --- |
| assets/nxvm/nxvm_0_5_0582_x64.exe | `370957dd7f233f84e005cc6014e3568cbd9dba31e7ba0b0b1b902bbc4ef0369b` |
| assets/nxvm/nxvm_0_5_0582_x86.exe | `d00a554391e4abcdbe3f8696ef23831eb31c9b410cb22aee14e62c3fcda56286` |
| assets/my5160/my5160_0_5_0582_x64.exe | `1e84d198db4a9461fd14d55af312f7a236416b07b6e3b8c1374f9efd8ccadef1` |
| assets/my5160/my5160_0_5_0582_x86.exe | `b2a0e441e411aaf0be931d5bfd9a12f0474b9a5069ea3625097d75c0b0b4bf1d` |
| assets/my5170/my5170_0_5_0582_x64.exe | `477eeaa71ab9c5d56ffc5c92517f7e475d7db244c5f5cb9f4e03aea51c5a2b20` |
| assets/my5170/my5170_0_5_0582_x86.exe | `7c2ee2cdb946354a67f0f14fecfca3537249a8486342ab993b9a75835fef7988` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe | `083a638d343ae12237725603ac80e43bbeaad90ba927a086a9871baa507092c6` |
| assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe | `3147325ef2a9dbdd5b3a6f23e4f6eb59ae8ecd597e7e08113a492f713d8c0109` |
| assets/mypc110/mypc110_x64.exe | `c2531f73b19939b94a3c3cdbc2324977503d631e0e12d7ec6e115e5565938ac9` |
| assets/mypc110/mypc110_x86.exe | `be729f7284655ee4542c28ff0c8698a5a19f230d93c03dcf4db8797c7fccc8f1` |
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | `3301ca497b8c5977ec1acab2f7bed736532331b1cb9c8e9a673cbc93633dd3c8` |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | `7ee0475d785632b4d65b9e7cc99b2e4db427fa9690756f8952ec437000db186c` |
| assets/mynes/mynes_0_0_0044_x64.exe | `b499cd2583ecddd1ae01eed882e21d49d20187558e2138858e5a27117cf74fc8` |
| assets/mynes/mynes_0_0_0044_x86.exe | `718617af2fbf6d49c36ae3bb10043dc3b2383e35440e7a371e503b8bd4b4ea16` |

Before removing task-local scratch, the log SHA-256 digests were:

| Run | SHA-256 |
| --- | --- |
| x64 final-source unit, first attempt | `9947e361dd7ed9b40260a366ea8d5ec6006aa3aba8ec060b5ddbe9ff62e229f1` |
| x64 final-source unit, four-way repeat | `ebc402672ef1bdf6a8ad122194f356d172d7b8dc003b92af5b43390f429b498f` |
| x86 final-source unit | `619782891f6df436ccc9c42ac466eab3a58cdf4c3496284a43b4008b2390e817` |
| x64 final-source integration | `8f9ddecda81f370843b45f7ba9708f7470fa557e4967596e7e3136f461b85950` |
| x86 final-source integration | `d798439a44732f11419e3034baeee8cee6194813daaf2410484ab23b116dea61` |

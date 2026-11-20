# T590 S1 OmniBook 300 reset and chipset decode

The supplied System Card 1.0 ABA common dump remains immutable. Its SHA-256
is `b98001a30b79f126483f823fd4d7a5ce91a973a5ab8fb35e3d76043b74a8e099`.
The probe uses only the upper 32 KiB of the candidate system BIOS
at dump offset 7F8000h. The selected span has SHA-256
`5e6a2c44bfeae3011926fffd40ec26c7f43d3389606d501b1d8b64294f50821b`.
It maps to F8000h–FFFFFh with a reset alias at FF8000h–FFFFFFh. This is
a deliberately narrow proof, independent of the disputed remaining
System Card, VGA and shadow windows.

## Firmware behavior and hardware evidence

| Observation | Implementing owner and qualification |
| --- | --- |
| Original reset jumps to F000:E05Bh and writes FBh at E05Eh. Other firmware routines repeat the access immediately before operations on ECh/EDh, including the loop at 8E05h. | VLSI SCAMP II p. 96, Table 59, identifies FBh as a write-only configuration-enable strobe. The payload is immaterial. F9h disables configuration writes. MISCSET bit 7 disables the special gate itself. This is a matching device contract, pending physical chipset marking. |
| E060h reads the selected register, E066h selects index 18h, and E068h–E06Eh read, update and test the data. | ECh is the readable 8-bit index latch. EDh is the selected data port. SCAMP II p. 79, Table 40, defines bit 3 as read-only SMMACT. Feed it from the existing CPU management-active signal; do not guess from the write value. |
| Cold reset writes POST 00h to port 80h and then touches CMOS 70h/71h and KBC status 64h. The branch via A65Fh programs master/slave PIC ports 20h/21h and A0h/A1h. | These reachable accesses identify the early device routes. The trace's POST marker ends before those later I/O operations. The next board batch supplies the actual AT-compatible device owners. |
| Management reset tests the signature at 60210h and jumps to 6080:0000 when it matches. The no-signature firmware branch reads EFh. | Read-only SMMACT selects the branch. The VLSI manual documents EFh as fast CPU reset. Production needs defined management-memory translation and normal-memory separation, including the BASIZ selection and the management CPU bus path. |
| The loop following 8E05h writes 23 distinct indexed entries sourced from F000:8DBAh. | S2 must classify readback and effects for the initial register batch, in particular indexed power/management timers. Guest writes remain the authority for their programmed values. |

The [VLSI SCAMP II data manual, October 1992](https://bitsavers.computerhistory.org/components/vti/pc/VTI_VL82C316_VL82C323_SCAMP_II_199210.pdf)
also describes the OSC/12 82C54 inputs on p. 77. The HP service manual
5965-0244 gives the OB300 CPU nominal clock as 20 MHz, distinct from the
25 MHz 425/430. Use the existing single Core time axis with a
SCAMP-compatible nominal PIT clock in the first model. Wait states and
strap-specific controls still need receiver evidence.

## Actual executor probe

The cached x64 and x86 X86 Core libraries execute the original reset bytes
under the 386SXLV profile. The probe starts in 2 MiB with FPU absent, takes
one instruction per iteration, and admits up to 64 iterations. It maps
only the reset ROM range, binds a byte-only provider for the four observed
configuration/POST ports plus F9h, and declines unsupported accesses.
The scratch SMM provider maps the ROM reset fetch and a private 4 KiB
range at 60000h. On the cold path, index 18h starts at the documented 20h.
On the management path the probe enables SMM and seeds the expected
signature as fixture setup before asserting Core's SMI input. The
production machine must establish the signature through firmware.

| Route | Observed on both host widths |
| --- | --- |
| Cold | FBh write, ECh read/select 18h, EDh read 20h/write 27h, POST 00h. Final PC FE097h. |
| SMM | FBh write, ECh read/select 18h, EDh read A8h/write AFh. The write latches A7h, with the active bit read-only. Final PC 60800h with no cold POST write. |

This proves original branch selection through the chipset boundary.
The firmware's creation of the signature, final ROM mapping, BASIZ/shadow
routing and full POST remain unverified. Tests do not infer exact physical
behavior from the probe's private SMM backing.

| Reproduction artifact | SHA-256 |
| --- | --- |
| Scratch C source | `69e51f5ea44d5d6c13d3c5fb01f59c4c2fd13f6b1f64f1e82b33584baf39d88e` |
| Scratch runner | `e1cf9ccbde07106c16defd5532fb508bea8a60eb7b64bbe6b3512b87f7c54b60` |
| Dual-width result | `51132fba99d7cf94146f355c2355137d82c73d20a10375c477d4a7ec5e681976` |

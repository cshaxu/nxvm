# M5 T585 S3 — PC Card status-change interrupt receiver

Ricoh manual EA-028-9804, printed pages 21 and 29–35, defines status-change
event enables, ISA IRQ selection, read/write acknowledgement and the
card-detect resume selection. Open-Source-PC110's Live-Dump identifies PCMCIA
IRQ5/IRQ10 use; this corroborates the board route but does not establish
electrical timing for all selections.

The independent RB5C396 now reports each slot's selected ISA status IRQ from
its existing register state. The PC110 adapter pre-binds two existing PIC
source leases, one per socket, and synchronizes them after register accesses,
input changes and reset. The PIC's existing wired-OR ownership handles two
slots selecting one line. Software card-detect generates the existing detect
status; selecting detect-resume suppresses that status on ISA IRQ. No second
queue, scheduler or PIC is introduced. INTR#/RI_OUT# signaling, ready/battery
events, actual card insertion UI, windows and media remain outside this
receiver and are not claimed implemented.

Initial x64 focused tests covered chip mask/selection, reserved IRQ, software
detect, resume selection and acknowledgement; the actual PC110 bus and PIC
test covered IRQ5, IRQ10, two sockets sharing IRQ5, deassertion and reset.
Product composition also passed. The shared chip and its tests were reviewed
and committed as `df12230c9`; the PC110 adapter and product artifacts follow
in the NXVM-scoped delivery.

During qualification, an initial x64 full-unit run was voluntarily stopped
before completion to add the missing board-level pending-IRQ reroute assertion;
it is not counted as a pass. The subsequent complete x64 and x86 runs each
passed 712/712. A further test-only assertion for reset with an asserted IRQ
was then added and its PC110 test rebuilt and passed separately on both widths;
production source did not change after the complete unit runs.

The first x64 48-case integration run passed 46 and failed the default-profile
INI and boot-matrix cases with `OPEN-FAILED:1`. Following earlier profile
switches, their test executables had not been relinked for the restored
default profile. Both were rebuilt and each passed on rerun in the same
isolated desktop (and in the ordinary background path). The original failure
is retained here rather than reported as a single 48/48 first pass.

The x86 registered integrations passed 48/48 in one isolated-desktop run.
For each width, the selected XT, AT and DeskPro integrations passed 3/3,
5/5 and 6/6 respectively. Both stable build trees were restored to default.
The selected-ROM MyRaidenII startup and product-Console probes passed on both
widths; these bounded probes do not establish gameplay. Cached Release
builds covered all seven product pairs, with PE width and artifact hashes
checked below. MyNES and MyRaidenII builds remained byte-identical; ten NXVM
family binaries changed. Original owner INIs and media were not changed.

## Released executable identities

| Product artifact | PE width | SHA-256 |
| --- | --- | --- |
| my5160/my5160_0_5_0582_x64.exe | x64 | `8c4f7ec325d3ff7d89cdb08b92ea701209208cc0aa5a5a02c9ed9cb30a50afb6` |
| my5160/my5160_0_5_0582_x86.exe | x86 | `050ebc068e75e4fe265abe0e890e49f2b3b8e8e6255ba25ba14468252d445d26` |
| my5170/my5170_0_5_0582_x64.exe | x64 | `f058450ddfce2b2b4fea4528c5568f77c6371932e76c7ad8edd35c2ea87354fa` |
| my5170/my5170_0_5_0582_x86.exe | x86 | `ba18f2a1c0ea741d63aa08708995f6fc4815b9330038c9c46fc64e3ed28ba39c` |
| mydeskpro386/mydeskpro386_0_5_0582_x64.exe | x64 | `3cbead0f90661dcdf0683ca162859c989b45fc115db5ef4280c0001684eea0c1` |
| mydeskpro386/mydeskpro386_0_5_0582_x86.exe | x86 | `f4ff5a7f236013f83466b575cc1be10ec3369dc59b0efe8c957fe93c4b641ccd` |
| mynes/mynes_0_0_0044_x64.exe | x64 | `b499cd2583ecddd1ae01eed882e21d49d20187558e2138858e5a27117cf74fc8` |
| mynes/mynes_0_0_0044_x86.exe | x86 | `718617af2fbf6d49c36ae3bb10043dc3b2383e35440e7a371e503b8bd4b4ea16` |
| mypc110/mypc110_x64.exe | x64 | `f364af7aa3179d71fd99053d6f1ad1596ed469e5f6fdcdbc72940e02aa6cba49` |
| mypc110/mypc110_x86.exe | x86 | `e65bd5118ebf31f00a18260f1702784fb3a79e36e97bf5151b4332462425116a` |
| myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `0e48ffde257e0f6094ed5dbccb3b03c5d2ea7e7fe8a01a0f27a11df1659fc637` |
| myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `5d0181af40715d47630fe3feffbc7bbf2bff1aaaad3825e8d5e95fcfec99f650` |
| nxvm/nxvm_0_5_0582_x64.exe | x64 | `48f37dcf420e2c9f262e0553f16e8cce976e0d38a8d58d186c138370f107e425` |
| nxvm/nxvm_0_5_0582_x86.exe | x86 | `649ec9df6dae0140c10d21565d80d53889a9444c3eb9f210aa878525c895bc1b` |

# T592 S2 — OmniBook native boot and release qualification

## Original ROM DOS through the Product

The ordinary x64 and x86 candidates loaded the exact external System 1.0 ABA
card and a disposable active FAT16 C volume. The volume used the owner's
unchanged CONFIG.SYS and a test AUTOEXEC that printed a DOS marker instead of
launching Windows. Overlay mode left the underlying image byte-identical.
The raw Console received a complete F1 and VER make/break sequence using
Win32 input records with physical scan codes.

| Original card, ordinary candidate | x64 | x86 |
| --- | ---: | ---: |
| Original firmware F1 prompt | 36.672 s | 43.500 s |
| DOS C prompt | 70.328 s | 83.297 s |
| DOS VER output | 70.765 s | 83.703 s |
| Boot probe SHA-256 | `abb9228572430fde31cf86feef7e25ce123033e5eed4bcf177660f97ea32fd1f` | `d558e88bfd0a9671714cfbb99ca961bbe6b77add1768f3db1fe05a0ab405ddf5` |

These runs validate the native path and key delivery. They do not replace the
controlled performance baseline or qualify the default Windows autostart.

The monitor command probe used the unmodified Product with a disposable disk
and verified info, start, pause, resume, reset/resume, stop/start and exit on
both architectures. The first monitor probe omitted the command terminator.
Supplying CR let the existing cooked reader submit the lines. After guest
presentation switched buffers, a handle opened before activation still
observed the monitor. The native boot probe opens the active buffer anew
and scans its viewport. One x86 run initially timed out before seeing F1;
driver-frame instrumentation then showed the real POST and DOS text and
a separate ordinary executable run reproduced the complete boot and VER.
No production Console, board or CPU correction was needed for these probes.

The first isolated x64 Window observation found only the initial small
window. It did not qualify a full desktop Window session, physical keyboard
or RDP. Native Console results and earlier frame probes are listed separately.

## Publication

The build menu and existing publication targets now select MyOB300 by name.
The shared PC version declaration supplies both the startup banner and
the versioned PC filenames. The version change is the only X86-NXVM content
change. MyOB300 links the existing Machine and Product, with Release linker
debug sections removed. The external card, owner C archive and media were
never staged as repository contents.

The exact staged-purpose Release binaries with revision 0.5.0592 were run
again on the same fixture. The original card and disposable C volumes
resolved relative to a copy of each published executable. The final x64
and x86 runs executed VER, and the underlying C volume remained unchanged.

| Published build | F1 | C prompt | VER output | SHA-256 of probe log |
| --- | ---: | ---: | ---: | --- |
| x64 | 38.766 s | 77.781 s | 78.234 s | `33c995ea1fda968f8da53a0beec4d5e7578a6a5c46e560b9bf71ccca71da6ee5` |
| x86 | 50.703 s | 98.141 s | 98.594 s | `6d17892a7207e235430befa86fc500009b3136db72874ff480748bf8650cf982` |

The two runs exceeded the cold-start budgets of 60 s on x64 and 90 s on x86.
Performance remains the next receiver's scope. The purpose of this probe
was exact release startup and input, so its timings are not a throughput
comparison against another emulator.

The dual-width build ran through `tools/build.cmd all --no-pause` using the
existing Release Ninja trees and repository ccache. The x64 unit suite passed
716 of 717 initially. Only the LF check failed, identifying mixed endings in
the local copy of the T591 S4 evidence. The normalized bytes match the existing
Git blob. The same LF test passed on rerun. x86 passed all 717 unit tests.
The X86-NXVM manifest, documentation governance and diff whitespace checks
passed.

## Registered integration and deployment identity

Both stable Release graphs passed their full registered integration label
(48/48 each), including the registered desktop and native-audio cases.
The x64 run took 268.79 s and the x86 run took 284.88 s. The JUnit records
are identified by SHA-256 below.

| Result | SHA-256 |
| --- | --- |
| x64 | "0e0f1f4364900e792219e49e098b47b8f9807e361ba1a1732b4111fe231c775c" |
| x86 | "b56f93ece6aa02c2a678bb70f392f7ee711790f51f7497839fa83bc768f6a746" |

Shared release inputs are pinned by implementation commit ebd5fcc55.
The App linker option and validated PC payloads are in this delivery commit.

All eight deployment directories retained two executable files. The published
MyOB300 hashes match the files run in the original-card boot probe. Both
MyOB300 images have the expected PE machine type and no compiler debug sections.
The other products were rebuilt because the PC version declaration is Shared.
The previous PC release names are retained in Git history.

| Artifact | PE architecture | SHA-256 |
| --- | --- | --- |
| `assets/nxvm/nxvm_0_5_0592_x64.exe` | x64 | `7a9b90176750652ce5a62e4e4ff359779a483e8f1aa0404d84e5e439afd8898c` |
| `assets/nxvm/nxvm_0_5_0592_x86.exe` | x86 | `49411f2863ad773b022d45ebbdc0c8d8ed9aae4e4de814f93e5e726f25b0a894` |
| `assets/my5160/my5160_0_5_0592_x64.exe` | x64 | `f1260248c249d8c1a4c67a85cc83152f26ae5679a61e2864f18c533147bee6e8` |
| `assets/my5160/my5160_0_5_0592_x86.exe` | x86 | `b863a288c83f790198ddd04ec0299977c1a91a4c85f574cfb7c1bee0f1a859a1` |
| `assets/my5170/my5170_0_5_0592_x64.exe` | x64 | `c8b40a29bd16783f0e012f0d18b2ff324723e4fdf9c03dde71df1e9a8eb315eb` |
| `assets/my5170/my5170_0_5_0592_x86.exe` | x86 | `e7c504d4e2195599752175a23db8556ca4da9b5a283dbf83ab334b07fefc0aaf` |
| `assets/mydeskpro386/mydeskpro386_0_5_0592_x64.exe` | x64 | `e14d8b56c128cf94f3beb5d0a3a5100fafc0e73497e828d105b2f64a45fa35f0` |
| `assets/mydeskpro386/mydeskpro386_0_5_0592_x86.exe` | x86 | `56275b9f65cbd54e4b2522b327e74e9e874da7c37f67096686c875544b9c014c` |
| `assets/myob300/myob300_0_5_0592_x64.exe` | x64 | `7e97dc2a987e2223825b3a82ead25d4e6f71a6d24cbb2c5e7d2ba2a42f786c13` |
| `assets/myob300/myob300_0_5_0592_x86.exe` | x86 | `7b32695d77ed43ecf24380fe4c1c3d271cf142314cd7d2e8116832cc94cd769f` |
| `assets/mypc110/mypc110_x64.exe` | x64 | `592a1b53248ad89d1a0c7c1152a2a3c1dacb9abcb1aca3d93bd47f53ab5f314c` |
| `assets/mypc110/mypc110_x86.exe` | x86 | `e8eafac127f6f828797cf93855468580e9fc55af421b0cd0671693e801b84ae7` |
| `assets/mynes/mynes_0_0_0044_x64.exe` | x64 | `436d7d38d303fd40e6d380aa4575b23859201b373ad4bcb8ccaa47a1efc2b7f3` |
| `assets/mynes/mynes_0_0_0044_x86.exe` | x86 | `dbce3f0b98aa7974fbf0286df476a2c4201dc5914f49fa2b44af0e80e6bc47c6` |
| `assets/myraidenii/myraidenii_0_0_0007_x64.exe` | x64 | `943dcd38c9992c118c3cd4d121cce10fce2fb0ff5a44959d669398d351c83142` |
| `assets/myraidenii/myraidenii_0_0_0007_x86.exe` | x86 | `cb0649dd20defd42bdd92dfdf15514b4004f4736ee67a857b8d580a570a12700` |

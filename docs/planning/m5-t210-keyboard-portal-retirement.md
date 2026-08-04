# M5 T210: Keyboard Portal Retirement

## Goal

Retire the default profile's private `INT F1h` and `INT F3h` keyboard portals.
The real path becomes host event -> profile scan-code mapping -> core 8042 KBC
ports/FIFO -> PIC IRQ1 -> default-ROM `INT 09h` -> BDA keyboard buffer ->
default-ROM `INT 16h`.

## Contract

`core/machine/kbc` owns `60h`/`64h`, keyboard FIFO, command byte, and IRQ1
request. The default profile owns scan-code layout and ROM firmware. The ROM
owns BDA translation/buffer mutation and the supported `INT 16h` services.
Platform code submits host input only; it does not write guest BDA state. The
only retained profile-side keyboard callback updates session-local modifier and
toggle state already represented in the BDA. No private firmware portal may
handle vector `F1h` or `F3h` after S2.

The admitted ROM subset is set-1 make-code translation for the current text
path, `INT 16h` `AH=00h/10h` read, `AH=01h/11h` status, `AH=02h` flags, and
`AH=05h` store. Break/E0/E1, scan-set selection, translation control, LED,
typematic, AUX mouse/IRQ12, timing, resend, and errors remain the separately
governed advanced-KBC backlog.

## Breakdown

### S1: Contract And Port Probe

Record the owner chain and retain the focused core KBC port probe for `60h`,
`64h`, IRQ1, command byte, A20/reset, and FIFO bounds. Pair it with the
task-specific closure gate and S3 ROM/DOS probe proving BDA mutation and
`INT 16h` delivery; T209's core portal probe proves unregistered `F1h/F3h`
uses ordinary guest IVT semantics. DMA and a guest memory window are not
applicable; the BDA is firmware data, not KBC storage.

### S2: ROM Implementation And Portal Removal

Install real default-ROM `INT 09h` and `INT 16h` routines, move the admitted
translation/buffer subset there, and remove F1/F3 registration and dispatch.
Keep KBC device ownership, profile mapping, session isolation, and retained
Console/debugger behavior unchanged.

### S3: DOS/System-Image Regression

Run the focused probe and the current FDD DOS keyboard regression using `ver`.
Run the retained current matrix: FDD/HDD boot where applicable, FDD prompt,
Console lifecycle, debugger pause/unified debugger, two-session isolation, and
keyboard/VADP coverage. No external reference is needed; optional Bochs work is
not admitted for this task.

## Stop Conditions

Stop for a second keyboard/BDA state owner, direct platform BDA mutation, a
new host shortcut, profile mutation of core instruction tables, a private F1/F3
portal, changed Console/debugger/start UX, or any retained keyboard/DOS boot
regression. The task produces `nxvm_0_5_0210.exe` only after S3 passes.

## Verification Evidence

Applicable rules: core owns KBC state; profile firmware owns ROM and BDA
translation; platform submits only input; composition remains the sole product
assembly point; retained NXVM UX is unchanged.

| Surface | Verdict | Evidence |
| --- | --- | --- |
| S1 KBC port/IRQ contract | run | `core-machine-kbc-controller-smoke` passed: `60h/64h`, IRQ1, command byte, A20/reset, FIFO bounds |
| T210 closure | run | `verify-keyboard-portal-closure` passed: no F1/F3 portal or C callback remains |
| S3 DOS/system image | run | `vm-dos-keyboard-smoke` passed: default FDD prompt accepts `ver` through ROM `INT 09h`/BDA/`INT 16h` |
| Boot and prompt | run | `vm-dos-prompt-smoke` passed |
| Console and debugger | run | Console lifecycle, debug-pause, and unified-debug smokes passed |
| Session and input isolation | run | two-session, host-ingress, and keyboard-provider smokes passed |
| Display | covered by | retained VADP and DOS prompt smokes passed |
| DMA | not applicable | keyboard path has no DMA channel |
| Optional Bochs differential | not used | project-owned port and DOS evidence are sufficient |

`cmake --build --preset current-gates-gcc --parallel 4` passed all static gates
and executed 45/45 current CTest smokes. `cmake --build --preset current-gcc
--parallel 4` produced `build/output/nxvm_0_5_0210.exe` with SHA-256
`8390A8DC64D8A7118D77BA6F780594F071FB56E0FCEB48BB3F570D0F333E21E1`.
The verified artifact prints `Neko's x86 Virtual Machine [0.5.0210]` and enters
the retained Console. It contains no guest media.

The DOS regression sends ordinary make codes at bounded 50 ms host intervals.
Fast typeahead queue-delivery, break/E0/E1, scan-set selection, translation,
LED/typematic, AUX mouse/IRQ12, timing, resend, and error paths remain the
separately admitted advanced-KBC work; this task makes no claim about them.

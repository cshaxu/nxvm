# M5 T580 S43 — Real PIT calibration failure

## Reproduction

A temporary mode in the existing TFT diagnostic ran the unchanged BIOS from
reset without per-retirement callbacks. It flushed PC/CX/DX/time samples after
each million retirements, then single-stepped the final timer-test region.
The x64 run reached a deliberate CLI/HLT at F000:6999 after 14,363,519 retired
instructions and 95,948,144 guest ticks. It did not fault or remain in the delay.

## Decisive facts

- At F000:6958, BIOS flag 0040:006B was zero, IF was set, and PIC mask was FE.
- The real IRQ handler reached F000:EB67 with AH=01 and then set that flag.
- At F000:6981, AX=ADE1 and BX=FCE0. Their 16-bit difference is 4EFF.
- The following firmware checks admit only 1614..1868; the actual result fails.
- The final flag was 01, with PIC mask FF. IRQ delivery itself succeeded.

The BIOS compares PIT counter 0 with 320 refresh transitions. Current App
construction advances PIT at one count per Core tick and synthesizes refresh
at one transition per three Core ticks. Those choices are not a coherent
PC110 clock relationship. PC110-EMU declares a 1,193,182 Hz PIT input; the
Open-Source-PC110 Discovery/Chipset readme independently describes its live
18.2 Hz system tick from a 65,536 reload at approximately 1.193 MHz. The QEMU
PC110 research comments identify this same calibration, but force the result;
no such interception is admitted here.

## Corrected interpretation

S36/S38/S39 timeouts did not prove an interpreter-throughput blocker. Their
retirement ceilings do not advance while a CLI/HLT guest remains halted, yet
the diagnostic repeatedly advances to device deadlines. Thus a missing return
marker cannot identify which stage stopped. The BIOS also has an alternate
delay return at F000:5407, so 5405 alone is not an exhaustive exit predicate.

The next change belongs to the App's existing PIT clock and refresh-period
configuration, with an explicitly L2 relationship and real calibration replay.
It does not require a new receiver, execution engine or firmware modification.
The temporary diagnostic mode was removed after the facts were collected.

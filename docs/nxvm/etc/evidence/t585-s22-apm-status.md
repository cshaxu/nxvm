# M5 T585 S22 — firmware APM status

Both selected firmware variants were run through the real PC110 chipset to
the cold-boot INT 19h handoff on both host architectures. A temporary guest
program then invoked INT 15h AX=5300h, 5301h and 530Ah, with BX=1 for the
status query. The test did not patch the external firmware or write user media.
All four runs produced the same result: APM installation check succeeded with
version 1.00 and `PM` signature; real-mode connection succeeded; power-status
query succeeded with BH=01h (AC online), BL=FFh (battery status unknown) and
CL=FFh (charge unknown). These are observed emulator/firmware results, not a
physical battery measurement.

The existing EC block defaults put both externally observed AC indicators in
the AC-present state. Open-Source-PC110 `Discovery/Pluto/readme.md` at
`02ccf2e8e19b298a2cf04c5d430278eb087cc368` records those indicator polarities
and physical APM readings. The current model does not expose changing AC,
battery charge, charging, lid events or physical battery life; an unknown
battery value is retained rather than invented as a live measurement. The
earlier T580 firmware suspend/F12/RSM checkpoint remains a separate guest
power path and is not equated with Emulator pause.

The existing cold-boot integration was extended to execute and assert the
three guest calls after its existing handoff, avoiding another permanent
long-running boot test. The TFT-linked integration passed on x64 and x86,
with `PC110:APM:status=0` and 857 guest instructions in each. Temporary DSTN
probes also reached the same result on both widths. The first probe attempts
incorrectly tried to modify a debug watchpoint before the prior instruction
finished, or between calls at that same boundary; those diagnostic failures
were corrected by completing the boundary and issuing all three calls in one
guest program. They are not product failures or successful qualification.

No production source, Shared component or release EXE changed in S22. The last
full cached suites and artifact qualification remain those recorded for S21;
this S ran only its affected dual-width firmware integration. Interactive RDP,
physical audio and native MIDI availability are not established here.

Before scratch cleanup, SHA-256 digests were:

| Probe or test | SHA-256 |
| --- | --- |
| TFT x64 guest APM sequence | `48cd8af38738e97b402e361ae9455ed9d73dc2122cf6b8c4c407670811f2b451` |
| TFT x86 guest APM sequence | `46fe967d123b047b85ab943a436030e68edf2a37337b57acf30c58f440f48392` |
| DSTN x64 guest APM sequence | `8089e57818551e6f34229cb474893ca527ddcdd53fd921f6edb9b015518f3ef0` |
| DSTN x86 guest APM sequence | `772047714a13dd65f2fd5ad0048b68ef33f8f62e791ecd3139d70399f815659b` |
| TFT x64 extended cold-boot integration | `d3054de5e7c4307a5dc31520999c0bd16e5780aa6e7de72a9155e4ecc6876a8a` |
| TFT x86 extended cold-boot integration | `3a32e85b6c9f0c8ac5c82322b11712cac0cd806daa1e5a049ab5529d57d99d62` |

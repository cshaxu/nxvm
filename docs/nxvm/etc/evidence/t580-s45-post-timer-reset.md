# T580 S45 — Post-timer reset and wait classification

## Baseline and method

Baseline `95cd914ef` retains S44's App clock correction and the admitted,
unchanged TFT BIOS. A temporary mode in the existing integration executable
used the same normal Core run path, copied retirement observations and existing
port observations. It retained only the last 16 instructions, last 128 I/O
operations and a copied CPU snapshot. No guest state or firmware was changed.

Containment was four processor resets, 200 million instructions or 600 seconds,
with output limited by the fixed arrays to less than 64 KiB. The x64 process
completed its instruction ceiling and was confirmed exited within 469 seconds.
No raw trace, task-local media or background process remains.

## Reset classification

The first post-timer sample records CMOS index 8F receiving shutdown code 02
and reading it back. The last copied instructions enter F000:5516 and execute
`mov al,FE`; the unchanged next instruction is `out 64,al`. Core reports its
processor-reset boundary at FFFFFFF0. The resetting OUT itself does not retire
and is not claimed as a captured retirement I/O event.

This is consistent with the BIOS's intentional, staged 8042 reset, not evidence
of a new CPU fault. Static review confirms the existing Core processor-reset
path preserves RAM, devices and their time state, unlike cold reset. The pinned
PC110-QEMU reference (`bae390ebf21665ec97860d28e26ec46f1fe8f4e9`,
`qemu/target-i386/pc110post.c`, KBC-reset commentary) independently identifies
this RAM-preserving requirement. Its firmware redirects are not imported.

## Complete bounded continuation

The second run continues through the normal reset boundaries rather than
stopping on the first post-timer reset:

```text
timer passed=1 failed=0 delta=1668
executed=200003077 resets=3 waits=0 status=0 reason=BUDGET
post-timer retirements=140084481 boot-interrupt=0
last CS=F000 base=000F0000 IP=DC47 SS=0000 SP=03C2
EFLAGS=00000006 CR0=00000010 IDTR=00000000/000003FF
```

The last instruction ring contains F000:A966/A969/A977/A9CB, then DD3E and
DC3E--DC44. The last 128 port operations repeat this sequence:

```text
DC47 OUT 0074,7E   DC4F IN 0076 -> 15
DC47 OUT 0074,7F   DC4F IN 0076 -> EE
DD57 OUT 15EE,80
```

Static firmware inspection shows A966 tests a memory flag at ES:DI, while
A977 calls the DD3E service. DD3E reads the configured 15EE port, sets AX=5380
and BH=84, then outputs 80 to it. The copied EFLAGS has IF clear. These facts
identify a firmware service/wait boundary for the next receiver investigation;
they do not prove the precise interrupt or controller protocol by themselves.

Core reports no terminal execution fault. Its diagnostic history contains four
delivered general-protection exceptions, including the prior protected-memory
probe; this record does not inaccurately describe the whole POST as exception
free. No INT 19h was observed. Reaching the instruction ceiling is not success,
and the temporary mode's inherited timer-success exit code is not boot proof.

## Receiver reconciliation and review

The already implemented CPU/Core has SMI entry/RSM and a board SMIACT output.
The PC110 App does not wire an SMI trigger or SMRAM view. Open-Source-PC110's
pinned Chipset record (`02ccf2e8e19b298a2cf04c5d430278eb087cc368`)
identifies SMM-banked code, SMI source/clear/rearm registers and the configured
15EE descriptor. S46 must reconcile those together with the actual wait's
flag and command, before choosing a hardware repair. A synthetic AX=5380 BIOS
service, direct flag mutation, forced jump, or always-ready port is not a repair.

The entire temporary source diff was removed and reviewed against baseline:
retained production/test delta is +0/-0. S44's dual-width timer and 712-case
per-width unit evidence remains applicable to the unchanged code. This S ran
one x64 extended observation, not another x86 integration or desktop/RDP test.
Documentation governance and whitespace checks pass. External inputs, INI,
published EXEs and every Shared package remain unchanged.

The frozen qualification rows remain open for later POST, panel, Setup,
storage/guests, lifecycle and release. The reset substage is classified;
the newly observed service/wait is the next coherent receiver batch, not a
reason to repeat unsampled long waits or declare the whole product blocked.

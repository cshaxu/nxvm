# 8086--80386DX Four-Profile Architecture-Coverage And Implementation-Breakdown Audit

## Objective

Create the evidence-led implementation breakdown for the M5 CPU-completeness
program. Reconcile each in-scope Intel 8086, 80186, 80286, and 80386DX
instruction form and architectural behavior with implementation evidence, a
focused regression, or an explicit boundary.

## Required output

The audit produces one profile/mode/form ledger. For every entry it records the
minimum CPU profile; opcode and operand/address-size form; prefix and LOCK/REP
legality; applicable real, protected, and VM86 mode behavior; privilege;
exception, restart, and atomicity contract; current route; focused evidence;
and disposition as complete, partial, missing, outside 80386, or external
coprocessor boundary.

It then converts every partial or missing in-scope entry into the smallest
coherent candidate under the ordered Queue program: shared state delivery,
8086/80186 closure, 80286 closure, 80386DX closure, or cross-profile
verification. The audit may refine the later candidates but must not assign a
numeric task identifier or claim completeness merely from an opcode smoke.

## Boundaries

The audit distinguishes the CPU-side 8087/80287/80387 interface from x87
execution, which remains an external-coprocessor boundary. It also identifies
later-than-80386 features (including VME/PVI and 486+ instructions) as outside
this program rather than silently treating them as 80386 requirements. Device
fidelity, timing, guest-media, and Windows readiness are separate candidates.

## Completion standard

The candidate closes only when the ledger, exact residual transfers, and
dependency-ordered implementation breakdown are durable evidence, with no
unclassified in-scope form or architecture-state row.

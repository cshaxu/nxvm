# M5 T234 S7: Fixture Contract Review

## Objective

Review whether raw mutable session and device borrows require a new test
contract.

## Contract

**Decision: do not implement P3--P6.** The proposed fixture facade would add
test-only APIs solely to conceal implementation pointers already legitimately
used inside the same production module. That violates the repository's
minimalism rule. Tests may directly use existing implementation state when they
exercise the same module without mirror state or an alternate runtime route.

## Subtasks

### P2: Minimalism Review

Audit found the proposed contract would replace roughly 458 existing direct
uses with test-only wrappers while adding no production behavior or isolation.
P2 closes with the above decision and the coding-standard rule.

### P3--P6: Not Admitted

The machine, device, platform/control, and closure-facade migrations are
rejected as overbuilt. Future changes may add a helper only under the coding
standard's explicit justification rule.

## Closure

**P2 complete; S7 closed by decision.** The existing fixture facade is removed;
same-module tests directly include `session.h`. No new test interface is
warranted.

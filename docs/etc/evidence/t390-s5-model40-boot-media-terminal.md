# T390 S5: Model-40 Boot-Media Terminal

`M5:T390:S5:MODEL40-BYOB-BOOT-MEDIA:OK`

## Scope And Containment

S5 extends the explicit operator-run Model-40 BYOB capture helper with one
owner-managed bootable 1.2 MB floppy argument.  It supplies that input only
through the pre-existing `vm_session_config.fdd_image` startup route.  The
normal VM session owns the configured 80-cylinder, two-head, 15-sector,
512-byte removable medium; this S adds no Core, public ABI, profile or default
runtime path.

The helper aggregates matching normalized form/context rows instead of emitting
one raw record per retirement.  It emits a non-identifying progress marker each
1024 retirements, has a 128-row aggregation limit and a 250,000-retirement
limit, and stops at the first source-unallocated success, aggregation limit,
fault/status failure or the real-address `0:7C00` boot-sector entry checkpoint.
The external harness owns the process, uses a 30-second wall limit and a
five-second no-progress limit, removes its fresh ignored output after exit,
and retains no local firmware, medium, path, hash, byte, provenance or raw
trace in this repository.

## Focused Coverage

The new project-owned smoke creates a synthetic valid Model-40 manifest pair
and a 1.2 MB raw image, passes the image through the public session creation
configuration, and proves the resulting session retains a present removable
medium with the selected 80x2x15x512 geometry and registry-visible 2,400
logical sectors.  It prints
`M5:T390:S5:MODEL40-BYOB-BOOT-MEDIA:OK`.  The current-smoke registration adds
that focused proof to the canonical gate.  The external helper also rejects an
incomplete argument list with its usage status before creating a session.

## Contained External Result

One owner-managed ROM-plus-boot-media run used the normal session startup path.
It reached 104 successful real-address, CPL-0, no-prefix, 16-bit-default
retirements: 103 classified and one source-unallocated.  It exited under the
first-unallocated terminal, not a wall, no-progress, output, aggregation or
retirement budget.  Its temporary normalized output was below the S2 output
limit and was deleted after the process exited.

The first terminal record remains only the helper's normalized `other` class
with no operand qualifier; it is not sufficient to assign an Intel timing row.
No raw firmware instruction bytes, PC, guest trace or media data are retained
here.  Therefore the `0:7C00` checkpoint was not reached, and this is not a
full boot-corpus matrix or a physical-eligibility result.

## Transfer

T390 must next add a bounded, non-proprietary semantic normalizer for the first
unallocated selected-corpus instruction (or otherwise obtain a policy-compliant
semantic identity), then cite and prove its exact Intel-primary row or retain
it nonphysical.  It must repeat this process through the `0:7C00` checkpoint
before any Model-40 physical-retirement selection.  Board clocks, device
physical timing and L3 remain outside S5.

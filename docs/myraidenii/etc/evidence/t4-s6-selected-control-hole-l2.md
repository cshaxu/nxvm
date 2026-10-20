# T4 S6: Selected Control Hole L2 Contract

## Admission

The owner authorized bounded L2 compatibility models where original-hardware
material is absent.  External emulators supplied observable map facts only;
no source, algorithm, table, ROM content or protected trace was imported.

## Bounded Model

The owner-supplied selected program deterministically faulted at `0x69c` on
both host widths after the completed `0x696--0x697` route.  The current MAME
RAIDEN II map has no receiver in the local `0x690--0x69f` gap: its preceding
receiver is the sprite-buffer trigger at `0x68e--0x68f`, and its next receiver
is at `0x6a0`.  That is L2 map context, not physical-board proof.

The local Board therefore admits only `0x69c--0x69d` as a write-only,
stateless selected-control route through the existing owner.  It does not
admit the surrounding gap, model a device, provide reads or inspection, or
claim a hardware explanation.  The existing Core byte-route behavior accepts
the exact two-byte word and either individual byte lane; mixed accesses remain
rejected.

## Verification

The existing Board smoke covers byte lanes, exact word access, mixed-width
refusal, read/inspection refusal, actual V30 word-store execution and both
immediate neighbour bytes.  It passed on x64 and x86.

The opt-in owner-ROM probe used its existing identity verification, transient
program-lane staging and deletion.  Both widths advanced one instruction past
`0x69c` and next stopped at `0x692`:

| Host width | Executed | Ticks | Next fault code |
| --- | ---: | ---: | ---: |
| x64 | 68,247 | 1,076,629 | `0x692` |
| x86 | 68,247 | 1,076,629 | `0x692` |

`0x692` is a separate receiver candidate.  It is not merged with this route
or treated as an authorization for a generic `0x690--0x69f` acceptance.

After the production change, the complete repository-only unit selection ran
on both widths.  Each ordinary run passed 641 of 642 cases; its only failure
was the existing repository build-cleanup test's attempt to create a Windows
junction under restricted execution.  Re-running that sole test with its
required Windows capability passed on both x64 and x86, completing 642/642
per width.  Documentation governance and `git diff --check` passed.  The
temporary staging root was empty after both probe wrappers completed and its
test-run logs were deleted before closure.

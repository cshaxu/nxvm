# M1 T4 S34 — Historical MAME Black-Box Oracle

## Purpose

S33 treated current MAME's strict PAL verification as an inability to execute
the owner-supplied Raiden II archive.  S34 tests that premise without importing
MAME code or acquiring PAL/ROM data.

## Tool And Input Boundary

The research tool was the official MAME 0.182 Windows x64 package listed by
the MAME project on SourceForge.  It was downloaded only to the untracked
external research directory `O:\repos.external\mame-0.182`.

| Item | Value |
| --- | --- |
| Package | `mame0182b_64bit.exe` |
| Package SHA-256 | `6d010681a3c7f8e19333adf5232348623103c1889196191e82339d5966bba39a` |
| Executable | `mame64.exe` from that package |
| Input | Existing read-only `O:\assets\raidenii\raiden2.zip` owner archive |
| PAL acquisition | None |
| Retained game payload/output | None |

The archive identity remains the inventory recorded by S1.  This record does
not list, extract or copy its protected members.

## Neutral Black-Box Results

The historical verifier was invoked with:

```text
mame64.exe -verifyroms raiden2 -rompath O:\assets\raidenii -noreadconfig -nowriteconfig
```

Its relevant neutral result was:

```text
jj4b02__ami18cv8-15.u0342.jed (648 bytes) - NOT FOUND - NO GOOD DUMP KNOWN
romset raiden2 is best available
1 romsets found, 1 were OK.
```

One bounded noninteractive launch then used:

```text
mame64.exe raiden2 -rompath O:\assets\raidenii -video none -sound none
  -nothrottle -skip_gameinfo -nowriteconfig -seconds_to_run 10
```

It terminated under the requested ten-second budget after reporting the same
missing no-good dump warning and `Average speed: 554.76% (9 seconds)`.  No
screen, audio, debugger trace or ROM output was retained.

## Disposition

The absent `jj4b02` PAL is not a valid implementation blocker for a historical
MAME black-box run of this archive.  Conversely, it is not a PAL source: MAME
does not distribute the missing dump, and the product source policy forbids
downloading protected game/PAL data from third-party ROM sites.

This only restores an executable external reference baseline.  It does **not**
establish graphics bitplane layout, decryption, priorities, scroll, brightness,
or a native frame contract.  Those remain later Video work and require a
separate, source-policy-compliant behavior contract before any decoder is
implemented.

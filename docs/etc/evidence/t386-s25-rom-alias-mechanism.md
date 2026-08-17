# T386 S25 P1: Immutable ROM Subwindow Mechanism

`M5:T386:S25:ROM-ALIAS-LIFECYCLE:OK`

## Scope And Separation

This P1 adds a general Core immutable-ROM subwindow mechanism.  It is not a
Model-40 address-decode finding and it does not alter the Model-40 profile,
firmware carrier, D4 control, CPU execution, FDC contract, or the external-ROM
consumer result.

The retained primary DeskPro board material distinguishes D4 compatibility RAM
at `FE0000h`--`FFFFFFh`, D4 ROM-replacement decoding at
`E0000h`--`FFFFFh` when enabled, and the board's limited low-A20 aliasing
scope.  It does not, by itself, make the current Model-40 firmware composition
an accepted representation of the ROM's startup visibility.  In particular,
the firmware-responsible compatibility-ROM copy remains separate from a Core
mapping capability.  Model-40 attachment requires its own source-backed
window, priority, reset and lifecycle decision and a renewed external-ROM
consumer replay.

No firmware bytes, guest-media data, local asset path, vendor hash, or
third-party implementation enters this record.

## Core Contract

A firmware provider may now register a read-only alias whose source is a
checked subrange of an immutable ROM backing already registered by that same
configuration.  The new window supplies an independent physical address and
length, but owns neither the source bytes nor their lifetime.  The original
exclusive immutable-ROM registration remains strict: ordinary mappings still
cannot overlap it.

The route resolver retains established explicit priority: an earlier registered
provider wins at a true overlap.  Therefore an alias never silently replaces an
existing ROM or device window.  A failed configuration destroys aliases before
the sole backing owner, and normal teardown frees that backing exactly once.
The surface stays configuration-only; it introduces no runtime remap,
mutable-ROM, Console/YAML, test-only production, or Model-40-specific API.

## Focused Regression

`core-machine-firmware-capability-smoke` registers immutable bytes at
`E0000h`, then aliases the source subrange beginning at `E0001h` for two bytes.
It proves all of the following:

- a target window reads bytes at the requested non-zero backing offset;
- the two-byte target window ends exactly at its declared length and the next
  address resolves to ordinary RAM rather than extending the alias;
- an alias deliberately overlapping the already registered source window does
  not take ownership: the earlier immutable ROM route remains observable;
- alias writes reject; a failed configuration rolls back both the owner and
  aliases; and no backing is freed through an alias.

The rebuilt focused regression passed and emitted the marker above.  Retained
Model-40 ROM-layout S14 and logical-FDC S24 smokes also pass against this Core
source graph.

## Transfer

P1 makes partial windows, mirrors and intentional overlay attempts expressible
without duplicating immutable firmware backing.  It proves only that generic
lifecycle and route semantics.  It does not prove which of those forms the
DeskPro 386 Model 40 uses, whether any Model-40 window must be partly covered
by a different mapping, whether D4 replacement is active, or that the selected
ROM reaches the boot sector.  Those remain S25's board-owner trace and BYOB
consumer exit condition; no L3, timing, or functional-closure claim follows.

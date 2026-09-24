# M6 T41 Shared Type Vocabulary Convergence

## Outcome

Retire direct C-runtime vocabulary from MyNES, then let NXVM consume the
already available Shared Types vocabulary before designing the remaining
replacement for NXVM's legacy `type.h`/`type.c` facade.

## Ordered Subtasks

1. **S1 — Shared and MyNES.** Add only proven cross-product scalar vocabulary
   missing from Lib Types, then migrate every MyNES production C-runtime/type
   use and the MyNES test memory/string/allocation uses to that vocabulary.
   Test-only assertions and native-presenter probes are not made into a new
   Shared facade.
2. **S2 — NXVM existing-vocabulary adoption.** Do not change Lib. Remove
   unused standard includes and migrate NXVM uses that already have exact
   `lib/types` equivalents. Record every remaining legacy-facade capability.
3. **S3 — NXVM facade retirement design.** Prepare the Lib Types capability
   additions, owner mappings, migration batches and deletion proof for
   `type.h`, `type.c`, and `type-facade`. The owner reviews this design before
   any S3 implementation starts.

4. **S7 — MyNES fixed-width completion.** Apply the later owner-approved
   fixed-width rule to every MyNES production and test internal representation.
   Text bytes and lengths use `lib_u8`/`lib_size`; `char *` remains only at an
   explicit ISO-C, Common-session, Lib-file, or Win32 boundary.  Replace
   internal success/failure integers with `lib_bool`, `lib_status`, or a
   fixed-width result as its meaning requires.  The C `main` signature and
   direct C/SDK format or callback signatures remain native adapters.  This
   batch also removes MyNES's final `lib_c_strstr` test receiver, allowing its
   later Shared deletion.

## Completion Standard

S1 and S2 leave no direct production C-runtime/type use within their admitted
surfaces when an existing or S1-added Lib Types equivalent exists. S3 provides
the reviewed, bounded plan that can remove NXVM's legacy facade without copying
its broad forwarding layer into Lib.

S7 leaves no unclassified native scalar or text storage in
`src/app-mynes` or `test/app-mynes`.  Every retained native spelling is a
named direct ABI boundary or the required `main` signature; it is not an
internal representation shortcut.  No Shared API is added or changed.

## Exclusions

No change to SoftPC. No blanket Lib wrapper for test assertion, Windows
integration probes, or every historical `STD_*` routine. No S3 code migration
or deletion occurs before owner review.

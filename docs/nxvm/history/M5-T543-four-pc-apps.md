# M5 T543 Four PC Apps

## Admission And Frozen Coverage

Owner admission on 2026-10-04: close T542 and split the four fixed PC Apps as
app-my5160, app-my5170, app-mydeskpro386 and app-nxvm. The additional owner
direction requires exactly one App per S, in that order. Reference commit
4c0c2db16 is the clean, pushed T542 S20 acceptance baseline.

The owner additionally requires parallel src/test/assets ownership, while
docs/nxvm, tools/nxvm, shared version and the NXVM MTSP sequence remain unified.
The target artifact roots are assets/my5160, assets/my5170,
assets/mydeskpro386 and assets/nxvm. No new per-App governance queue or counter
is created; the existing NXVM commit target continues to own these four Apps.

The [proposal](../proposals/m5-independent-pc-apps.md) owns the sequential
delivery design. [Current](../states/CURRENT.md) owns the one active packet.
This record is evidence and convergence inventory, not another current status.

| Batch | Existing source owner | Receiving source/test owner | Admission disposition |
| --- | --- | --- | --- |
| S1 XT | app-nxvm/profiles/xt and XT binding/firmware composition assertions | app-my5160 and test/app-my5160 | Admitted; no implementation acceptance yet. |
| S2 IBM AT | app-nxvm/profiles/at and shared-with-default translation units, IBM AT assertions | app-my5170 and test/app-my5170 | Planned; reconcile shared mechanisms rather than copy a peer App. |
| S3 DeskPro | app-nxvm/profiles/model40, D4, ROM and copied observations, Model40 assertions | app-mydeskpro386 and test/app-mydeskpro386 | Planned; retain actual Compaq-specific ownership. |
| S4 NXVM | app-nxvm/profiles/default_profile, project-owned firmware and remaining shell | app-nxvm and test/app-nxvm | Planned; retains original default hardware, not a new board. |

Each row consumes source, public/private includes, fixed composition/binding,
CMake selection/link graph, repository-only tests, external integration
scenarios, documentation/tool references and its two deployed artifacts.
Common PC mechanisms and their tests remain in ibmpc. Lib/Common/x86 and MyNES
are excluded because this is a PC product-ownership cutover, not their upgrade.

Required proof per accepted row: no peer-App production dependency; one shared
Product/Machine implementation; preserved board/ROM/CMOS/media values; full
repository-only units on x64 and x86; original affected boot checkpoint;
optimized stripped pair retaining runtime Debug; SHA-256 and architecture;
source/test line accounting and reviewed, pushed one-target P commits.

Whole-T closure requires all four rows accepted, independent build entries,
the retained 58 original integration contexts passing once with unchanged
predicates, eight verified current EXEs, complete manifests/dependency/docs
checks, owner INI/media-path preservation and no obsolete live selection or
unclassified migration residue. New hardware, timing upgrades, PC110 and
MyArcade are non-applicable with their existing separate receivers.

Accepted Td S175 reconciles artifact/commit-target mapping in the execution
rule before the migration: assets/<app>/<profile> retains the original INI
relative-path depth; the existing NXVM scope, PC version and MTSP stay unified.
It creates no chip/Product/Machine mechanism or per-App governance hierarchy.

## S1 Admission Review

Confirmed the source inventory contains a real XT composition and firmware
provider, while the thin main/binding and CMake-generated binding currently
remain under the four-profile app-nxvm shell. S1 moves rather than forks the XT
owner and consumes the accepted shared PC runtime. Later App deliveries are
not admitted simultaneously. Planning documents create no 0543 artifact or
claim a completed source migration.

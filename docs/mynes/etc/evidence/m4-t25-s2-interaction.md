# M4 T25 S2 Interaction Convergence

## Scope And Baseline

S2 implements the owner's 2026-09-21 direction to make MyNes interaction and
configuration follow SoftPC. The read-only baseline was SoftPC's App config,
command header/implementation, and App config/command smoke tests. SoftPC was
not modified.

## Delivered Mechanism

MyNes App now owns a compact session record with SoftPC-equivalent display,
transition-pending, prompt-due, and pending-monitor-text facts. A lifecycle
request reserves the transition and suppresses a cooked prompt. INIT remains
only an acknowledgement. Terminal lifecycle wording is held until Common
reports that the monitor is current.

Window presentation reports a start or resume outcome through the cooked
monitor followed by one prompt. Console presentation intentionally clears
running monitor output because raw gameplay owns that endpoint. Paused, stopped
and error outcomes remain deferred until the monitor regains ownership. The
existing NES-specific ROM and nested debug commands retain their grammar.

The adjacent configuration template and parser use display=window|console.
The retired video key is rejected. As in SoftPC, the parser accepts quoted
values, strips line comments introduced by semicolon or hash, and ignores
ordinary non-assignment records.

## Similar-Issue Sweep

| Query / scope | Hit disposition |
| --- | --- |
| App/configuration source, tests and packaged templates for the retired configuration key and old pending field | The only video hits are the deliberate retired-key rejection and native graphics-status wording; all configuration templates and parser keys use display; no old pending field remains. |
| App and App-facing tests for prompt arming, transition-pending, pending text, broker and monitor-current callbacks | App's one session record is the sole transition/prompt owner. Composition and both native KVM fixtures install its broker/current callbacks. Direct command results retain their existing local-output path. |
| Active design/contract/proposal/configuration records | UI, App/Core contract, M3 design, M4 proposal, source template and packaged template use display. Historical records retain their original terminology by the documentation rule. |
| Active interaction records for F5/F12 versus Esc | M4 plan, M3 design/delivery and App/Core verification now name Esc; prior closed-task evidence retains its historical vocabulary. Lib's generic F-key identities remain neutral host input capabilities, not MyNes product bindings. |

## Verification

On both build/shared-x64 and build/shared-x86, rebuilt the versioned MyNes
executable and ran the App config and command smokes, App debug/media command
smokes, and native Window/Console smokes. All passed.

The command smoke covers no-prompt INIT, deferred terminal output, Window
started/resumed output, raw-Console running suppression, broker stale-notice
clearing, output spacing, and the original lifecycle/media errors. The
configuration smoke covers both display values, retired video rejection, quoted
ROM paths, comments and non-assignment records. The whitespace diff check
passed.

## Remaining Boundary

This is interaction convergence evidence, not a claim that T25's owner-visible
named-ROM protocol is complete. That direct Window/Console game-experience
observation remains the active T25 exit obligation.

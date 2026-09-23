# Console, KVM And SoftPC Reuse Design Evidence

## Request And Scope

On 2026-09-19 the owner requested cooked management/debug, no menus, defaults
editable through Console and Window/ASCII gameplay. Follow-up explicitly gave
both KVM backends equal importance, selected SoftPC's overall architecture with
NES Core replacing VM/Compat/MVDM, and preferred direct Lib/Common code reuse.
This is M0 Td S3 on MyNes 2c22a10; planning remains the next unadmitted S in the
same Td. No runtime implementation or import is performed by this design.
S2 evidence remains historical; the new authority supersedes its direct
Common/Core binding and control-thread execution, not its hardware discipline.

## Reference And Reuse Findings

Initial SoftPC inspection used cd6a6dc7be036334ab60eaab705bf3c7dc66e313.
During follow-up its HEAD advanced; the additional review pins
6541e43d61d9f1bd923ac0bc477186bf38e86451. References are src/lib and src/common
READMEs, machine/session/UI/Console/KVM interfaces, their implementations and
design architecture. Unrelated working-tree changes were neither used nor edited.
No source copy or license compatibility claim is made. No root license was found
in the initial inspection; per-unit origin/grants remain an import prerequisite.

| Source area | Reuse assessment | Required targeted work |
| --- | --- | --- |
| lib/types, base, storage | Prefer unchanged units and dependencies. | Pin notices, manifests, build selection and focused tests. |
| lib/console, console-broker | Prefer existing logical endpoints and transactional native ownership. | Verify cooked cancellation, raw handoff and shutdown with MyNes integration. |
| lib/kvm-base, kvm-window | Prefer existing copied events/frame and native presentation. | Validate palette/stride/frame limits and NES adapter without importing PC policy. |
| lib/kvm-console | Reuse endpoint/input/worker mechanisms. | Add generic pixel-to-ASCII; current graphics submission skips rendering. |
| common/session, ui | Reuse queue/reducer/prompt/composition mechanisms. | Replace graphical Console status routing with real ASCII submission; review machine API coupling. |
| common/machine | Reuse single executor, generation, publication and paused rendezvous. | Isolate x86-specific debug request/snapshot fields before adoption; reconcile NES driver callbacks and loading. |
| common/debug, xasm32 | Not an unchanged NES debugging capability. | Keep x86 commands/assembler unselected; implement NES semantics through the same session/rendezvous. |
| SoftPC VM/Compat/MVDM | Not imported. | Own NES Core hardware and its real driver/input/frame/debug adapter. |

This is an inventory assessment, not a verified import manifest. Pinned public
machine_interface.h embeds x86 segment/control-register snapshots and port/real
address requests. Generic coordination therefore needs a bounded neutral contract
change before it can be honestly called unchanged reuse. The frame contract already
carries indexed graphics/palette/stride; existing PC text fields do not justify a
second MyNes frame ABI. Validate fit and inactive fields in adapter tests.
SoftPC Common UI's graphic-mode explanatory text and KVM Console's graphics no-op
cannot satisfy ASCII acceptance. These are named extension points, not hidden gaps.

## Coverage And Review

| Owner requirement or boundary | Authority / evidence | Disposition |
| --- | --- | --- |
| SoftPC overall architecture | design/ARCHITECTURE.md: shared Common with injected Core driver, composition-only Core entry. | Accepted by actual-change review. |
| Targeted reuse | Source policy and planning inventory above; compatible units retained, true gaps isolated. | Accepted by actual-change review. |
| Equal backends | Architecture/UI/roadmap: dual-backend gameplay requires both gates. | Accepted by actual-change review. |
| Cooked management/debug, no menus | UI and one session command route. | Accepted by actual-change review. |
| Responsive input and execution | Control thread plus sole Common executor, copied queues, finite Core slices. | Accepted by actual-change review. |
| Defaults/live config and safe keys | App transaction, source reset/generation and executor-side NES mapping. | Accepted by actual-change review. |
| Raw/cooked handoff | One broker, retirement/reset before replacement, preserved monitor and return action. | Accepted by actual-change review. |
| ASCII implementation receiver | Lib renderer plus Common UI routing and explicit rendering/input acceptance. | Accepted by actual-change review. |
| Debug and teardown | Existing paused rendezvous; stop/join before releasing callback targets. | Accepted by actual-change review. |
| Same Td governance | Current S3 closes design only; planning stays unnumbered in Queue. | Accepted by actual-change review. |

## Verification And Delivery

Active documentation gate, checker self-tests and actual-diff review passed. Final documentation and staged whitespace/scope checks are required before the delivery commit.
Same session performs execution and coordinator review; no independent review or
runtime/UI test is claimed. Check commands are
`tools/Verify-DocumentationGovernance.ps1 -RepositoryRoot .`, its `-SelfTest`,
`git diff --check`, `git diff --cached --check` and documentation scope inspection
against 2c22a10. Search current authorities for obsolete direct Common/Core,
control-thread guest execution and optional/later-only Console claims.

ROMs remain ignored and untouched. No remote was configured at admission;
recheck before commit and apply remote-aware delivery. Intended local subject:
`M0 Td S3 P1: align SoftPC reuse and equal KVM backends`.

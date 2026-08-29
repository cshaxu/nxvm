# M5 T501 Four-Profile Post-Scheduler Media Closure

| Subtask | Accepted result |
| --- | --- |
| S1 | Rebuilt the Release baseline and classified all nine supported profile/CPU/media rows. |
| S2 | Completed the Model-40 ROM/FDD diagnostic List 1/List 2 audit; original material did not determine the firmware-visible board relation. |
| S3 | Transferred that whole underdetermined relation to one source-gated TODO receiver without a scheduler, firmware, FDD, DMA, PIC, VM, or profile shortcut. |
| S4 | Rebuilt the normal 0501 stripped Release target, replayed all nine rows, and passed complete unit and integration gates. |

T501 is closed. Seven matrix rows reach a supported semantic terminal. The
sole nonterminal rows are explicit external boundaries: the supplied 1.2-MB
80286 input reaches an 80386-only loader path, and the Model-40 ROM/FDD board
diagnostic relation remains source-gated at its recorded TODO receiver. Neither
is a scheduler, controller, firmware, or VM compatibility defect.

The current artifact is `build/output/nxvm_0_5_0501.exe`, a stripped Release
executable retaining the runtime debugger, SHA-256
`3651CD9245D11374F6089199E83A15F1F4C67B43E6A27F2751F74D3B57B89297`.
Complete Debug unit passed 312/312 and external-input integration passed 20/20.

Evidence: [S1 matrix](../etc/evidence/t501-s1-release-matrix-baseline.md),
[Model-40 List 1](../etc/evidence/t501-s2-model40-diagnostic-list-1.md),
[Model-40 List 2](../etc/evidence/t501-s2-model40-diagnostic-list-2.md), and
[S4 closure](../etc/evidence/t501-s4-release-matrix-closure.md). The retained
[proposal](M5-T501-four-profile-post-scheduler-media-closure-proposal.md)
preserves the admitted scope.

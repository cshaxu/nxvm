# M5 T501 Four-Profile Post-Scheduler Media Closure

| Subtask | Accepted result |
| --- | --- |
| S1 | Rebuilt the Release baseline and classified all nine supported profile/CPU/media rows. |
| S2 | Completed the Model-40 ROM/FDD diagnostic List 1/List 2 audit; original material did not determine the firmware-visible board relation. |
| S3 | Transferred that whole underdetermined relation to one source-gated TODO receiver without a scheduler, firmware, FDD, DMA, PIC, VM, or profile shortcut. |
| S4 | Rebuilt the normal 0501 stripped Release target, replayed all nine rows, and passed complete unit and integration gates. |
| S5 | Replaced the duplicate D4 ROM owner with the immutable firmware owner, selected the External-L2 two-mechanism/one-media construction, and reached the Model-40 DOS prompt through normal FDC/DMA/IRQ/KBC paths. |

S4's matrix replay established seven supported terminals and one independent
80286 external-input boundary.  S5 closes the previously nonterminal Model-40
row at a real DOS prompt, without fabricating a drive-ready signal, firmware
bypass, second media owner, or profile time path.  The original-source gap for
physical Model-40 READY/change timing remains bounded debt; it is not needed to
claim the observed functional terminal.

The current artifact is `build/output/nxvm_0_5_0501.exe`, a stripped Release
executable retaining the runtime debugger, SHA-256
`502D12BE1E30EDBCBE609F424F778A97F665210CA2B3F8B50C88636F8377B2CC`.
Complete Debug unit passed 312/312 and external-input integration passed 20/20.

Evidence: [S1 matrix](../etc/evidence/t501-s1-release-matrix-baseline.md),
[Model-40 List 1](../etc/evidence/t501-s2-model40-diagnostic-list-1.md),
[Model-40 List 2](../etc/evidence/t501-s2-model40-diagnostic-list-2.md), and
[S4 closure](../etc/evidence/t501-s4-release-matrix-closure.md), and
[S5 boot universe](../etc/evidence/t501-s5-model40-boot-universe.md). The retained
[proposal](../proposals/m5-four-profile-post-scheduler-media-closure.md)
preserves the admitted scope.

# M5 T429: CECG 8-bit Port Wait Skeleton

## Outcome

T429 S1 accepts `1c02a853`, a Model-40-selected, Core-owned generic-AT wait
skeleton for documented CECG I/O ports. CPU external cycles now explicitly name
memory or port space. The existing CPU transaction stays the sole lifecycle
owner; a selected window adds its logical wait only on a matching commit.

## Evidence And Verification

The retained [S1 evidence](../etc/evidence/t429-s1-cecg-8bit-bus-wait.md)
records original CECG addressing/interface facts separately from the generic-AT
wait, lifecycle cancellation and reset proof, real `OUT` coverage, Model-40
selection, 5170 isolation, documentation governance, diff hygiene, and serial
current-gate result.

## Closure And Transfer

This closes only the port-surface wait skeleton. It does not establish CECG
memory-aperture timing, IOCHRDY/NOWS/BUSRDY electrical behavior, raster/status
or monitor timing, a physical clock calibration, or DeskPro L3 acceptance.
Those remain bounded receivers under the physical-cycle proposal.
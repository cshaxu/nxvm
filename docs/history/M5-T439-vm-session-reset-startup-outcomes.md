# M5 T439: VM Session Reset And Startup Outcome Propagation

T439 is active at S1. Its approved
[candidate proposal](../proposals/m5-vm-session-reset-startup-outcomes.md)
consumes T438's Core reset-failure contract through the existing VM session
control and lifecycle path. The owner's minimalism constraint requires one
session outcome owner, one production reset/start path and no new wrapper,
fault model or rollback framework. The active acceptance contract is in
[CURRENT.md](../states/CURRENT.md).

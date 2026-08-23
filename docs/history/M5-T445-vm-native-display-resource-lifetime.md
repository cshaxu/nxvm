# M5 T445: VM Native Display Resource Lifetime Repair

T445 is admitted from the owner-approved
[proposal](../proposals/m5-vm-native-display-resource-lifetime.md).
S1 repairs the owner-local Win32 window-DC and Linux curses lifecycles without
merging their distinct host policies.

## S1 Result

The implementation gives the Win32 renderer one display-thread-owned DC
cleanup route and gives the Linux display thread both curses initialization and
finalization.  Complete proof and the 0445 artifact are in the
[S1 evidence](../etc/evidence/t445-s1-vm-native-display-resource-lifetime.md).

# Receiving Shared Core partial-memory route

NXVM M5 T585 S6 Shared commit `6f68a2bde` changes the existing Core memory
route used by MyRaidenII. The product source and configuration are unchanged;
this record and the rebuilt x64/x86 release executables form the separate
MyRaidenII delivery. The two executables were stripped and their deployed
bytes compared against the prepared releases; no ROM was committed.

Both architecture-wide unit suites passed 712/712, including the existing
MyRaidenII board-bus test. The selected-set startup probe and the deployed
hidden Console smoke passed on both widths using a task-local copy of the
owner's archive. The probes exercise a bounded startup path; they do not
claim gameplay, physical input, RDP or audio qualification. NXVM retains the
complete receiver and integration evidence in its T585 S6 record.
| assets/myraidenii/myraidenii_0_0_0007_x64.exe | x64 | `3F8C61E1630D85D23E06D44EA4FDF34C8E107DEEC53563AAB92DB82BE668033E` |
| assets/myraidenii/myraidenii_0_0_0007_x86.exe | x86 | `056607CC84588A475301FDB9DE98E162B700D46B64254E7BCB2A2B76049EFF92` |

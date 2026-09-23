# M5 T32 S3 CNROM Verification

The synthetic media lifecycle fixture now inserts a valid Mapper-3 image after
Mapper-2, runs 64 Driver slices without a trap, and proves an invalid medium
mode retains the accepted Mapper-3 machine. This supplements the dedicated
fixture's bank/conflict/write-protection proof.

No supplied commercial CNROM ROM exists. T32 therefore qualifies only the
declared finite Mapper-3 fixture profile; it does not add a six-input product
acceptance claim.

- Focused Mapper-3/lifecycle tests passed on x64 and x86.
- Complete serial CTest passed: x64 114/114 and x86 114/114.
- Documentation governance and whitespace gates passed. The versioned x64/x86
  artifacts remain the rebuilt S2 pair; S3 changes test/evidence only.

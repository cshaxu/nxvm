# T582 S10 — PIT deadline aggregation

The Board selects the earliest positive PIT channel deadline in device ticks and converts it once per PIT clock domain. A separate found flag retains the maximum integer as a valid candidate; an immediate event is retained independently. The primary and auxiliary PIT domains remain independent. No public interface or persistent state was added.

The device-to-source conversion is monotonic for valid inputs; if its multiplication overflows for the minimum positive deadline it also overflows for larger ones. A deterministic 150,000-case synthetic comparison covered zero, maximum, overflow, phase and pre-existing deadlines. This is model evidence, not a substitute for the production path. The existing board timing qualification now programs two channels on the same PIT and verifies that the shorter deadline wins; the existing auxiliary PIT test covers distinct domains. Both architecture-specific tests passed.

Cached x64 and x86 builds and tests passed: 712 unit and 48 integration tests per architecture, plus 3 XT, 5 AT and 6 DeskPro profile integrations per architecture. The new targeted board test passed on each architecture after that run. The selected-set MyRaidenII startup and deployed Console checks passed for both architectures. They do not qualify gameplay or audio. Full qualification log SHA-256: B428A216988FD12A78E2BE87C5685A31C3C88B8D3C5B0DE5AB4AFAA87719BFE7.

The bounded S9 comparison for 10 million matching guest ticks found x64 24610/23596 ms and x86 30114/26812 ms (baseline/candidate, one sequential pair). This is not evidence of stable real-time performance or usability. Interactive desktop and RDP acceptance were not run in this step.

## Deployed artifact identities

```text
assets/nxvm/nxvm_0_5_0582_x64.exe x64 EB043A92E8E7640FF3D0C2A6D215F5FF2F722B91ED286D117180F2F7A840D959
assets/my5160/my5160_0_5_0582_x64.exe x64 EE18F7F6920F6562537377AE3BB2E0BDFCA92292E04257C9AC90852E90CC276A
assets/my5170/my5170_0_5_0582_x64.exe x64 2C2DA915C3EC37C91C1DB7B2B26C008AD57FC6E57C14E26DF18F1EEC4E164410
assets/mydeskpro386/mydeskpro386_0_5_0582_x64.exe x64 D708CB5BDF0B5231A1DF8D54F9CE82E0C0AC640FE6FF4A50E7D0B68A493D7024
assets/mypc110/mypc110_x64.exe x64 403B6D0A693E59B767093491283FAE6D9D7997022BF5C05E2169AF250B0B10A2
assets/myraidenii/myraidenii_0_0_0007_x64.exe x64 FAEED74706744930024D5689BE4C1257D33DD992EEFF2F32AC1951BCFE9E8A69
assets/mynes/mynes_0_0_0044_x64.exe x64 B499CD2583ECDDD1AE01EED882E21D49D20187558E2138858E5A27117CF74FC8
assets/nxvm/nxvm_0_5_0582_x86.exe x86 EB5E18361723C09053D92AEF99AA23D197DBC77827E17A3E99A10AF89A76A42A
assets/my5160/my5160_0_5_0582_x86.exe x86 ABF15A45FA0000B75840AFE692F4562A2138BCE817F2A5341130C1CF578D59E1
assets/my5170/my5170_0_5_0582_x86.exe x86 12AF69E191B690697E1672F8F4D7A92D55CA164E5A9610D580CB7D9BCB1AB245
assets/mydeskpro386/mydeskpro386_0_5_0582_x86.exe x86 ECE4681810B963AFFF12B89DA7A56FCB24665730374310E6ADD89B50C7171857
assets/mypc110/mypc110_x86.exe x86 C12550D6F6709E009E7C345795C3CE21A2025A41117C4441A6E4610A2614FC98
assets/myraidenii/myraidenii_0_0_0007_x86.exe x86 95422A4F02BAC119D15418830FBD02740B87644E2939BD2B51895EA3D99ECF68
assets/mynes/mynes_0_0_0044_x86.exe x86 718617AF2FBF6D49C36AE3BB10043DC3B2383E35440E7A371E503B8BD4B4EA16

```
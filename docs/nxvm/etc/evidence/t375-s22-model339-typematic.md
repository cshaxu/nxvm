# T375 S22: Model-339 Typematic Contract

IBM PC/AT Technical Reference 6280099 specifies the default keyboard delay as
500 ms plus or minus 20 percent and default typematic rate as 10 cps plus or
minus 20 percent. The selected 8-MHz Model-339 profile therefore uses nominal
4,000,000-tick initial delay and 800,000-tick repeat interval. Their accepted
ranges are 3,200,000--4,800,000 and 640,000--960,000 ticks respectively.

Only the Model-339 descriptor changes. Generic PC/AT remains zero-configured.
The 20-ms keyboard command-response statement is an upper bound, not a fixed
completion time, so `kbc_command_response_ticks` remains zero. Focused profile
proof confirms descriptor-to-session transfer and prints:

```text
M5:T375:S22:MODEL339-TYPEMATIC:OK
```

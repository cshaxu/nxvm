# T417 S2 Refresh Locality Closure Audit

M5:T417:S2:REFRESH-LOCALITY-CLOSURE:OK

Fresh coordinator review of P1 `4f9ca836` confirms one bounded existing-owner
receiver. D4 composition alone binds the shared PIT counter-1 output. Its
active-low refresh pulse invalidates the existing Core CPU external-memory
locality key; a later same-page CPU physical cycle therefore receives a miss.
The focused test advances the actual programmed counter through 18 ticks, and
also proves reset clearing. No generic PIT output, public ABI, scheduler, or
second transaction path was added.

Original D3PE evidence establishes the refresh topology and non-CPU refresh
route. It does not establish the generic policy's page retention, the sampled
counter frequency, a physical DRAM refresh interval, or calibrated D4
arbitration. The policy is accepted only as a labelled generic-AT locality
boundary. The full serialized 288-test current gate, documentation governance,
diff hygiene, D4 platform regression, CPU/DMA competition regression and the
corrected Model-40 private-composition timing contract pass.

T417 closes this refresh-locality receiver. Exact D4 refresh timing, refresh
versus DMA/BWAIT arbitration, physical row retention, locked/RMW timing, device
service timing and Model-L3 acceptance remain transferred.
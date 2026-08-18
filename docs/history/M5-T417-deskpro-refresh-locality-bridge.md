# M5 T417: DeskPro Refresh Locality Bridge

T417 consumes the bounded refresh receiver after T416. It uses the original D3PE
refresh topology and an existing Core PC/AT counter-1 refresh owner to add one
explicitly generic-AT locality boundary: a D4-configured counter-1 low pulse
invalidates the CPU external-memory locality key. It does not derive original
DeskPro refresh period, page retention, arbitration phase or cycle timing.
P1 implements the callback and proves the real D4-configured counter reaches
its refresh low pulse before the later same-page CPU external cycle is charged
as a miss.
P2 independently closes T417. It confirms D4-only binding, the counter-1
active-low refresh boundary, the real 18-tick focused proof and reset behavior,
and retains all calibrated D4 refresh/arbitration work as future receivers.

#ifndef MYPC110_CLOCK_H
#define MYPC110_CLOCK_H

#define MYPC110_CPU_CLOCK_HZ 33000000u
#define MYPC110_PIT_INPUT_HZ 1193182u
#define MYPC110_RTC_INPUT_HZ 32768u
/* L2 nominal refresh period, rounded onto the one Core time axis. */
#define MYPC110_REFRESH_SOURCE_TICKS \
    ((18u * MYPC110_CPU_CLOCK_HZ + MYPC110_PIT_INPUT_HZ / 2u) / MYPC110_PIT_INPUT_HZ)

#endif

#ifndef CORE_MACHINE_CPU_TRACE_H
#define CORE_MACHINE_CPU_TRACE_H

/*
 * The decoder's exception-flow notation is local to the CPU implementation.
 * CPU_TRACE_ERROR and CPU_TRACE_SET_ERROR are supplied by its execution unit.
 */
#define CPU_TRACE_CALL_BEGIN(call_name)
#define CPU_TRACE_BLOCK_BEGIN(block_name)
#define CPU_TRACE_CALL_END
#define CPU_TRACE_BLOCK_END
#define CPU_TRACE_CHECK_BREAK(call) \
    if (1) { (call); if (CPU_TRACE_ERROR) { break; } } else
#define CPU_TRACE_CHECK_RETURN(call) \
    do { (call); if (CPU_TRACE_ERROR) { return; } } while (0)
#define CPU_TRACE_CHECK_RETURN_ZERO(call) \
    do { (call); if (CPU_TRACE_ERROR) { return 0; } } while (0)
#define CPU_TRACE_IMPOSSIBLE_BREAK CPU_TRACE_CHECK_BREAK(CPU_TRACE_SET_ERROR)
#define CPU_TRACE_IMPOSSIBLE_RETURN CPU_TRACE_CHECK_RETURN(CPU_TRACE_SET_ERROR)
#define CPU_TRACE_IMPOSSIBLE_RETURN_ZERO \
    CPU_TRACE_CHECK_RETURN_ZERO(CPU_TRACE_SET_ERROR)

#endif

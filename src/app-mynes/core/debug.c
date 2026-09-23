#include "core/driver.h"
#include "core/debug_interface.h"
#include "core/machine.h"

#define CORE_DEBUG_OBSERVATION_BYTES 40u

static lib_u16 core_debug_read_u16(const lib_u8 *bytes)
{
    return (lib_u16)bytes[0] | ((lib_u16)bytes[1] << 8u);
}

static lib_u32 core_debug_read_u32(const lib_u8 *bytes)
{
    return (lib_u32)bytes[0] | ((lib_u32)bytes[1] << 8u) |
        ((lib_u32)bytes[2] << 16u) | ((lib_u32)bytes[3] << 24u);
}

static void core_debug_write_u16(lib_u8 *bytes, lib_u16 value)
{
    bytes[0] = (lib_u8)value;
    bytes[1] = (lib_u8)(value >> 8u);
}

static void core_debug_write_u32(lib_u8 *bytes, lib_u32 value)
{
    bytes[0] = (lib_u8)value;
    bytes[1] = (lib_u8)(value >> 8u);
    bytes[2] = (lib_u8)(value >> 16u);
    bytes[3] = (lib_u8)(value >> 24u);
}

static void core_debug_write_u64(lib_u8 *bytes, lib_u64 value)
{
    lib_u32 index;
    for (index = 0u; index < 8u; ++index) bytes[index] = (lib_u8)(value >> (index * 8u));
}

static lib_u32 core_debug_domain_result(const core_run_result *result)
{
    if (!result->trap_valid) return result->reason == CORE_MACHINE_STOP_BREAKPOINT ?
        2u : result->reason == CORE_MACHINE_STOP_BUDGET ? 1u : 0u;
    switch (result->reason) {
    case CORE_MACHINE_STOP_UNSUPPORTED_OPCODE: return 3u;
    case CORE_MACHINE_STOP_UNSUPPORTED_DEVICE: return 4u;
    case CORE_MACHINE_STOP_BUS_FAILURE: return 5u;
    default: return 0u;
    }
}

static void core_debug_write_observation(lib_u8 *output,
    const core_observation *observation)
{
    lib_memory_set(output, 0, CORE_DEBUG_OBSERVATION_BYTES);
    output[0] = observation->a;
    output[1] = observation->x;
    output[2] = observation->y;
    output[3] = observation->s;
    output[4] = observation->p;
    output[5] = observation->cartridge_present;
    core_debug_write_u16(output + 6u, observation->pc);
    core_debug_write_u64(output + 8u, observation->cycles);
    core_debug_write_u64(output + 16u, observation->instructions);
    core_debug_write_u32(output + 24u, (lib_u32)observation->trap_reason);
    core_debug_write_u16(output + 28u, observation->trap_pc);
    core_debug_write_u16(output + 30u, observation->trap_address);
    output[32] = observation->trap_opcode;
    output[34] = observation->trap_valid;
}

static void core_debug_begin_response(lib_u8 *output, lib_u16 operation,
    lib_u32 data_size, lib_u32 domain)
{
    core_debug_write_u16(output, CORE_DEBUG_VERSION);
    core_debug_write_u16(output + 2u, operation);
    core_debug_write_u32(output + 4u, CORE_DEBUG_DOMAIN_BYTES + data_size);
    core_debug_write_u32(output + CORE_DEBUG_HEADER_BYTES, domain);
}

static lib_status core_driver_execute_debug(void *context, const void *request,
    lib_size request_size, void *response, lib_size response_capacity,
    lib_size *response_size)
{
    const lib_u8 *input = request;
    lib_u8 *output = response;
    core_driver *driver = context;
    core_observation observation;
    core_run_result run_result;
    lib_u16 operation;
    lib_u32 payload_size;
    lib_size data_size;
    lib_size total_size;
    lib_status status;

    if (response_size == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *response_size = 0u;
    if (driver == LIB_NULL || driver->machine == LIB_NULL || input == LIB_NULL ||
        output == LIB_NULL || request_size < CORE_DEBUG_HEADER_BYTES)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (core_debug_read_u16(input) != CORE_DEBUG_VERSION) return LIB_STATUS_UNSUPPORTED;
    operation = core_debug_read_u16(input + 2u);
    payload_size = core_debug_read_u32(input + 4u);
    if (payload_size != request_size - CORE_DEBUG_HEADER_BYTES)
        return LIB_STATUS_INVALID_ARGUMENT;

    switch (operation) {
    case CORE_DEBUG_OBSERVE:
    case CORE_DEBUG_WARM_RESET:
        if (payload_size != 0u) return LIB_STATUS_INVALID_ARGUMENT;
        data_size = CORE_DEBUG_OBSERVATION_BYTES;
        break;
    case CORE_DEBUG_PEEK:
        if (payload_size != 4u || core_debug_read_u16(input + 10u) == 0u ||
            core_debug_read_u16(input + 10u) > 256u) return LIB_STATUS_INVALID_ARGUMENT;
        data_size = 4u + core_debug_read_u16(input + 10u);
        break;
    case CORE_DEBUG_POKE:
        if (payload_size < 4u || core_debug_read_u16(input + 10u) == 0u ||
            core_debug_read_u16(input + 10u) > 64u ||
            payload_size != 4u + core_debug_read_u16(input + 10u))
            return LIB_STATUS_INVALID_ARGUMENT;
        data_size = 0u;
        break;
    case CORE_DEBUG_STEP:
        if (payload_size != 4u || core_debug_read_u32(input + 8u) == 0u ||
            core_debug_read_u32(input + 8u) > 1000u) return LIB_STATUS_INVALID_ARGUMENT;
        data_size = 8u + CORE_DEBUG_OBSERVATION_BYTES;
        break;
    case CORE_DEBUG_BREAK_SET:
        if (payload_size != 4u || core_debug_read_u16(input + 10u) > 1u)
            return LIB_STATUS_INVALID_ARGUMENT;
        data_size = 0u;
        break;
    case CORE_DEBUG_BREAK_LIST:
        if (payload_size != 0u) return LIB_STATUS_INVALID_ARGUMENT;
        data_size = 36u;
        break;
    case CORE_DEBUG_OUTPUT_SET:
        if (payload_size != 8u) return LIB_STATUS_INVALID_ARGUMENT;
        if (input[8u] > 1u || input[9u] != 0u || input[10u] != 0u || input[11u] != 0u ||
            input[12u] != 0u || input[13u] != 0u || input[14u] != 0u || input[15u] != 0u)
            return LIB_STATUS_INVALID_ARGUMENT;
        data_size = 0u;
        break;
    case CORE_DEBUG_BIND_SET:
        if (payload_size != 12u) return LIB_STATUS_INVALID_ARGUMENT;
        return LIB_STATUS_UNSUPPORTED;
    default:
        return LIB_STATUS_UNSUPPORTED;
    }
    total_size = CORE_DEBUG_HEADER_BYTES + CORE_DEBUG_DOMAIN_BYTES + data_size;
    if (response_capacity < total_size) return LIB_STATUS_INVALID_ARGUMENT;

    if (operation == CORE_DEBUG_OBSERVE) {
        status = core_machine_observe(driver->machine, &observation);
        if (status != LIB_STATUS_OK) return status;
        core_debug_begin_response(output, operation, (lib_u32)data_size, 0u);
        core_debug_write_observation(output + 12u, &observation);
    } else if (operation == CORE_DEBUG_OUTPUT_SET) {
        driver->text_output = input[8u] != 0u;
        driver->published_frame_revision = 0u;
        core_debug_begin_response(output, operation, 0u, 0u);
    } else if (operation == CORE_DEBUG_PEEK) {
        lib_u16 address = core_debug_read_u16(input + 8u);
        lib_u16 count = core_debug_read_u16(input + 10u);
        lib_u8 bytes[256];
        status = core_machine_peek(driver->machine, address, count, bytes);
        if (status != LIB_STATUS_OK) return status;
        core_debug_begin_response(output, operation, (lib_u32)data_size, 0u);
        core_debug_write_u16(output + 12u, count);
        core_debug_write_u16(output + 14u, 0u);
        lib_memory_copy(output + 16u, bytes, count);
    } else if (operation == CORE_DEBUG_POKE) {
        status = core_machine_poke(driver->machine, core_debug_read_u16(input + 8u),
            input + 12u, core_debug_read_u16(input + 10u));
        if (status != LIB_STATUS_OK) return status;
        core_debug_begin_response(output, operation, 0u, 0u);
    } else if (operation == CORE_DEBUG_STEP) {
        status = core_machine_debug_step(driver->machine, core_debug_read_u32(input + 8u),
            8000u, &run_result);
        if (status != LIB_STATUS_OK) return status;
        status = core_machine_observe(driver->machine, &observation);
        if (status != LIB_STATUS_OK) return status;
        core_debug_begin_response(output, operation, (lib_u32)data_size,
            core_debug_domain_result(&run_result));
        core_debug_write_u32(output + 12u, run_result.instructions);
        core_debug_write_u32(output + 16u, run_result.cycles);
        core_debug_write_observation(output + 20u, &observation);
    } else if (operation == CORE_DEBUG_BREAK_SET) {
        status = core_machine_breakpoint_set(driver->machine,
            core_debug_read_u16(input + 8u), core_debug_read_u16(input + 10u) != 0u);
        if (status != LIB_STATUS_OK) return status;
        core_debug_begin_response(output, operation, 0u, 0u);
    } else if (operation == CORE_DEBUG_BREAK_LIST) {
        lib_u16 addresses[16];
        lib_u32 count;
        lib_u32 index;
        status = core_machine_breakpoint_list(driver->machine, addresses, 16u, &count);
        if (status != LIB_STATUS_OK) return status;
        total_size = CORE_DEBUG_HEADER_BYTES + CORE_DEBUG_DOMAIN_BYTES + 4u + count * 2u;
        core_debug_begin_response(output, operation, 4u + count * 2u, 0u);
        core_debug_write_u16(output + 12u, (lib_u16)count);
        core_debug_write_u16(output + 14u, 0u);
        for (index = 0u; index < count; ++index)
            core_debug_write_u16(output + 16u + index * 2u, addresses[index]);
    } else {
        status = core_machine_reset(driver->machine, CORE_RESET_WARM);
        if (status != LIB_STATUS_OK) return status;
        status = core_machine_observe(driver->machine, &observation);
        if (status != LIB_STATUS_OK) return status;
        core_debug_begin_response(output, operation, (lib_u32)data_size, 0u);
        core_debug_write_observation(output + 12u, &observation);
    }
    *response_size = total_size;
    return LIB_STATUS_OK;
}

common_machine_debug_execute core_driver_debug_execute(void)
{
    return core_driver_execute_debug;
}

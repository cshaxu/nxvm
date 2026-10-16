#include "lib/types/file.h"
#include "lib/types/test.h"
#include "lib/types/types_interface.h"
#include "x86-nxvm/machine/keyboard_mapper_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-at/kbc_interface.h"

static lib_i32 vm_keyboard_native_set2_expect(kvm_key key, lib_u16 scan,
    lib_u32 flags, lib_bool pressed, const lib_u8 *expected, lib_u8 count)
{
    kvm_input_event event = {0};
    x86_nxvm_machine_keyboard_sequence sequence;
    lib_u8 index;

    event.type = KVM_EVENT_KEY;
    event.data.key.key = key;
    event.data.key.scan_code = scan;
    event.data.key.flags = flags;
    event.data.key.pressed = pressed;
    if (x86_nxvm_machine_keyboard_map_kvm_event(&event, &sequence) !=
            LIB_STATUS_OK || sequence.count != count) {
        return 0;
    }
    for (index = 0u; index < count; ++index) {
        if (sequence.bytes[index] != expected[index]) return 0;
    }
    return 1;
}

static lib_bool vm_keyboard_native_set3_matrix(void)
{
    /* Physical key numbers join IBM set-1 (4-50--53) and set-3
     * (4-58--60) tables. 100h records E0 in this test table only;
     * the actual KVM event receives a scan code and a separate flag. */
    static const struct {
        lib_u8 key_number;
        lib_u16 set1;
        lib_u8 set3;
    } keys[] = {
        {1,0x29,0x0e}, {2,0x02,0x16}, {3,0x03,0x1e}, {4,0x04,0x26},
        {5,0x05,0x25}, {6,0x06,0x2e}, {7,0x07,0x36}, {8,0x08,0x3d},
        {9,0x09,0x3e}, {10,0x0a,0x46}, {11,0x0b,0x45}, {12,0x0c,0x4e},
        {13,0x0d,0x55}, {15,0x0e,0x66}, {16,0x0f,0x0d}, {17,0x10,0x15},
        {18,0x11,0x1d}, {19,0x12,0x24}, {20,0x13,0x2d}, {21,0x14,0x2c},
        {22,0x15,0x35}, {23,0x16,0x3c}, {24,0x17,0x43}, {25,0x18,0x44},
        {26,0x19,0x4d}, {27,0x1a,0x54}, {28,0x1b,0x5b}, {29,0x2b,0x5c},
        {30,0x3a,0x14}, {31,0x1e,0x1c}, {32,0x1f,0x1b}, {33,0x20,0x23},
        {34,0x21,0x2b}, {35,0x22,0x34}, {36,0x23,0x33}, {37,0x24,0x3b},
        {38,0x25,0x42}, {39,0x26,0x4b}, {40,0x27,0x4c}, {41,0x28,0x52},
        {43,0x1c,0x5a}, {44,0x2a,0x12}, {46,0x2c,0x1a}, {47,0x2d,0x22},
        {48,0x2e,0x21}, {49,0x2f,0x2a}, {50,0x30,0x32}, {51,0x31,0x31},
        {52,0x32,0x3a}, {53,0x33,0x41}, {54,0x34,0x49}, {55,0x35,0x4a},
        {57,0x36,0x59}, {58,0x1d,0x11}, {60,0x38,0x19}, {61,0x39,0x29},
        {62,0x138,0x39}, {64,0x11d,0x58}, {75,0x152,0x67}, {76,0x153,0x64},
        {79,0x14b,0x61}, {80,0x147,0x6e}, {81,0x14f,0x65}, {83,0x148,0x63},
        {84,0x150,0x60}, {85,0x149,0x6f}, {86,0x151,0x6d}, {89,0x14d,0x6a},
        {90,0x45,0x76}, {91,0x47,0x6c}, {92,0x4b,0x6b}, {93,0x4f,0x69},
        {95,0x135,0x77}, {96,0x48,0x75}, {97,0x4c,0x73}, {98,0x50,0x72},
        {99,0x52,0x70}, {100,0x37,0x7e}, {101,0x49,0x7d}, {102,0x4d,0x74},
        {103,0x51,0x7a}, {104,0x53,0x71}, {105,0x4a,0x84}, {106,0x4e,0x7c},
        {108,0x11c,0x79}, {110,0x01,0x08}, {112,0x3b,0x07}, {113,0x3c,0x0f},
        {114,0x3d,0x17}, {115,0x3e,0x1f}, {116,0x3f,0x27}, {117,0x40,0x2f},
        {118,0x41,0x37}, {119,0x42,0x3f}, {120,0x43,0x47}, {121,0x44,0x4f},
        {122,0x57,0x56}, {123,0x58,0x5e}, {124,0x137,0x57}, {125,0x46,0x5f},
        {126,0,0x62}
    };
    x86_nxvm_machine_keyboard_sequence sequence;
    kvm_input_event event = {.type = KVM_EVENT_KEY};
    lib_bool failed = LIB_FALSE;

    failed |= sizeof(keys) / sizeof(keys[0]) != 101u;
    for (lib_size index = 0u; index < sizeof(keys) / sizeof(keys[0]); ++index) {
        event.data.key.key = keys[index].key_number == 126u ? KVM_KEY_PAUSE :
            keys[index].key_number == 124u ? KVM_KEY_PRINT_SCREEN : 0u;
        event.data.key.scan_code = keys[index].set1 & 0xffu;
        event.data.key.flags = keys[index].set1 & 0x100u ? KVM_KEY_FLAG_EXTENDED : 0u;
        for (lib_u8 pressed = 0u; pressed <= 1u; ++pressed) {
            event.data.key.pressed = pressed != 0u;
            failed |= x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(&event,
                X86_NXVM_BOARD_KEYBOARD_SCAN_SET_3, &sequence) != LIB_STATUS_OK;
            failed |= sequence.count != (pressed ? 1u : 2u);
            failed |= sequence.bytes[pressed ? 0u : 1u] != keys[index].set3;
            if (!pressed) failed |= sequence.bytes[0] != 0xf0u;
        }
    }
    event = (kvm_input_event){.type = KVM_EVENT_KEY,
        .data.key = {.key = KVM_KEY_PAUSE, .pressed = LIB_TRUE}};
    for (lib_u16 set = 0u; set < 256u; ++set) {
        if (set >= 1u && set <= 3u) continue;
        sequence.count = 8u;
        failed |= x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(&event,
            (lib_u8)set, &sequence) != LIB_STATUS_UNSUPPORTED || sequence.count != 0u;
    }
    event.data.key.key = KVM_KEY_PRINT_SCREEN;
    failed |= x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(&event,
        X86_NXVM_BOARD_KEYBOARD_SCAN_SET_3, &sequence) != LIB_STATUS_OK ||
        sequence.count != 1u || sequence.bytes[0] != 0x57u;
    event.data.key.key = 0u;
    event.data.key.scan_code = 0x56u; /* 102-key-only position, not this layout. */
    failed |= x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(&event,
        X86_NXVM_BOARD_KEYBOARD_SCAN_SET_3, &sequence) != LIB_STATUS_UNSUPPORTED ||
        sequence.count != 0u;
    event.data.key.scan_code = 0x1eu;
    event.data.key.flags = KVM_KEY_FLAG_EXTENDED; /* No E0 A key. */
    failed |= x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(&event,
        X86_NXVM_BOARD_KEYBOARD_SCAN_SET_3, &sequence) != LIB_STATUS_UNSUPPORTED ||
        sequence.count != 0u;
    return failed;
}

static void mapped_keyboard_irq(void *context, lib_bool auxiliary, lib_bool asserted)
{
    x86_nxvm_board_pic_irq_source *source = context;
    lib_test_assert(!auxiliary);
    if (asserted) x86_nxvm_board_pic_irq_source_assert(source);
    else x86_nxvm_board_pic_irq_source_deassert(source);
}

static lib_bool mapped_keyboard_take(x86_core_machine *machine,
    x86_nxvm_board_pic_bus *master, x86_nxvm_board_pic_bus *slave,
    x86_nxvm_board_pic_irq_source *source, lib_u8 expected)
{
    lib_u32 value;
    if (!x86_nxvm_board_pic_irq_source_is_asserted(source) ||
        x86_nxvm_board_pic_get_interrupt(master, slave) != 9u ||
        x86_core_machine_bus_read(machine, 0x60u, &value) != LIB_STATUS_OK || value != expected ||
        x86_core_machine_bus_write(machine, 0x20u, 0x20u) != LIB_STATUS_OK) return LIB_TRUE;
    return LIB_FALSE;
}

static lib_bool vm_keyboard_set3_receiving_route(void)
{
    static const struct {
        kvm_key key;
        lib_u16 scan;
        lib_u32 flags;
        lib_u8 code;
        lib_bool has_break;
    } keys[] = {
        {'A',0x1e,0u,0x1c,LIB_FALSE},
        {KVM_KEY_UP,0x48,KVM_KEY_FLAG_EXTENDED,0x63,LIB_FALSE},
        {KVM_KEY_CONTROL,0x1d,0u,0x11,LIB_TRUE},
        {KVM_KEY_CONTROL,0x1d,KVM_KEY_FLAG_EXTENDED,0x58,LIB_FALSE},
        {KVM_KEY_ALT,0x38,0u,0x19,LIB_TRUE},
        {KVM_KEY_ALT,0x38,KVM_KEY_FLAG_EXTENDED,0x39,LIB_FALSE},
        {KVM_KEY_SHIFT,0x2a,0u,0x12,LIB_TRUE},
        {KVM_KEY_SHIFT,0x36,0u,0x59,LIB_TRUE},
        {KVM_KEY_PAUSE,0u,0u,0x62,LIB_FALSE},
        {KVM_KEY_PRINT_SCREEN,0u,0u,0x57,LIB_FALSE}
    };
    const x86_core_machine_executor_config config = {
        .cpu_profile = X86_CORE_CPU_PROFILE_80386,
        .memory_bytes = (2u * 1024u * 1024u)
    };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_pic_bus *master = LIB_NULL;
    x86_nxvm_board_pic_bus *slave = LIB_NULL;
    x86_nxvm_board_pic_irq_source *source = LIB_NULL;
    t_kbc *keyboard = LIB_NULL;
    lib_bool failed = LIB_FALSE;
    lib_u32 value;
    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_initialize(&master, &slave, machine,
            X86_NXVM_BOARD_PIC_TOPOLOGY_CASCADED) != LIB_STATUS_OK ||
        x86_nxvm_board_kbc_create(machine, &keyboard) != LIB_STATUS_OK ||
        x86_nxvm_board_pic_irq_source_bind(&source, master, slave, 1u) != LIB_STATUS_OK) {
        failed = LIB_TRUE;
        goto done;
    }
    x86_nxvm_board_kbc_bind_core_services(keyboard, mapped_keyboard_irq, source,
        LIB_NULL, LIB_NULL, LIB_NULL, LIB_NULL, LIB_FALSE);
    if (x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) {
        failed = LIB_TRUE;
        goto done;
    }
    x86_nxvm_board_kbc_set_command_response_timing(keyboard, 0u);
    x86_nxvm_board_kbc_set_command_response_status_polls(keyboard, 0u);
    x86_nxvm_board_kbc_set_serial_delivery_timing(keyboard, 0u);
    failed |= x86_nxvm_board_pic_write_register(master, 0u, 0x13u) != LIB_STATUS_OK;
    failed |= x86_nxvm_board_pic_write_register(master, 1u, 8u) != LIB_STATUS_OK;
    failed |= x86_nxvm_board_pic_write_register(master, 1u, 1u) != LIB_STATUS_OK;
    failed |= x86_core_machine_bus_write(machine, 0x60u, 0xf0u) != LIB_STATUS_OK;
    failed |= mapped_keyboard_take(machine, master, slave, source, 0xfau);
    failed |= x86_core_machine_bus_write(machine, 0x60u, 3u) != LIB_STATUS_OK;
    failed |= mapped_keyboard_take(machine, master, slave, source, 0xfau);
    failed |= x86_nxvm_board_kbc_get_native_scan_set(keyboard) != 3u;
    for (lib_u8 all_breaks = 0u; all_breaks < 2u; ++all_breaks) {
        if (all_breaks != 0u) {
            failed |= x86_core_machine_bus_write(machine, 0x60u, 0xfau) != LIB_STATUS_OK;
            failed |= mapped_keyboard_take(machine, master, slave, source, 0xfau);
        }
        for (lib_size index = 0u; index < sizeof(keys) / sizeof(keys[0]); ++index) {
            kvm_input_event event = {.type = KVM_EVENT_KEY,
                .data.key = {.key = keys[index].key, .scan_code = keys[index].scan,
                    .flags = keys[index].flags}};
            for (lib_u8 pressed = 1u; pressed < 3u; ++pressed) {
                x86_nxvm_machine_keyboard_sequence sequence;
                event.data.key.pressed = pressed == 1u;
                failed |= x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(&event,
                    x86_nxvm_board_kbc_get_native_scan_set(keyboard), &sequence) != LIB_STATUS_OK;
                failed |= x86_nxvm_board_kbc_submit_native_bytes(keyboard, sequence.bytes,
                    sequence.count) != LIB_STATUS_OK;
                if (pressed == 1u || all_breaks != 0u || keys[index].has_break) {
                    if (pressed != 1u)
                        failed |= mapped_keyboard_take(machine, master, slave, source, 0xf0u);
                    failed |= mapped_keyboard_take(machine, master, slave, source, keys[index].code);
                }
                failed |= x86_nxvm_board_pic_irq_source_is_asserted(source);
                failed |= x86_core_machine_bus_read(machine, 0x64u, &value) != LIB_STATUS_OK ||
                    (value & 1u) != 0u;
            }
        }
    }
done:
    x86_nxvm_board_kbc_destroy(keyboard);
    x86_nxvm_board_pic_finalize(master, slave);
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    static const lib_u8 function_set1[] = { 0x3bu, 0x3cu, 0x3du,
        0x3eu, 0x3fu, 0x40u, 0x41u, 0x42u, 0x43u, 0x44u, 0x57u, 0x58u };
    static const lib_u8 function_set2[] = { 0x05u, 0x06u, 0x04u,
        0x0cu, 0x03u, 0x0bu, 0x83u, 0x0au, 0x01u, 0x09u, 0x78u, 0x07u };
    static const lib_u8 a_make[] = { 0x1cu };
    static const lib_u8 a_break[] = { 0xf0u, 0x1cu };
    static const lib_u8 up_make[] = { 0xe0u, 0x75u };
    static const lib_u8 up_break[] = { 0xe0u, 0xf0u, 0x75u };
    static const lib_u8 right_make[] = { 0xe0u, 0x74u };
    static const lib_u8 left_control_make[] = { 0x14u };
    static const lib_u8 right_control_make[] = { 0xe0u, 0x14u };
    static const lib_u8 left_alt_break[] = { 0xf0u, 0x11u };
    static const lib_u8 right_alt_break[] = { 0xe0u, 0xf0u, 0x11u };
    static const lib_u8 pause_make[] = {
        0xe1u, 0x14u, 0x77u, 0xe1u, 0xf0u, 0x14u, 0xf0u, 0x77u
    };
    lib_u8 index;
    x86_nxvm_machine_keyboard_sequence set1;

    {
        kvm_input_event event = { .type = KVM_EVENT_KEY };
        event.data.key.key = KVM_KEY_UP;
        event.data.key.scan_code = 0x48u;
        event.data.key.flags = KVM_KEY_FLAG_EXTENDED;
        event.data.key.pressed = LIB_TRUE;
        if (x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(&event,
                X86_NXVM_BOARD_KEYBOARD_SCAN_SET_1, &set1) != LIB_STATUS_OK ||
            set1.count != 2u || set1.bytes[0u] != 0xe0u || set1.bytes[1u] != 0x48u)
            return 1;
        event.data.key.pressed = LIB_FALSE;
        if (x86_nxvm_machine_keyboard_map_kvm_event_for_scan_set(&event,
                X86_NXVM_BOARD_KEYBOARD_SCAN_SET_1, &set1) != LIB_STATUS_OK ||
            set1.count != 2u || set1.bytes[1u] != 0xc8u) return 1;
    }

    if (!vm_keyboard_native_set2_expect('A', 0x1eu, 0u, LIB_TRUE, a_make, sizeof(a_make)) ||
        !vm_keyboard_native_set2_expect('A', 0x1eu, 0u, LIB_FALSE, a_break, sizeof(a_break)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_UP, 0x48u, KVM_KEY_FLAG_EXTENDED, LIB_TRUE, up_make,
            sizeof(up_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_UP, 0x48u, KVM_KEY_FLAG_EXTENDED, LIB_FALSE, up_break,
            sizeof(up_break)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_RIGHT, 0u, KVM_KEY_FLAG_EXTENDED, LIB_TRUE, right_make,
            sizeof(right_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_CONTROL, 0x1du, 0u, LIB_TRUE,
            left_control_make, sizeof(left_control_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_CONTROL, 0x1du, KVM_KEY_FLAG_EXTENDED, LIB_TRUE,
            right_control_make, sizeof(right_control_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_ALT, 0x38u, 0u, LIB_FALSE,
            left_alt_break, sizeof(left_alt_break)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_ALT, 0x38u, KVM_KEY_FLAG_EXTENDED, LIB_FALSE,
            right_alt_break, sizeof(right_alt_break)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_PAUSE, 0u, 0u, LIB_TRUE, pause_make,
            sizeof(pause_make)) ||
        !vm_keyboard_native_set2_expect(KVM_KEY_PAUSE, 0u, 0u, LIB_FALSE, LIB_NULL, 0u)) {
        return 1;
    }
    for (index = 0u; index < sizeof(function_set1); ++index) {
        if (!vm_keyboard_native_set2_expect(KVM_KEY_F1 + index,
                function_set1[index], 0u, LIB_TRUE,
                &function_set2[index], 1u)) return 1;
    }
    if (vm_keyboard_native_set3_matrix()) return 1;
    if (vm_keyboard_set3_receiving_route()) return 1;
    lib_c_printf("HOST-KEY-TO-NATIVE-SCAN-SETS:OK\n");
    return 0;
}

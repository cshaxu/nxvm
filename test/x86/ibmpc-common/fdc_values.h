#ifndef TEST_FDC_VALUES_H
#define TEST_FDC_VALUES_H

/* Wire values asserted by board tests, independent of chip-private layout. */
enum {
    TEST_FDC_MSR_CB = 0x10u,
    TEST_FDC_MSR_NDM = 0x20u,
    TEST_FDC_MSR_DIO = 0x40u,
    TEST_FDC_MSR_RQM = 0x80u,
    TEST_FDC_MSR_READY_READ = 0xc0u,
    TEST_FDC_MSR_PROCESS_READ = 0xd0u,
    TEST_FDC_MSR_RESULT = 0xd0u,
    TEST_FDC_ST0_NORMAL = 0x00u,
    TEST_FDC_ST0_SEEK_END = 0x20u,
    TEST_FDC_ST0_EQUIPMENT_CHECK = 0x10u,
    TEST_FDC_ST0_ABNORMAL = 0x40u,
    TEST_FDC_ST0_READY_CHANGE = 0xc0u,
    TEST_FDC_ST2_SCAN_MATCH = 0x08u,
    TEST_FDC_ST2_SCAN_MISMATCH = 0x04u,
    TEST_FDC_ST2_CONTROL_MARK = 0x40u
};
#endif

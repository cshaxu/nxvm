#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


static void ega_write_crtc(x86_video *video, x86_video_register index_register,
    lib_u8 index, lib_u8 value)
{
    video_test_write(video, index_register, index);
    video_test_write(video, index_register == X86_VIDEO_REGISTER_MONO_CRTC_INDEX ?
        X86_VIDEO_REGISTER_MONO_CRTC_DATA : X86_VIDEO_REGISTER_COLOR_CRTC_DATA, value);
}

static lib_u8 ega_read_crtc(x86_video *video, x86_video_register index_register,
    lib_u8 index)
{
    video_test_write(video, index_register, index);
    return video_test_read(video, index_register == X86_VIDEO_REGISTER_MONO_CRTC_INDEX ?
        X86_VIDEO_REGISTER_MONO_CRTC_DATA : X86_VIDEO_REGISTER_COLOR_CRTC_DATA);
}

lib_i32 main(void)
{
    const x86_video_ega_sequencer_config sequencer = {
        X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
        0x03u, 0x00u, 0x0fu, 0x02u, LIB_TRUE
    };
    const x86_video_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x01u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    x86_video *vadp = LIB_NULL;
    x86_video_snapshot snapshot;
    x86_video_snapshot_observation observation;
    const lib_u8 chain4_bytes[] = { 0x10u, 0x11u, 0x12u, 0x13u };
    lib_u64 vga_generation;
    lib_i32 failed = 0;

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_sequencer(vadp, &sequencer) != LIB_STATUS_OK ||
        x86_video_configure_ega_controllers(vadp, &controllers) != LIB_STATUS_OK ||
        x86_video_configure_ega_personality(vadp, X86_VIDEO_EGA_PERSONALITY_GENERIC) != LIB_STATUS_OK ||
        x86_video_configure_vga(vadp) != LIB_STATUS_OK;
    x86_video_reset(vadp);
    failed |= (video_test_read(vadp,
        X86_VIDEO_REGISTER_EXTERNAL_CONTROL) & 0x7fu) != 0u;

    ega_write_crtc(vadp, X86_VIDEO_REGISTER_MONO_CRTC_INDEX, 0x0eu, 0x12u);
    failed |= ega_read_crtc(vadp, X86_VIDEO_REGISTER_MONO_CRTC_INDEX, 0x0eu) !=
        0x12u || ega_read_crtc(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0eu) !=
        0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL,
        0x01u);
    ega_write_crtc(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0eu, 0x34u);
    failed |= ega_read_crtc(vadp, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x0eu) !=
        0x34u || ega_read_crtc(vadp, X86_VIDEO_REGISTER_MONO_CRTC_INDEX,
        0x0eu) != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_MONO_STATUS,
        0x03u);
    video_test_write(vadp, X86_VIDEO_REGISTER_COLOR_STATUS,
        0x02u);
    failed |= vadp->data.ega_feature_control != 0x02u;

    video_test_write(vadp, X86_VIDEO_REGISTER_DAC_WRITE_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_DAC_DATA, 0x7fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_DAC_DATA, 0x15u);
    video_test_write(vadp, X86_VIDEO_REGISTER_DAC_DATA, 0x2au);
    video_test_write(vadp, X86_VIDEO_REGISTER_DAC_READ_INDEX, 2u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_DAC_DATA) !=
        0x3fu || video_test_read(vadp, X86_VIDEO_REGISTER_DAC_DATA) !=
        0x15u || video_test_read(vadp, X86_VIDEO_REGISTER_DAC_DATA) !=
        0x2au;
    video_test_write(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL, 0xa5u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL) !=
        0xa5u;

    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x0eu);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 5u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x40u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x05u);
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x30u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x01u);
    failed |= x86_video_memory_write(vadp, X86_VIDEO_MEMORY_PLANAR,
        X86_VIDEO_EGA_APERTURE_BASE, chain4_bytes,
        sizeof(chain4_bytes)) != LIB_STATUS_OK;
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= !x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) ||
        snapshot.kind != X86_VIDEO_KIND_VGA_320X200X256 ||
        snapshot.pixel_width != 320u || snapshot.pixel_height != 200u ||
        snapshot.pixels[0] != 0x10u || snapshot.pixels[1] != 0x11u ||
        snapshot.pixels[2] != 0x12u || snapshot.pixels[3] != 0x13u;
    x86_video_observe_snapshot(vadp, LIB_FALSE, 0u, &observation);
    vga_generation = observation.generation;
    x86_video_observe_snapshot(vadp, LIB_TRUE, vga_generation, &observation);
    failed |= !observation.generation_reliable || observation.capture_required;
    video_test_write(vadp, X86_VIDEO_REGISTER_DAC_DATA, 0x1fu);
    x86_video_observe_snapshot(vadp, LIB_TRUE, vga_generation, &observation);
    failed |= !observation.generation_reliable || !observation.capture_required ||
        observation.generation == vga_generation;

    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x00u);
    failed |= !vadp->data.attribute_data_phase;
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    failed |= vadp->data.attribute_data_phase;
    x86_video_reset(vadp);
    failed |= vadp->data.ega_miscellaneous_output != 0u ||
        vadp->data.ega_feature_control != 0u ||
        video_test_read(vadp, X86_VIDEO_REGISTER_AUXILIARY_CONTROL) != 0xffu ||
        ega_read_crtc(vadp, X86_VIDEO_REGISTER_MONO_CRTC_INDEX, 0x0eu) !=
        0u;

    x86_video_destroy(vadp);
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "EGA-EXTERNAL-PORT:FAIL\n");
        return 1;
    }
    lib_c_printf("EGA-EXTERNAL-PORT:OK\n");
    lib_c_printf("DAC:OK\n");
    lib_c_printf("CHAIN4:OK\n");
    lib_c_printf("SNAPSHOT:OK\n");
    return 0;
}

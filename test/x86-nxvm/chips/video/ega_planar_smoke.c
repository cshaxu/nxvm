#include "video_test.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"


static lib_i32 ega_planar_write(x86_video *video, lib_u32 physical,
    lib_u8 value)
{
    return x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR, physical,
        &value, sizeof(value)) == LIB_STATUS_OK;
}

static lib_i32 ega_planar_read(x86_video *video, lib_u32 physical,
    lib_u8 *value)
{
    return x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR, physical,
        value, sizeof(*value)) == LIB_STATUS_OK;
}

static void ega_graphics_write(x86_video *port, lib_u8 index,
    lib_u8 value)
{
    video_test_write(port, X86_VIDEO_REGISTER_GRAPHICS_INDEX, index);
    video_test_write(port, X86_VIDEO_REGISTER_GRAPHICS_DATA, value);
}

static void ega_planar_crtc_write(x86_video *video, lib_u8 index, lib_u8 value)
{
    video_test_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, index);
    video_test_write(video, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, value);
}

static void ega_planar_select_mode_d(x86_video *port)
{
    (void)video_test_read(port, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(port, X86_VIDEO_REGISTER_ATTRIBUTE, 0x30u);
    video_test_write(port, X86_VIDEO_REGISTER_ATTRIBUTE, 1u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x01u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x27u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x07u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x10u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x17u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0xe3u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x18u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0xffu);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x12u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0xc7u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_INDEX, 0x13u);
    video_test_write(port, X86_VIDEO_REGISTER_COLOR_CRTC_DATA, 0x14u);
}

static lib_bool ega_planar_ram_gate_matches(void)
{
    const x86_video_ega_sequencer_config sequencer = {
        0xa0000u, 0x10000u, 3u, 1u, 15u, 6u, LIB_TRUE };
    const x86_video_ega_controller_config controllers = {
        .graphics = { [6] = 5u, [8] = 0xffu } };
    lib_bool valid = LIB_TRUE;

    for (lib_u8 kind = 0u; kind < 2u; ++kind) {
        x86_video *video = LIB_NULL;
        x86_video_bus_observation observation;
        const lib_u8 enabled = 3u;
        const lib_u8 disabled = 1u;
        lib_u8 value = 0x5au;
        lib_u8 read;
        lib_u64 generation;

        if (x86_video_create(&video) != LIB_STATUS_OK) return LIB_FALSE;
        valid &= x86_video_configure_ega_sequencer(video, &sequencer) == LIB_STATUS_OK &&
            x86_video_configure_ega_controllers(video, &controllers) == LIB_STATUS_OK &&
            x86_video_configure_ega_personality(video, kind == 0u ?
                X86_VIDEO_EGA_PERSONALITY_GENERIC :
                X86_VIDEO_EGA_PERSONALITY_COMPAQ_ENHANCED_COLOR) == LIB_STATUS_OK;
        video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, enabled);
        valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
            0xa0000u, &value, 1u) == LIB_STATUS_OK;
        video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, disabled);
        generation = video->data.dirty_generation;
        read = 0xa5u;
        valid &= x86_video_observe_bus(video, &observation) == LIB_STATUS_OK &&
            observation.cpu_memory_disabled &&
            x86_video_memory_query(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, 1u, LIB_FALSE) == LIB_STATUS_UNSUPPORTED &&
            x86_video_memory_query(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, 1u, LIB_TRUE) == LIB_STATUS_UNSUPPORTED &&
            x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &read, 1u) == LIB_STATUS_UNSUPPORTED && read == 0xa5u;
        value = 0x96u;
        valid &= x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
            0xa0000u, &value, 1u) == LIB_STATUS_UNSUPPORTED &&
            video->data.dirty_generation == generation;
        video_test_write(video, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, enabled);
        read = 0u;
        valid &= x86_video_observe_bus(video, &observation) == LIB_STATUS_OK &&
            !observation.cpu_memory_disabled &&
            x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &read, 1u) == LIB_STATUS_OK && read == 0x5au;
        /* Attribute blanking changes scanout, not processor RAM access. */
        (void)video_test_read(video, X86_VIDEO_REGISTER_COLOR_STATUS);
        video_test_write(video, X86_VIDEO_REGISTER_ATTRIBUTE, 0u);
        valid &= x86_video_memory_query(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, 1u, LIB_TRUE) == LIB_STATUS_OK &&
            x86_video_memory_write(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &value, 1u) == LIB_STATUS_OK;
        read = 0u;
        valid &= x86_video_memory_read(video, X86_VIDEO_MEMORY_PLANAR,
                0xa0000u, &read, 1u) == LIB_STATUS_OK && read == 0x96u;
        x86_video_destroy(video);
    }
    return valid;
}

lib_i32 main(void)
{
    const x86_video_ega_sequencer_config sequencer = {
        X86_VIDEO_EGA_APERTURE_BASE, X86_VIDEO_EGA_APERTURE_BYTES,
        0x03u, 0x01u, 0x0fu, 0x06u, LIB_TRUE
    };
    const x86_video_ega_controller_config controllers = {
        { 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x05u, 0x00u, 0xffu },
        { 0x00u, 0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u, 0x07u,
            0x08u, 0x09u, 0x0au, 0x0bu, 0x0cu, 0x0du, 0x0eu, 0x0fu,
            0x00u, 0x00u, 0x0fu, 0x00u, 0x00u }
    };
    x86_video *vadp = LIB_NULL;
    video_test_backing backing = {0};
    const x86_video_memory_reader reader = {video_test_backing_read, &backing};
    lib_u8 value = 0u;
    lib_u8 status_first = 0u;
    lib_u8 status_second = 0u;
    x86_video_snapshot snapshot;
    x86_video_kind copied_kind;
    lib_u8 copied_pixel_zero;
    lib_u8 copied_pixel_two;
    lib_u32 copied_palette_fifteen;
    lib_i32 failed = !ega_planar_ram_gate_matches();

    if (x86_video_create(&vadp) != LIB_STATUS_OK) return 1;
    failed |= x86_video_configure_ega_sequencer(vadp, &sequencer) != LIB_STATUS_OK ||
        x86_video_configure_ega_controllers(vadp, &controllers) != LIB_STATUS_OK;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);

    /* EGA text and planar graphics share the Attribute Controller's
       display-enable state; it is not a renderer-local visibility flag. */
    video_test_text_geometry(vadp, 80u, 25u, 8u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x0eu);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 3u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 3u);
    ega_graphics_write(vadp, 5u, 0x10u);
    video_test_write(vadp, X86_VIDEO_REGISTER_MODE, 0x09u);
    value = 'T';
    failed |= x86_video_memory_write(vadp, X86_VIDEO_MEMORY_PLANAR, X86_VIDEO_TEXT_BASE,
        &value, 1u) != LIB_STATUS_OK;
    value = 0x1fu;
    failed |= x86_video_memory_write(vadp, X86_VIDEO_MEMORY_PLANAR,
        X86_VIDEO_TEXT_BASE + 1u, &value, 1u) != LIB_STATUS_OK;
    failed |= x86_video_capture_snapshot_from(vadp, &reader, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_TEXT || snapshot.characters[0] != 'T';
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x00u);
    failed |= x86_video_capture_snapshot_from(vadp, &reader, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_TEXT || snapshot.characters[0] != 0x20u ||
        snapshot.attributes[0] != 0u;
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x20u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 4u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 6u);
    ega_graphics_write(vadp, 5u, 0u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x05u);
    ega_planar_select_mode_d(vadp);

    status_first = video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    status_second = video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    failed |= (status_first & 0x30u) != 0u || (status_second & 0x30u) != 0u;

    failed |= !x86_video_ega_aperture_contains(vadp, 0x000a0000u,
        0x00010000u);
    failed |= x86_video_ega_aperture_contains(vadp, 0x000b0000u, 1u);
    failed |= x86_video_ega_aperture_contains(vadp, 0x000a0000u,
        0x00010001u);

    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x0fu);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0xffu ||
        vadp->data.sequencer[2] != 0x0fu;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 5u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x00u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA) != 0xffu ||
        vadp->data.graphics[5] != 0x00u;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA) != 0xffu ||
        vadp->data.graphics[6] != 0x05u;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x05u);
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x30u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x01u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_ATTRIBUTE_DATA) != 0xffu ||
        vadp->data.attribute[16] != 0x01u;

    failed |= !ega_planar_write(vadp, 0x000a0000u, 0xa5u);
    failed |= !ega_planar_read(vadp, 0x000a0000u, &value) ||
        value != 0xa5u;
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= x86_video_capture_snapshot_from(vadp, &reader, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_EGA_320X200X16 ||
        snapshot.pixel_width != 320u || snapshot.pixel_height != 200u ||
        snapshot.pixels[0] != 15u || snapshot.pixels[1] != 0u ||
        snapshot.pixels[2] != 15u || snapshot.palette_rgb[15] != 0xaaaaffu;
    copied_kind = snapshot.kind;
    copied_pixel_zero = snapshot.pixels[0];
    copied_pixel_two = snapshot.pixels[2];
    copied_palette_fifteen = snapshot.palette_rgb[15];
    /* GC graphics scanout remains graphics with text attribute interpretation. */
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x30u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x00u);
    failed |= x86_video_capture_snapshot_from(vadp, &reader, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_EGA_320X200X16 || snapshot.pixels[0] != 15u;
    (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x30u);
    video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x01u);

    /* Read mode 1 compares the four latches; mode 1 copies them and mode 2
     * expands the four low processor-data bits into the selected planes. */
    ega_graphics_write(vadp, 1u, 0u);
    ega_graphics_write(vadp, 3u, 0u);
    ega_graphics_write(vadp, 8u, 0xffu);
    ega_graphics_write(vadp, 5u, 0u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x01u);
    failed |= !ega_planar_write(vadp, 0x000a0003u, 0xaau);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x02u);
    failed |= !ega_planar_write(vadp, 0x000a0003u, 0x55u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x04u);
    failed |= !ega_planar_write(vadp, 0x000a0003u, 0xf0u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x08u);
    failed |= !ega_planar_write(vadp, 0x000a0003u, 0x0fu);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x0fu);
    ega_graphics_write(vadp, 4u, 0u);
    failed |= !ega_planar_read(vadp, 0x000a0003u, &value) ||
        value != 0xaau;
    ega_graphics_write(vadp, 2u, 0x01u);
    ega_graphics_write(vadp, 7u, 0x0eu);
    ega_graphics_write(vadp, 5u, 0x08u);
    failed |= !ega_planar_read(vadp, 0x000a0003u, &value) ||
        value != 0xaau;
    ega_graphics_write(vadp, 5u, 0u);
    ega_graphics_write(vadp, 4u, 0x02u);
    failed |= !ega_planar_read(vadp, 0x000a0003u, &value) ||
        value != 0xf0u;
    ega_graphics_write(vadp, 4u, 0x04u);
    failed |= !ega_planar_read(vadp, 0x000a0003u, &value) ||
        value != 0u;
    ega_graphics_write(vadp, 5u, 0x01u);
    failed |= !ega_planar_write(vadp, 0x000a0004u, 0u);
    ega_graphics_write(vadp, 5u, 0u);
    ega_graphics_write(vadp, 4u, 0x03u);
    failed |= !ega_planar_read(vadp, 0x000a0004u, &value) ||
        value != 0x0fu;
    ega_graphics_write(vadp, 5u, 0x02u);
    failed |= !ega_planar_write(vadp, 0x000a0005u, 0x05u);
    ega_graphics_write(vadp, 5u, 0u);
    ega_graphics_write(vadp, 4u, 0x00u);
    failed |= !ega_planar_read(vadp, 0x000a0005u, &value) ||
        value != 0xffu;
    ega_graphics_write(vadp, 4u, 0x01u);
    failed |= !ega_planar_read(vadp, 0x000a0005u, &value) ||
        value != 0u;
    ega_graphics_write(vadp, 4u, 0x02u);
    failed |= !ega_planar_read(vadp, 0x000a0005u, &value) ||
        value != 0xffu;
    ega_graphics_write(vadp, 4u, 0x03u);
    failed |= !ega_planar_read(vadp, 0x000a0005u, &value) ||
        value != 0u;
    ega_graphics_write(vadp, 5u, 0x04u);
    failed |= x86_video_memory_query(vadp, X86_VIDEO_MEMORY_PLANAR, 0x000a0005u, 1u,
        LIB_FALSE) != LIB_STATUS_UNSUPPORTED ||
        x86_video_capture_snapshot_from(vadp, &reader, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_EGA_320X200X16 ||
        snapshot.pixels[0] != 0u;
    ega_graphics_write(vadp, 5u, 0u);

    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 0u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x02u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0xffu ||
        vadp->data.sequencer[0] != 0x02u ||
        x86_video_memory_write(vadp, X86_VIDEO_MEMORY_PLANAR,
        0xa0000u, &value, 1u) != LIB_STATUS_UNSUPPORTED ||
        x86_video_memory_query(vadp, X86_VIDEO_MEMORY_PLANAR, 0x000a0000u, 1u,
        LIB_TRUE) != LIB_STATUS_UNSUPPORTED ||
        x86_video_capture_snapshot_from(vadp, &reader, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_EGA_320X200X16 ||
        snapshot.pixels[0] != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x01u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0xffu ||
        vadp->data.sequencer[0] != 0x01u ||
        x86_video_memory_read(vadp, X86_VIDEO_MEMORY_PLANAR,
        0xa0000u, &value, 1u) != LIB_STATUS_UNSUPPORTED ||
        x86_video_memory_query(vadp, X86_VIDEO_MEMORY_PLANAR, 0x000a0000u, 1u,
        LIB_FALSE) != LIB_STATUS_UNSUPPORTED ||
        x86_video_capture_snapshot_from(vadp, &reader, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_EGA_320X200X16 ||
        snapshot.pixels[0] != 0u;
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x03u);
    failed |= video_test_read(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA) != 0xffu ||
        vadp->data.sequencer[0] != 0x03u ||
        !ega_planar_read(vadp, 0x000a0000u, &value) ||
        value != 0xa5u || x86_video_memory_query(vadp, X86_VIDEO_MEMORY_PLANAR, 0x000a0000u, 1u,
        LIB_FALSE) != LIB_STATUS_OK;

    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x02u);
    failed |= !ega_planar_write(vadp, 0x000a0001u, 0x80u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 4u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x01u);
    failed |= !ega_planar_read(vadp, 0x000a0001u, &value) ||
        value != 0x80u;
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 0u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x01u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 1u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x01u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 3u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x00u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 8u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0xffu);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_INDEX, 2u);
    video_test_write(vadp, X86_VIDEO_REGISTER_SEQUENCER_DATA, 0x01u);
    failed |= !ega_planar_write(vadp, 0x000a0002u, 0x00u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 4u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x00u);
    failed |= !ega_planar_read(vadp, 0x000a0002u, &value) ||
        value != 0xffu;

    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x09u);
    failed |= !x86_video_ega_aperture_contains(vadp, 0x000b0000u,
        0x00008000u);
    failed |= x86_video_memory_read(vadp, X86_VIDEO_MEMORY_PLANAR,
        0xa0000u, &value, 1u) != LIB_STATUS_UNSUPPORTED;

    /* Reset clears the transient planar store; a guest mode write re-arms it. */
    x86_video_reset(vadp);
    failed |= x86_video_memory_query(vadp, X86_VIDEO_MEMORY_PLANAR,
        0xa0000u, 1u, LIB_FALSE) != LIB_STATUS_UNSUPPORTED;
    video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
    ega_planar_select_mode_d(vadp);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_INDEX, 6u);
    video_test_write(vadp, X86_VIDEO_REGISTER_GRAPHICS_DATA, 0x05u);
    status_first = video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    status_second = video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
    failed |= (status_first & 0x30u) != 0u || (status_second & 0x30u) != 0u;

    failed |= !x86_video_ega_aperture_contains(vadp, 0x000a0000u,
        0x00010000u);
    failed |= !ega_planar_read(vadp, 0x000a0000u, &value) ||
        value != 0u;
    lib_memory_set(&snapshot, 0, sizeof(snapshot));
    failed |= x86_video_capture_snapshot_from(vadp, &reader, &snapshot) != LIB_STATUS_OK ||
        snapshot.kind != X86_VIDEO_KIND_EGA_320X200X16 ||
        snapshot.pixels[0] != 0u || !snapshot.buffer_changed;
    {
        static const struct {
            lib_u8 mode, stride, maximum, preset;
            lib_u16 compare;
            lib_u8 pan;
            lib_u16 start, x, y, address;
            lib_u8 bit;
        } cases[] = {
            {0xe3u,0u,0u,0u,511u,0u,0u,0u,1u,0u,0u},
            {0xe3u,20u,0u,0u,511u,0u,0u,0u,1u,40u,0u},
            {0xe3u,1u,0u,0u,511u,0u,0u,0u,1u,2u,0u},
            {0xe3u,255u,0u,0u,511u,0u,0u,0u,199u,35954u,0u},
            {0xa3u,20u,0u,0u,511u,0u,0u,0u,1u,80u,0u},
            {0xe3u,20u,3u,2u,511u,0u,0u,0u,2u,40u,0u},
            {0xe2u,20u,1u,0u,511u,0u,0u,0u,1u,8192u,0u},
            {0xe1u,20u,3u,0u,511u,0u,0u,0u,2u,16384u,0u},
            {0xe0u,20u,3u,0u,511u,0u,0u,0u,3u,24576u,0u},
            {0xe7u,20u,0u,0u,511u,0u,0u,0u,1u,0u,0u},
            {0xe7u,20u,0u,0u,511u,0u,0u,0u,2u,40u,0u},
            {0xebu,20u,0u,0u,511u,0u,0u,8u,0u,0u,0u},
            {0xebu,20u,0u,0u,511u,0u,0u,16u,0u,1u,0u},
            {0xe3u,20u,0u,0u,511u,3u,0u,0u,0u,0u,3u},
            {0xe3u,20u,0u,0u,511u,3u,0u,6u,0u,1u,1u},
            {0xe3u,20u,0u,0u,3u,0u,2u,0u,3u,122u,0u},
            {0xe3u,20u,0u,0u,3u,3u,2u,0u,4u,0u,3u},
            {0xe3u,20u,0u,0u,3u,0u,2u,0u,5u,40u,0u},
            {0xe3u,20u,3u,3u,1u,0u,2u,0u,2u,0u,0u},
            {0xe3u,20u,0u,0u,511u,0u,65535u,8u,0u,0u,0u},
            {0xa3u,20u,0u,0u,511u,0u,32768u,0u,0u,1u,0u},
            {0x83u,20u,0u,0u,511u,0u,8192u,0u,0u,16385u,0u}
        };
        lib_u8 *seed = lib_allocate(X86_VIDEO_EGA_PLANE_BYTES);
        x86_video_snapshot *saved = lib_allocate(sizeof(*saved));

        if (seed == LIB_NULL || saved == LIB_NULL) {
            lib_release(seed);
            lib_release(saved);
            x86_video_destroy(vadp);
            return 1;
        }
        video_test_write(vadp, X86_VIDEO_REGISTER_EXTERNAL_CONTROL, 3u);
        for (lib_u32 address = 0u; address < X86_VIDEO_EGA_PLANE_BYTES; ++address)
            seed[address] = (lib_u8)(address ^ (address >> 8u) ^ 0x69u);
        failed |= x86_video_memory_write(vadp, X86_VIDEO_MEMORY_PLANAR,
            X86_VIDEO_EGA_APERTURE_BASE, seed, X86_VIDEO_EGA_PLANE_BYTES) != LIB_STATUS_OK;
        for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
            ega_planar_crtc_write(vadp, 0x17u, cases[index].mode);
            ega_planar_crtc_write(vadp, 0x13u, cases[index].stride);
            ega_planar_crtc_write(vadp, 0x09u, cases[index].maximum);
            ega_planar_crtc_write(vadp, 0x08u, cases[index].preset);
            ega_planar_crtc_write(vadp, 0x07u, cases[index].compare >= 256u ? 0x10u : 0u);
            ega_planar_crtc_write(vadp, 0x18u, (lib_u8)cases[index].compare);
            ega_planar_crtc_write(vadp, 0x12u, (cases[index].mode & 4u) != 0u ? 99u : 199u);
            ega_planar_crtc_write(vadp, 0x0cu, (lib_u8)(cases[index].start >> 8u));
            ega_planar_crtc_write(vadp, 0x0du, (lib_u8)cases[index].start);
            (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
            video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x33u);
            video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, cases[index].pan);
            failed |= x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) != LIB_STATUS_OK ||
                snapshot.kind != X86_VIDEO_KIND_EGA_320X200X16 ||
                snapshot.pixel_width != 320u || snapshot.pixel_height != 200u ||
                snapshot.pixels[cases[index].y * 320u + cases[index].x] !=
                    ((seed[cases[index].address] & (0x80u >> cases[index].bit)) != 0u ? 15u : 0u);
            failed |= x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) != LIB_STATUS_OK ||
                snapshot.buffer_changed;
        }
        lib_memory_copy(saved, &snapshot, sizeof(snapshot));
        {
            const lib_u64 captured = vadp->data.captured_generation;

            ega_planar_crtc_write(vadp, 0x09u, 0u);
            ega_planar_crtc_write(vadp, 0x08u, 1u);
            failed |= x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) != LIB_STATUS_UNSUPPORTED ||
                lib_memory_compare(saved, &snapshot, sizeof(snapshot)) != 0 ||
                vadp->data.captured_generation != captured;
            ega_planar_crtc_write(vadp, 0x08u, 0u);
            (void)video_test_read(vadp, X86_VIDEO_REGISTER_COLOR_STATUS);
            video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 0x33u);
            video_test_write(vadp, X86_VIDEO_REGISTER_ATTRIBUTE, 8u);
            failed |= x86_video_capture_snapshot_from(vadp, LIB_NULL, &snapshot) != LIB_STATUS_UNSUPPORTED ||
                lib_memory_compare(saved, &snapshot, sizeof(snapshot)) != 0 ||
                vadp->data.captured_generation != captured;
        }
        lib_release(saved);
        lib_release(seed);
    }
    failed |= copied_kind != X86_VIDEO_KIND_EGA_320X200X16 ||
        copied_pixel_zero != 15u || copied_pixel_two != 15u ||
        copied_palette_fifteen != 0xaaaaffu;

    x86_video_destroy(vadp);
    if (failed) {
        lib_c_fprintf(lib_c_stderr, "EGA-PLANAR:PORT:FAIL\n");
        return 1;
    }
    lib_c_printf("EGA-PLANAR:PORT:OK\n");
    return 0;
}

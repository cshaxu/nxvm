#include "lib/kvm-window/geometry.h"

#include <assert.h>

static void assert_inside(const kvm_window_rect *inner, const kvm_window_rect *outer)
{
    assert(inner->left >= outer->left);
    assert(inner->top >= outer->top);
    assert(inner->right <= outer->right);
    assert(inner->bottom <= outer->bottom);
}

static void check_stable_fit(void)
{
    static const lib_u32 sizes[][2] = {
        {640,480}, {720,400}, {1920,1080}, {480,640}, {1,4096}, {4096,1}
    };
    for (lib_u32 i = 0; i < sizeof(sizes)/sizeof(sizes[0]); ++i) {
        for (lib_i32 w = 1; w <= 1100; w += 7) {
            for (lib_i32 h = 1; h <= 800; h += 11) {
                lib_i32 fw, fh, again_w, again_h;
                assert(kvm_window_fit_aspect_size(w,h,sizes[i][0],sizes[i][1],&fw,&fh));
                assert(fw > 0 && fw <= w && fh > 0 && fh <= h);
                assert(kvm_window_fit_aspect_size(fw,fh,sizes[i][0],sizes[i][1],&again_w,&again_h));
                assert(again_w == fw && again_h == fh);
            }
        }
    }
    for (lib_i32 edge = KVM_WINDOW_EDGE_LEFT; edge <= KVM_WINDOW_EDGE_BOTTOMRIGHT; ++edge) {
        kvm_window_rect rect = {10,20,1027,810};
        lib_i32 w,h;
        kvm_window_constrain_sizing(&rect,(kvm_window_edge)edge,16,39,640,480);
        assert(kvm_window_fit_aspect_size(rect.right-rect.left-16,
            rect.bottom-rect.top-39,640,480,&w,&h));
        assert(w == rect.right-rect.left-16 && h == rect.bottom-rect.top-39);
    }
    {
        lib_i32 w,h;
        assert(kvm_window_fit_aspect_size(1001,751,640,480,&w,&h));
        assert(w == 1001 && h == 751);
        assert(!kvm_window_fit_aspect_size(0,751,640,480,&w,&h));
        assert(!kvm_window_fit_aspect_size(1001,751,0,480,&w,&h));
    }
}

int main(void)
{
    kvm_window_rect work_area = { 0, 0, 1920, 1080 };
    kvm_window_rect fitted;
    check_stable_fit();

    assert(kvm_window_fit_outer_rect(&work_area, 680, 560, &fitted));
    assert(fitted.right - fitted.left == 680);
    assert(fitted.bottom - fitted.top == 560);
    assert_inside(&fitted, &work_area);

    work_area.right = 600;
    work_area.bottom = 900;
    assert(kvm_window_fit_outer_rect(&work_area, 680, 560, &fitted));
    assert(fitted.right - fitted.left == 600);
    assert(fitted.bottom - fitted.top == 495);
    assert_inside(&fitted, &work_area);

    work_area.left = 100;
    work_area.top = 50;
    work_area.right = 1700;
    work_area.bottom = 450;
    assert(kvm_window_fit_outer_rect(&work_area, 680, 560, &fitted));
    assert(fitted.right - fitted.left == 486);
    assert(fitted.bottom - fitted.top == 400);
    assert_inside(&fitted, &work_area);

    work_area.left = 0;
    work_area.top = 0;
    work_area.right = 600;
    work_area.bottom = 500;
    {
        lib_i32 client_width;
        lib_i32 client_height;
        assert(kvm_window_fit_client_size(&work_area, 16, 39, 640, 480,
            &client_width, &client_height));
        assert(client_width == 584);
        assert(client_height == 438);
        assert(client_width + 16 <= work_area.right - work_area.left);
        assert(client_height + 39 <= work_area.bottom - work_area.top);
    }
    {
        lib_i32 client_width;
        lib_i32 client_height;

        assert(kvm_window_fit_aspect_size(1920, 1041, 640, 480,
            &client_width, &client_height));
        assert(client_width == 1388);
        assert(client_height == 1041);
        assert(client_width <= 1920);
        assert(client_height <= 1041);

        assert(kvm_window_fit_aspect_size(600, 900, 640, 480,
            &client_width, &client_height));
        assert(client_width == 600);
        assert(client_height == 450);
        assert(client_width <= 600);
        assert(client_height <= 900);
    }
    return 0;
}

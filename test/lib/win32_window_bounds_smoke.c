#include "lib/kvm-window/geometry.h"

#include <assert.h>

static void assert_inside(const kvm_window_rect *inner, const kvm_window_rect *outer)
{
    assert(inner->left >= outer->left);
    assert(inner->top >= outer->top);
    assert(inner->right <= outer->right);
    assert(inner->bottom <= outer->bottom);
}

int main(void)
{
    kvm_window_rect work_area = { 0, 0, 1920, 1080 };
    kvm_window_rect fitted;

    assert(kvm_window_fit_outer_rect(&work_area, 680, 560, &fitted));
    assert(fitted.right - fitted.left == 680);
    assert(fitted.bottom - fitted.top == 560);
    assert_inside(&fitted, &work_area);

    work_area.right = 600;
    work_area.bottom = 900;
    assert(kvm_window_fit_outer_rect(&work_area, 680, 560, &fitted));
    assert(fitted.right - fitted.left == 600);
    assert(fitted.bottom - fitted.top == 494);
    assert_inside(&fitted, &work_area);

    work_area.left = 100;
    work_area.top = 50;
    work_area.right = 1700;
    work_area.bottom = 450;
    assert(kvm_window_fit_outer_rect(&work_area, 680, 560, &fitted));
    assert(fitted.right - fitted.left == 485);
    assert(fitted.bottom - fitted.top == 400);
    assert_inside(&fitted, &work_area);

    work_area.left = 0;
    work_area.top = 0;
    work_area.right = 600;
    work_area.bottom = 500;
    {
        int client_width;
        int client_height;
        assert(kvm_window_fit_client_size(&work_area, 16, 39, 640, 480,
            &client_width, &client_height));
        assert(client_width == 584);
        assert(client_height == 438);
        assert(client_width + 16 <= work_area.right - work_area.left);
        assert(client_height + 39 <= work_area.bottom - work_area.top);
    }
    {
        int client_width;
        int client_height;

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

#include "core/product/wait.h"
#include "core/product/wait_provider.h"

static core_product_wait_provider coreProductWaitProvider;
static void *coreProductWaitContext;

void core_product_wait_bind(core_product_wait_provider provider, void *context)
{
    coreProductWaitProvider = provider;
    coreProductWaitContext = context;
}

void core_product_wait_milliseconds(uint32_t milliseconds)
{
    if (coreProductWaitProvider != NULL) {
        coreProductWaitProvider(coreProductWaitContext, milliseconds);
    }
}

void utilsSleep(uint32_t milisec)
{
    core_product_wait_milliseconds(milisec);
}

#include <stdio.h>

#define BLAWORK_IMPL_WINAPI
#define BLAWORK_IMPLEMENTATION
#include "blawork.h"

#ifndef BLAWORK_IMPL_WINAPI
#ifdef __cplusplus
extern "C"
#endif
unsigned Sleep(unsigned ms);
#endif

static void doit(void * arg)
{
    Sleep(1000);
    printf("%p\n", arg);
}

#define hehe 5

int main(void)
{
    int i;
    blawork_t works[hehe];

    printf("threading: %s\n", blawork_implementation_name());
    for(i = 0; i < hehe; ++i)
    {
        blawork_begin(&works[i], doit, NULL);
    }

    for(i = 0; i < hehe; ++i)
    {
        blawork_end(&works[i]);
    }

    return 0;
}


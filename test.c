#include <stdio.h>

#define BLAWORK_IMPL_NULL
//#define BLAWORK_IMPL_C11
//#define BLAWORK_IMPL_WINAPI
//#define BLAWORK_IMPL_POSIX
//#define BLAWORK_IMPL_CPP11
#define BLAWORK_IMPLEMENTATION
#include "blawork.h"

#if 1
#include <Windows.h> /* for Sleep */
#define mysleep(x) Sleep(x * 1000)
#else
#include <unistd.h> /* for sleep */
#define mysleep(x) sleep(x)
#endif

static void doit(void * arg)
{
    mysleep(1);
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


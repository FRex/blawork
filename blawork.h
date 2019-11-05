#ifndef BLAWORK_H
#define BLAWORK_H
typedef void(*blawork_func_t)(void*);

typedef struct blawork {
    blawork_func_t func;
    void * arg;
    int running;
    void * impl;
} blawork_t;

void blawork_begin(blawork_t * work, blawork_func_t func, void * arg);
void blawork_end(blawork_t * work);
const char * blawork_implementation_name(void);
#endif /* BLAWORK_H */



#ifdef BLAWORK_IMPLEMENTATION
#include <stdlib.h>

#if (defined(BLAWORK_IMPL_NULL) + defined(BLAWORK_IMPL_CPP11) + defined(BLAWORK_IMPL_C11) + defined(BLAWORK_IMPL_WINAPI) + defined(BLAWORK_IMPL_POSIX)) == 0
#define BLAWORK_IMPL_NULL
#endif

#if (defined(BLAWORK_IMPL_NULL) + defined(BLAWORK_IMPL_CPP11) + defined(BLAWORK_IMPL_C11) + defined(BLAWORK_IMPL_WINAPI) + defined(BLAWORK_IMPL_POSIX)) > 1
#error "more than one implementation requested"
#endif



#ifdef BLAWORK_IMPL_NULL
typedef struct blawork_impl blawork_impl_t;

static blawork_impl_t * blawork_impl_create(blawork_func_t func, void * arg)
{
    return NULL;
}

static int blawork_impl_join(blawork_impl_t * impl)
{
    return 0;
}
#define BLAWORK_IMPL_NAME_STRING "NONE"
#endif /* BLAWORK_IMPL_NULL */



#ifdef BLAWORK_IMPL_CPP11
#include <thread>
typedef std::thread blawork_impl_t;

static blawork_impl_t * blawork_impl_create(blawork_func_t func, void * arg)
{
    try {
        return new std::thread(func, arg);
    }
    catch(const std::exception& e) {
        return nullptr;
    }
}

static int blawork_impl_join(blawork_impl_t * impl)
{
    try {
        if(impl->joinable())
            impl->join();

        delete impl;
        return 1;
    }
    catch(const std::exception& e) {
        return 0;
    }
}

#define BLAWORK_IMPL_NAME_STRING "C++11"
#endif /* BLAWORK_IMPL_CPP11 */



#ifdef BLAWORK_IMPL_C11
#include <threads.h>
typedef thrd_t blawork_thread_t;
#define BLAWORK_HELPER_RET int
#define BLAWORK_HELPER_RET_VALUE 0
#define BLAWORK_CREATE_THREAD(ret, f) (thrd_create(&ret->thread, f, ret) == thrd_success)
#define BLAWORK_JOIN_THREAD(thread) thrd_join(thread, NULL);
#define BLAWORK_IMPL_NAME_STRING "C11"
#endif /* BLAWORK_IMPL_C11 */



#ifdef BLAWORK_IMPL_WINAPI
#include <Windows.h>
typedef HANDLE blawork_thread_t;
#define BLAWORK_HELPER_RET DWORD WINAPI
#define BLAWORK_HELPER_RET_VALUE 0u
#define BLAWORK_CREATE_THREAD(ret, f) ((ret->thread = CreateThread(NULL, 0u, f, ret, 0u, NULL)) != NULL)
#define BLAWORK_JOIN_THREAD(thread) WaitForSingleObject(thread, INFINITE);
#define BLAWORK_IMPL_NAME_STRING "WINAPI"
#endif /* BLAWORK_IMPL_WINAPI */



#ifdef BLAWORK_IMPL_POSIX
#include <pthread.h>
typedef pthread_t blawork_thread_t;
#define BLAWORK_HELPER_RET void *
#define BLAWORK_HELPER_RET_VALUE NULL
#define BLAWORK_CREATE_THREAD(ret, f) (pthread_create(&ret->thread, NULL, f, ret) == 0)
#define BLAWORK_JOIN_THREAD(thread) pthread_join(thread, NULL);
#define BLAWORK_IMPL_NAME_STRING "POSIX"
#endif /* BLAWORK_IMPL_POSIX */



#if (defined(BLAWORK_IMPL_C11) || defined(BLAWORK_IMPL_WINAPI) || defined(BLAWORK_IMPL_POSIX))

typedef struct blawork_impl {
    blawork_thread_t thread;
    blawork_func_t func;
    void * arg;
    int threaddone;
} blawork_impl_t;

static BLAWORK_HELPER_RET blawork_impl_helper(void * impl)
{
    blawork_impl_t * i = (blawork_impl_t*)impl;
    i->func(i->arg);
    i->threaddone = 1;
    return BLAWORK_HELPER_RET_VALUE;
}

static blawork_impl_t * blawork_impl_create(blawork_func_t func, void * arg)
{
    blawork_impl_t * ret = (blawork_impl_t *)malloc(sizeof(blawork_impl_t));
    if(!ret)
        return NULL;

    ret->threaddone = 0;
    ret->func = func;
    ret->arg = arg;
    if(!BLAWORK_CREATE_THREAD(ret, blawork_impl_helper))
    {
        free(ret);
        return NULL;
    }

    return ret;
}

static int blawork_impl_join(blawork_impl_t * impl)
{
    int ret;
    if(!impl)
        return 0;

    BLAWORK_JOIN_THREAD(impl->thread);
    ret = impl->threaddone;
    free(impl);
    return ret;
}
#endif /* (defined(BLAWORK_IMPL_C11) || defined(BLAWORK_IMPL_WINAPI) || defined(BLAWORK_IMPL_POSIX)) */



void blawork_begin(blawork_t * work, blawork_func_t func, void * arg)
{
    if(!work)
        return;

    work->func = func;
    work->arg = arg;
    work->impl = blawork_impl_create(func, arg);
    work->running = 1;
}

void blawork_end(blawork_t * work)
{
    if(!work || !work->func || !work->running)
        return;

    if(!blawork_impl_join((blawork_impl_t*)work->impl))
        work->func(work->arg);

    work->running = 0;
}

const char * blawork_implementation_name(void)
{
    return BLAWORK_IMPL_NAME_STRING;
}

#endif /* BLAWORK_IMPLEMENTATION */

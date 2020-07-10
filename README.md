# blawork

Simple API wrapper around 4 different threading APIs with a fallback 100%
portable C implementation that does things on same thread transparently.

There is one struct (`blawork_t`) and three functions (`blawork_begin`,
`blawork_end`, `blawork_implementation_name`). `blawork_implementation_name`
simply returns name of currently internally used threading API.

The struct is safe to memset to zero (which will make `blawork_end` a no-op)
and copy (via `realloc` or in `std::vector`) at any time (including between a
call to `blawork_begin` and the corresponding call to `blawork_end`, the thread
implementation handle is kept in a pointer field of that struct).

For each call of `blawork_begin` there must be one or more calls to
`blawork_end` (on a given `blawork_t` calls after the first one are no-ops).

Calling `blawork_end` on an uninitialized `blawork_t` or on two copies of the
same instance of `blawork_t` filled by `blawork_begin` might result in a crash.

Calling `blawork_begin` twice on the same instance of `blawork_t` without a
call to `blawork_end` or not calling `blawork_end` at all results in a
small memory and thread handle leak.

Reusing the `blawork_t` as long as calls to `blawork_begin` and `blawork_end`
are paired to each other properly is completely fine.

The API is as simple as possible and meant only for clearly independent worker
functions: a single function that returns nothing and takes one void pointer as
argument. There are no mutexes, no thread local storage, no error codes to
check (by the point `blawork_end` returnes the work has been completed one way
or another) and errors in API calls are not reported. If it fails to create the
thread or if it's running with `BLAWORK_IMPL_NULL` then the function will be
called in `blawork_end` (sort of as if it were a single core CPU). In a very
narrow specific case (thread created and did work but then failed to join
properly) it might call the function twice, once in the thread and then again
in `blawork_end`. Just in case you might want to make your function a no-op on
repeated calls by setting some flag in the data pointed to by your pointer.

Take care to ensure to not throw out of your worker function (if using C++) and
to make sure data pointed to by the pointer you provide as argument won't be
freed before `blawork_end` is called.

To use it, include the header as normal and define `BLAWORK_IMPLEMENTATION` in
a single translation unit. Beware: the headers included in some implementations
are very heavy and noisy (e.g. `Windows.h` on `BLAWORK_IMPL_WINAPI`).

It used to default to `BLAWORK_IMPL_NULL` which is plain C version that does all
the work in `blawork_end` and makes no extra allocations or API calls and thus
cannot ever fail and will always complete the work (but on the same thread), but
doesn't anymore to avoid erorrs (running with plain C single threaded version by
mistake) so one of `BLAWORK_IMPL_` macros (including `BLAWORK_IMPL_NULL`) must
be defined in the same file as `BLAWORK_IMPLEMENTATION` was defined in.

To eanble one of the threading APIs to be used internally define one of:
`BLAWORK_IMPL_CPP11`, `BLAWORK_IMPL_C11`, `BLAWORK_IMPL_WINAPI`, `BLAWORK_IMPL_POSIX`.

See `test.c` for simple example usage.

The idea for this lib came to me when improving on my other program
[pixelsum](https://github.com/FRex/pixelsum) which is very easy to parallelize
with one thread per file given. See its code for antother example usage.

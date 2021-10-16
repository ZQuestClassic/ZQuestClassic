#ifndef _ZC_MUTEX_H_
#define _ZC_MUTEX_H_

// pthread functions return a value, Windows equivalents don't.
// Shouldn't be an issue at the moment.

#ifdef _WIN32

#include <windows.h>

typedef CRITICAL_SECTION mutex;

inline void mutex_init(CRITICAL_SECTION* m)
{
    return InitializeCriticalSection(m);
}

inline void mutex_destroy(CRITICAL_SECTION* m)
{
    return DeleteCriticalSection(m);
}

inline void mutex_lock(CRITICAL_SECTION* m)
{
    return EnterCriticalSection(m);
}

inline void mutex_unlock(CRITICAL_SECTION* m)
{
    return LeaveCriticalSection(m);
}

#else // Non-Windows

#include <pthread.h>

typedef pthread_mutex_t mutex;

inline int32_t mutex_init(pthread_mutex_t* m)
{
    return pthread_mutex_init(m, 0);
}

inline int32_t mutex_destroy(pthread_mutex_t* m)
{
    return pthread_mutex_destroy(m);
}

inline int32_t mutex_lock(pthread_mutex_t* m)
{
    return pthread_mutex_lock(m);
}

inline int32_t mutex_unlock(pthread_mutex_t* m)
{
    return pthread_mutex_unlock(m);
}

#endif

#endif

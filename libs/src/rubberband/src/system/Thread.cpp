/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Rubber Band Library
    An audio time-stretching and pitch-shifting library.
    Copyright 2007-2014 Particular Programs Ltd.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.

    Alternatively, if you have a valid commercial licence for the
    Rubber Band Library obtained by agreement with the copyright
    holders, you may redistribute and/or modify it under the terms
    described in that licence.

    If you wish to distribute code using the Rubber Band Library
    under terms other than those of the GNU General Public License,
    you must obtain a valid commercial licence before doing so.
*/

#include "Thread.h"

#include <iostream>
#include <cstdlib>

#ifdef USE_PTHREADS
#include <sys/time.h>
#include <time.h>
#endif

using std::cerr;
using std::endl;
using std::string;

namespace RubberBand
{

#ifdef _WIN32

Thread::Thread() :
    m_id(0),
    m_extant(false)
{
}

Thread::~Thread()
{
    if (m_extant) {
        WaitForSingleObject(m_id, INFINITE);
    }
}

void
Thread::start()
{
    m_id = CreateThread(NULL, 0, staticRun, this, 0, 0);
    if (!m_id) {
        cerr << "ERROR: thread creation failed" << endl;
        exit(1);
    } else {
        m_extant = true;
    }
}

void
Thread::wait()
{
    if (m_extant) {
        WaitForSingleObject(m_id, INFINITE);
        m_extant = false;
    }
}

Thread::Id
Thread::id()
{
    return m_id;
}

bool
Thread::threadingAvailable()
{
    return true;
}

DWORD
Thread::staticRun(LPVOID arg)
{
    Thread *thread = static_cast<Thread *>(arg);
    thread->run();
    return 0;
}

Mutex::Mutex()
{
    m_mutex = CreateMutex(NULL, FALSE, NULL);
}

Mutex::~Mutex()
{
    CloseHandle(m_mutex);
}

void
Mutex::lock()
{
    WaitForSingleObject(m_mutex, INFINITE);
}

void
Mutex::unlock()
{
    ReleaseMutex(m_mutex);
}

bool
Mutex::trylock()
{
    DWORD result = WaitForSingleObject(m_mutex, 0);
    if (result == WAIT_TIMEOUT || result == WAIT_FAILED) {
        return false;
    } else {
        return true;
    }
}

Condition::Condition(string name) :
    m_locked(false)
{
    m_mutex = CreateMutex(NULL, FALSE, NULL);
    m_condition = CreateEvent(NULL, FALSE, FALSE, NULL);
}

Condition::~Condition()
{
    if (m_locked) ReleaseMutex(m_mutex);
    CloseHandle(m_condition);
    CloseHandle(m_mutex);
}

void
Condition::lock()
{
    WaitForSingleObject(m_mutex, INFINITE);
    m_locked = true;
}

void
Condition::unlock()
{
    if (!m_locked) {
        return;
    }
    m_locked = false;
    ReleaseMutex(m_mutex);
}

void
Condition::wait(int us)
{
    if (us == 0) {
        SignalObjectAndWait(m_mutex, m_condition, INFINITE, FALSE);
        WaitForSingleObject(m_mutex, INFINITE);

    } else {

        DWORD ms = us / 1000;
        if (us > 0 && ms == 0) ms = 1;

        SignalObjectAndWait(m_mutex, m_condition, ms, FALSE);
        WaitForSingleObject(m_mutex, INFINITE);
    }

    m_locked = true;
}

void
Condition::signal()
{
    SetEvent(m_condition);
}

#else /* !_WIN32 */

#ifdef USE_PTHREADS

Thread::Thread() :
    m_id(0),
    m_extant(false)
{
}

Thread::~Thread()
{
    if (m_extant) {
        pthread_join(m_id, 0);
    }
}

void
Thread::start()
{
    if (pthread_create(&m_id, 0, staticRun, this)) {
        cerr << "ERROR: thread creation failed" << endl;
        exit(1);
    } else {
        m_extant = true;
    }
}

void
Thread::wait()
{
    if (m_extant) {
        pthread_join(m_id, 0);
        m_extant = false;
    }
}

Thread::Id
Thread::id()
{
    return m_id;
}

bool
Thread::threadingAvailable()
{
    return true;
}

void *
Thread::staticRun(void *arg)
{
    Thread *thread = static_cast<Thread *>(arg);
    thread->run();
    return 0;
}

Mutex::Mutex()
{
    pthread_mutex_init(&m_mutex, 0);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_mutex);
}

void
Mutex::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void
Mutex::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

bool
Mutex::trylock()
{
    if (pthread_mutex_trylock(&m_mutex)) {
        return false;
    } else {
        return true;
    }
}

Condition::Condition(string name) :
    m_locked(false)
{
    pthread_mutex_init(&m_mutex, 0);
    pthread_cond_init(&m_condition, 0);
}

Condition::~Condition()
{
    if (m_locked) pthread_mutex_unlock(&m_mutex);
    pthread_cond_destroy(&m_condition);
    pthread_mutex_destroy(&m_mutex);
}

void
Condition::lock()
{
    pthread_mutex_lock(&m_mutex);
    m_locked = true;
}

void
Condition::unlock()
{
    if (!m_locked) {
        return;
    }
    m_locked = false;
    pthread_mutex_unlock(&m_mutex);
}

void
Condition::wait(int us)
{
    if (us == 0) {
        pthread_cond_wait(&m_condition, &m_mutex);
    } else {
        struct timeval now;
        gettimeofday(&now, 0);

        now.tv_usec += us;
        while (now.tv_usec > 1000000) {
            now.tv_usec -= 1000000;
            ++now.tv_sec;
        }

        struct timespec timeout;
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = now.tv_usec * 1000;

        pthread_cond_timedwait(&m_condition, &m_mutex, &timeout);
    }

    m_locked = true;
}

void
Condition::signal()
{
    pthread_cond_signal(&m_condition);
}

#else /* !USE_PTHREADS */

Thread::Thread()
{
}

Thread::~Thread()
{
}

void
Thread::start()
{
    abort();
}

void
Thread::wait()
{
    abort();
}

Thread::Id
Thread::id()
{
    abort();
}

bool
Thread::threadingAvailable()
{
    return false;
}

Mutex::Mutex()
{
}

Mutex::~Mutex()
{
}

void
Mutex::lock()
{
    abort();
}

void
Mutex::unlock()
{
    abort();
}

bool
Mutex::trylock()
{
    abort();
}

Condition::Condition(const char *)
{
}

Condition::~Condition()
{
}

void
Condition::lock()
{
    abort();
}

void
Condition::wait(int us)
{
    abort();
}

void
Condition::signal()
{
    abort();
}

#endif /* !USE_PTHREADS */
#endif /* !_WIN32 */

MutexLocker::MutexLocker(Mutex *mutex) :
    m_mutex(mutex)
{
    if (m_mutex) {
        m_mutex->lock();
    }
}

MutexLocker::~MutexLocker()
{
    if (m_mutex) {
        m_mutex->unlock();
    }
}

}

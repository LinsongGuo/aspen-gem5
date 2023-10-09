#include <base/time.h>
#include <pthread.h>
#include <runtime/sync.h>
#include <sys/time.h>

#include <stdio.h>

#include "common.h"

#define INIT_MAGIC 0xDEADBEEF

struct condvar_with_attr {
	condvar_t cv; // condvar must be first
	clockid_t clockid;
	uint32_t magic;
};

struct mutex_with_attr {
	mutex_t mutex; // must be first
	uint32_t magic;
};

struct rwmutex_with_attr {
	rwmutex_t rwmutex; // must be first
	uint32_t magic;
};

BUILD_ASSERT(sizeof(pthread_barrier_t) >= sizeof(barrier_t));
BUILD_ASSERT(sizeof(pthread_mutex_t) >= sizeof(struct mutex_with_attr));
BUILD_ASSERT(sizeof(pthread_spinlock_t) >= sizeof(spinlock_t));
BUILD_ASSERT(sizeof(pthread_cond_t) >= sizeof(struct condvar_with_attr));
BUILD_ASSERT(sizeof(pthread_rwlock_t) >= sizeof(struct rwmutex_with_attr));

static inline void mutex_intialized_check(pthread_mutex_t *mutex) {
	struct mutex_with_attr *m = (struct mutex_with_attr *)mutex;
	if (unlikely(m->magic != INIT_MAGIC))
		pthread_mutex_init(mutex, NULL);
}

static inline void cond_intialized_check(pthread_cond_t *cond) {
	struct condvar_with_attr *cvattr = (struct condvar_with_attr *)cond;
	if (unlikely(cvattr->magic != INIT_MAGIC))
		pthread_cond_init(cond, NULL);
}

static inline void rwmutex_intialized_check(pthread_rwlock_t *r) {
	struct rwmutex_with_attr *rwattr = (struct rwmutex_with_attr *)r;
	if (unlikely(rwattr->magic != INIT_MAGIC))
		pthread_rwlock_init(r, NULL);
}


int pthread_mutex_init(pthread_mutex_t *mutex,
		       const pthread_mutexattr_t *mutexattr)
{
	NOTSELF(pthread_mutex_init, mutex, mutexattr);

#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif

	struct mutex_with_attr *m = (struct mutex_with_attr *)mutex;
	mutex_init(&m->mutex);
	m->magic = INIT_MAGIC;

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
	NOTSELF(pthread_mutex_lock, mutex);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // unsigned int old = preempt_get_disable();
#endif
	
	// printf("pthread_mutex_lock: %p\n", mutex);
	mutex_intialized_check(mutex);
	mutex_lock((mutex_t *)mutex);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_set(old);
#endif
	return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	NOTSELF(pthread_mutex_trylock, mutex);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif

	mutex_intialized_check(mutex);
	int res = mutex_try_lock((mutex_t *)mutex) ? 0 : EBUSY;

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return res;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	NOTSELF(pthread_mutex_unlock, mutex);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif
	
	mutex_unlock((mutex_t *)mutex);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	// printf("NOTSELF(pthread_mutex_destroy) \n");
	NOTSELF(pthread_mutex_destroy, mutex);

#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
   //  preempt_disable();
#endif
	
	// printf("SELF(pthread_mutex_destroy) \n");
	NOTSELF(pthread_mutex_destroy, mutex);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_barrier_init(pthread_barrier_t *restrict barrier,
			 const pthread_barrierattr_t *restrict attr,
			 unsigned count)
{
	NOTSELF(pthread_barrier_init, barrier, attr, count);

	barrier_init((barrier_t *)barrier, count);

	return 0;
}

int pthread_barrier_wait(pthread_barrier_t *barrier)
{
	NOTSELF(pthread_barrier_wait, barrier);

	if (barrier_wait((barrier_t *)barrier))
		return PTHREAD_BARRIER_SERIAL_THREAD;

	return 0;
}

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
	NOTSELF(pthread_barrier_destroy, barrier);
	return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *lock)
{
	NOTSELF(pthread_spin_destroy, lock);
	return 0;
}

int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
	NOTSELF(pthread_spin_init, lock, pshared);
	spin_lock_init((spinlock_t *)lock);
	return 0;
}

int pthread_spin_lock(pthread_spinlock_t *lock)
{
	NOTSELF(pthread_spin_lock, lock);
	spin_lock_np((spinlock_t *)lock);
	return 0;
}

int pthread_spin_trylock(pthread_spinlock_t *lock)
{
	NOTSELF(pthread_spin_trylock, lock);
	return spin_try_lock_np((spinlock_t *)lock) ? 0 : EBUSY;
}

int pthread_spin_unlock(pthread_spinlock_t *lock)
{
	NOTSELF(pthread_spin_unlock, lock);
	spin_unlock_np((spinlock_t *)lock);
	return 0;
}

int pthread_cond_init(pthread_cond_t *__restrict cond,
		      const pthread_condattr_t *__restrict cond_attr)
{
	NOTSELF(pthread_cond_init, cond, cond_attr);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif
		
	struct condvar_with_attr *cvattr = (struct condvar_with_attr *)cond;
	condvar_init(&cvattr->cv);
	cvattr->magic = INIT_MAGIC;

	if (!cond_attr ||
	    pthread_condattr_getclock(cond_attr, &cvattr->clockid)) {
		cvattr->clockid = CLOCK_REALTIME;
	}

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
	NOTSELF(pthread_cond_signal, cond);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif
		
	cond_intialized_check(cond);
	condvar_signal((condvar_t *)cond);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_cond_broadcast(pthread_cond_t *cond)
{
	NOTSELF(pthread_cond_broadcast, cond);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif
		
	cond_intialized_check(cond);
	condvar_broadcast((condvar_t *)cond);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
	NOTSELF(pthread_cond_wait, cond, mutex);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif
		
	cond_intialized_check(cond);
	condvar_wait((condvar_t *)cond, (mutex_t *)mutex);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex,
			   const struct timespec *abstime)
{
	NOTSELF(pthread_cond_timedwait, cond, mutex, abstime);

#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif
		
	bool done;
	uint64_t wait_us, now_us;
	struct condvar_with_attr *cvattr = (struct condvar_with_attr *)cond;
	struct timespec now_ts;

	cond_intialized_check(cond);

	BUG_ON(clock_gettime(cvattr->clockid, &now_ts));
	wait_us = abstime->tv_sec * ONE_SECOND + abstime->tv_nsec / 1000;
	now_us = now_ts.tv_sec * ONE_SECOND + now_ts.tv_nsec / 1000;

	if (wait_us <= now_us)
		return ETIMEDOUT;

	done = condvar_wait_timed((condvar_t *)cond, (mutex_t *)mutex, wait_us - now_us);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return done ? 0 : ETIMEDOUT;
}

int pthread_cond_destroy(pthread_cond_t *cond)
{
	NOTSELF(pthread_cond_destroy, cond);

#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif

	NOTSELF(pthread_cond_destroy, cond);	
	
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t *r)
{
	NOTSELF(pthread_rwlock_destroy, r);

#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif
		
	NOTSELF(pthread_rwlock_destroy, r);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_rwlock_init(pthread_rwlock_t *r, const pthread_rwlockattr_t *attr)
{
	NOTSELF(pthread_rwlock_init, r, attr);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif
		
	struct rwmutex_with_attr *rwattr = (struct rwmutex_with_attr *)r;
	rwattr->magic = INIT_MAGIC;
	rwmutex_init(&rwattr->rwmutex);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *r)
{
	NOTSELF(pthread_rwlock_rdlock, r);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // unsigned int old = preempt_get_disable();
#endif
		
	rwmutex_intialized_check(r);
	rwmutex_rdlock((rwmutex_t *)r);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_set(old);
#endif
	return 0;
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *r)
{
	NOTSELF(pthread_rwlock_tryrdlock, r);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif

	rwmutex_intialized_check(r);
	int res = rwmutex_try_rdlock((rwmutex_t *)r) ? 0 : EBUSY;

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return res;
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *r)
{
	NOTSELF(pthread_rwlock_trywrlock, r);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif

	rwmutex_intialized_check(r);
	int res = rwmutex_try_wrlock((rwmutex_t *)r) ? 0 : EBUSY;

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return res;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *r)
{
	NOTSELF(pthread_rwlock_wrlock, r);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // unsigned int old = preempt_get_disable();
#endif
		
	rwmutex_intialized_check(r);
	rwmutex_wrlock((rwmutex_t *)r);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_set(old);
#endif
	return 0;
}

int pthread_rwlock_unlock(pthread_rwlock_t *r)
{
	NOTSELF(pthread_rwlock_unlock, r);
	
#if defined(UNSAFE_PREEMPT_CLUI)
    unsigned char uif = _testui();
    if (likely(uif))
        _clui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_disable();
#endif
		
	rwmutex_unlock((rwmutex_t *)r);

#if defined(UNSAFE_PREEMPT_CLUI)
    if (likely(uif))
        _stui();
#elif defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // preempt_enable();
#endif
	return 0;
}

/*
 * init.c - initializes the runtime
 */

#include <pthread.h>

#include <base/cpu.h>
#include <base/init.h>
#include <base/log.h>
#include <base/limits.h>
#include <runtime/thread.h>

#include "defs.h"

#include <m5_mmap.h>

static pthread_barrier_t init_barrier;
pthread_t kth_tid[NCPU];

struct init_entry {
	const char *name;
	int (*init)(void);
};

static initializer_fn_t global_init_hook = NULL;
static initializer_fn_t perthread_init_hook = NULL;
static initializer_fn_t late_init_hook = NULL;


#define GLOBAL_INITIALIZER(name) \
	{__cstr(name), &name ## _init}

/* global subsystem initialization */
static const struct init_entry global_init_handlers[] = {
	/* runtime core */
	GLOBAL_INITIALIZER(kthread),
	GLOBAL_INITIALIZER(ioqueues),
	GLOBAL_INITIALIZER(stack),
	GLOBAL_INITIALIZER(sched),
	GLOBAL_INITIALIZER(preempt),
	GLOBAL_INITIALIZER(smalloc),
	GLOBAL_INITIALIZER(uintr),

	/* network stack */
	GLOBAL_INITIALIZER(net),
	GLOBAL_INITIALIZER(udp),
	GLOBAL_INITIALIZER(directpath),
	GLOBAL_INITIALIZER(arp),
	GLOBAL_INITIALIZER(trans),

	/* storage */
	GLOBAL_INITIALIZER(storage),

#ifdef GC
	GLOBAL_INITIALIZER(gc),
#endif
};

#define THREAD_INITIALIZER(name) \
	{__cstr(name), &name ## _init_thread}

/* per-kthread subsystem initialization */
static const struct init_entry thread_init_handlers[] = {
	/* runtime core */
	THREAD_INITIALIZER(preempt),
	THREAD_INITIALIZER(kthread),
	THREAD_INITIALIZER(ioqueues),
	THREAD_INITIALIZER(stack),
	THREAD_INITIALIZER(sched),
	THREAD_INITIALIZER(timer),
	THREAD_INITIALIZER(smalloc),
	// THREAD_INITIALIZER(uintr),

	/* network stack */
	THREAD_INITIALIZER(net),
	THREAD_INITIALIZER(directpath),

	/* storage */
	THREAD_INITIALIZER(storage),
};

#define LATE_INITIALIZER(name) \
	{__cstr(name), &name ## _init_late}

static const struct init_entry late_init_handlers[] = {
	/* network stack */
	LATE_INITIALIZER(net),
	LATE_INITIALIZER(arp),
	LATE_INITIALIZER(stat),
	LATE_INITIALIZER(tcp),
	LATE_INITIALIZER(rcu),
	LATE_INITIALIZER(directpath),

	/* runtime core */
	// LATE_INITIALIZER(uintr),
};

static int run_init_handlers(const char *phase,
			     const struct init_entry *h, int nr)
{
	int i, ret;

	log_debug("entering '%s' init phase", phase);
	for (i = 0; i < nr; i++) {
		log_debug("init -> %s", h[i].name);
		ret = h[i].init();
		if (ret) {
			log_debug("failed, ret = %d", ret);
			return ret;
		}
	}

	return 0;
}

static int runtime_init_thread(void)
{
	int ret;

	ret = base_init_thread();
	if (ret) {
		log_err("base library per-thread init failed, ret = %d", ret);
		return ret;
	}

	ret = run_init_handlers("per-thread", thread_init_handlers,
				 ARRAY_SIZE(thread_init_handlers));
	if (ret || perthread_init_hook == NULL)
		return ret;

	return perthread_init_hook();

}

static void *pthread_entry(void *data)
{
	int ret;

	ret = runtime_init_thread();
	BUG_ON(ret);

	pthread_barrier_wait(&init_barrier);
	pthread_barrier_wait(&init_barrier);

	pthread_barrier_wait(&uintr_init_barrier);
	if (!is_load_generator && uthread_quantum_us < 100000000) {
		uintr_init_thread();
	}
	pthread_barrier_wait(&uintr_init_barrier);

	sched_start();

	/* never reached unless things are broken */
	BUG();
	return NULL;
}

/**
 * runtime_set_initializers - allow runtime to specifcy a function to run in
 * each stage of intialization (called before runtime_init).
 */
int runtime_set_initializers(initializer_fn_t global_fn,
			     initializer_fn_t perthread_fn,
			     initializer_fn_t late_fn)
{
	global_init_hook = global_fn;
	perthread_init_hook = perthread_fn;
	late_init_hook = late_fn;
	return 0;
}

/**
 * runtime_init - starts the runtime
 * @cfgpath: the path to the configuration file
 * @main_fn: the first function to run as a thread
 * @arg: an argument to @main_fn
 *
 * Does not return if successful, otherwise return  < 0 if an error.
 */
int runtime_init(const char *cfgpath, thread_fn_t main_fn, void *arg)
{
	m5op_addr = 0xFFFF0000;
    map_m5_mem();

	int ret, i;

	ret = ioqueues_init_early();
	if (unlikely(ret))
		return ret;

	cycles_per_us = iok.iok_info->cycles_per_us;

	ret = base_init();
	if (ret) {
		log_err("base library global init failed, ret = %d", ret);
		return ret;
	}

	ret = cfg_load(cfgpath);
	if (ret)
		return ret;

	log_info("process pid: %u", getpid());

	pthread_barrier_init(&init_barrier, NULL, maxks);
	pthread_barrier_init(&uintr_init_barrier, NULL, maxks);

	ret = run_init_handlers("global", global_init_handlers,
				ARRAY_SIZE(global_init_handlers));
	if (ret)
		return ret;

	if (global_init_hook) {
		ret = global_init_hook();
		if (ret) {
			log_err("User-specificed global initializer failed, ret = %d", ret);
			return ret;
		}
	}

	ret = runtime_init_thread();
	BUG_ON(ret);

	log_info("spawning %d kthreads", maxks);
	for (i = 1; i < maxks; i++) {
		ret = pthread_create(&kth_tid[i], NULL, pthread_entry, NULL);
		BUG_ON(ret);
	}
	kth_tid[0] = pthread_self();

	pthread_barrier_wait(&init_barrier);

	ret = ioqueues_register_iokernel();
	if (ret) {
		log_err("couldn't register with iokernel, ret = %d", ret);
		return ret;
	}

	pthread_barrier_wait(&init_barrier);

	/* point of no return starts here */

	ret = thread_spawn_main(main_fn, arg);
	BUG_ON(ret);

	ret = run_init_handlers("late", late_init_handlers,
				ARRAY_SIZE(late_init_handlers));
	BUG_ON(ret);

	if (!is_load_generator && uthread_quantum_us < 100000000) {
#ifndef M5_UTIMER
		pthread_barrier_init(&uintr_timer_barrier, NULL, 2);
		uintr_init_early();
		pthread_barrier_wait(&uintr_timer_barrier);
#endif
	}

	log_info("********** is_load_generator: %d", is_load_generator);
	if (!is_load_generator) {
		log_info("sleep(5) starts and then switch to gem5.");
		sleep(5);

		// log_info("========== switching ======");
		long long switch_end_tsc = rdtsc() + 8500000;
		m5_switch_cpu_addr();
		while(rdtsc() < switch_end_tsc);
	}

	pthread_barrier_wait(&uintr_init_barrier);

	if (!is_load_generator && uthread_quantum_us < 100000000) {
		uintr_init_thread();
#ifndef M5_UTIMER
		uintr_init_late();
#endif
	}

	pthread_barrier_wait(&uintr_init_barrier);
	uintr_timer_start_internal();

	if (late_init_hook) {
		ret = late_init_hook();
		if (ret) {
			log_err("User-specificed late initializer failed, ret = %d", ret);
			return ret;
		}
	}

	sched_start();

	/* never reached unless things are broken */
	BUG();
	return 0;
}

extern "C" {
#include "loop.h"
}

#include <chrono>
#include <iostream>
#include <sstream>

#include "uintr.h"
#include "runtime.h"
#include "sync.h"
#include "thread.h"
#include "timer.h"

barrier_t barrier;
int task_num = 24;

void MainHandler(void *arg) {	// printf("enter handler\n");
	rt::WaitGroup wg(1);
	barrier_init(&barrier, 1);

	rt::UintrTimerStart();
	
	for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
			spinloop();
    	});
	}
	
	wg.Wait();

	rt::UintrTimerEnd();
	rt::UintrTimerSummary();
}


int main(int argc, char *argv[]) {
	int ret;
	
	if (argc != 3) {
		std::cerr << "usage: [config_file] [task_num]"
              << std::endl;
		return -EINVAL;
	}

	ret = runtime_init(argv[1], MainHandler, NULL);
	task_num = atoi(argv[2]);
	
	if (ret) {
		printf("failed to start runtime\n");
		return ret;
	}
	
	return 0;
}

extern "C" {
#include "programs/programs.h"
}

#include <chrono>
#include <iostream>
#include <sstream>
#include <x86intrin.h>

#include "uintr.h"
#include "runtime.h"
#include "sync.h"
#include "thread.h"
#include "timer.h"
// #include <base/log.h>

barrier_t barrier;

namespace {

typedef long long (*bench_type)(void);

int threads;
uint64_t n;
std::string worker_spec;

const int BENCH_NUM = 6;
std::string bench_name[BENCH_NUM] = {"mcf", "linpack", "base64", "matmul", "matmul_int", "sum"};
bench_type bench_ptr[BENCH_NUM] = {mcf, linpack, base64, matmul, matmul_int, sum};
std::vector<std::string> task_name;
std::vector<bench_type> task_ptr;
long long task_result[BENCH_NUM];

long long now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

bench_type name2ptr(std::string name) {
	bench_type ptr = nullptr;
	for (int i = 0; i < BENCH_NUM; ++i) {
		if (name == bench_name[i]) {
			ptr = bench_ptr[i];
		}
	}
	return ptr;
}

void parse(std::string input) {
    char delimiter = '+';
    std::stringstream ss(input);
    std::string name;

    while (getline(ss, name, delimiter)) {
       	task_name.push_back(name); 
		task_ptr.push_back(name2ptr(name));   
	}

	std::cout << "tasks:";
    for (const auto& t : task_name) {
        std::cout << ' ' << t;
    }
	std::cout << std::endl;
}

void MainHandler(void *arg) {
	// printf("enter handler\n");
	rt::WaitGroup wg(1);
	barrier_init(&barrier, threads);
	
	// Init functions for benchmarks.
  	base64_init();
	
	int started = 0, finished = 0;
	int task_num = task_name.size();
	for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
			started += 1;
    		// printf("%s start: %d\n", task_name[i].c_str(), started);
			
			if (started < task_num) {
				rt::Yield();
			}
			
			_stui();
			
			long long t1 = now();
			task_result[i] = task_ptr[i]();
			long long t2 = now();
			
			_clui();
			
			finished += 1;
			if (finished == task_num) {
				rt::UintrTimerEnd();
				printf("results:");
				for (int t = 0; t < task_num; ++t) {
					printf(" %lld", task_result[t]);
				}
				printf("\n");
				
				rt::UintrTimerSummary();
				wg.Done();
			}

    	// printf("%s end: %d %.3f\n", task_name[i].c_str(), finished, 1.*(t2-t1)/1e9);
		});
	}
	
	// never returns
	wg.Wait();
}

}  // anonymous namespace


int main(int argc, char *argv[]) {
	int ret;
	
	if (argc != 3) {
		std::cerr << "usage: [config_file] [worker_spec]"
              << std::endl;
		return -EINVAL;
	}

	worker_spec = std::string(argv[2]);
	parse(worker_spec);
	
	ret = runtime_init(argv[1], MainHandler, NULL);
	
	printf("runtime_init ends\n");

	if (ret) {
		printf("failed to start runtime\n");
		return ret;
	}
	
	return 0;
}

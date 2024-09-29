extern "C" {
#include "programs/programs.h"
}
#include "programs/array.cc"

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

// #include <m5_mmap.h>

namespace {

typedef long long (*bench_type)(void);

const int BENCH_NUM = 9;
std::string worker_spec;
std::string bench_name[BENCH_NUM] = {"mcf", "linpack", "base64", "matmul", "matmul_int", "sum", "array", "cmp", "malloctest"};
bench_type bench_ptr[BENCH_NUM] = {mcf, linpack, base64, matmul, matmul_int, sum, array, cmp, malloctest};
std::vector<std::string> task_name;
std::vector<bench_type> task_ptr;
long long task_result[128];

// long long now() {
// 	struct timespec ts;
// 	timespec_get(&ts, TIME_UTC);
// 	return ts.tv_sec * 1e9 + ts.tv_nsec;
// }

bench_type name2ptr(std::string name) {
	bench_type ptr = nullptr;
	for (int i = 0; i < BENCH_NUM; ++i) {
		if (name == bench_name[i]) {
			ptr = bench_ptr[i];
		}
	}
	return ptr;
}

// void parse(std::string input) {
//     char delimiter = '+';
//     std::stringstream ss(input);
//     std::string name;

//     while (getline(ss, name, delimiter)) {
//        	task_name.push_back(name); 
// 		task_ptr.push_back(name2ptr(name));   
// 	}

// 	std::cout << "tasks:";
//     for (const auto& t : task_name) {
//         std::cout << ' ' << t;
//     }
// 	std::cout << std::endl;
// }

void parse(std::string input) {
	char delimiter = '*';
    std::stringstream ss(input);
    std::string name, num_;

	getline(ss, name, delimiter);
	getline(ss, num_);

	int num = atoi(num_.c_str());
	for (int i = 0; i < num; ++i) {
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
	printf("handler\n");
	rt::WaitGroup wg(1);
	
	// Init functions for benchmarks.
  	// base64_init();
	// cmp_init();
	
	int started = 0, finished = 0;
	int task_num = task_name.size();
	for (int i = 0; i < task_num; ++i) {
		rt::Spawn([&, i]() {
			printf("%s start: %d %d\n", task_name[i].c_str(), i, started);
			if (started == 0) {
				rt::UintrTimerStart();
				// printf("UintrTimerStart\n");
			}
			
			started += 1;
			if (started < task_num) {
				rt::Yield();
			}

			task_result[i] = task_ptr[i]();
					
			finished += 1;
			if (finished == task_num) {
				rt::UintrTimerEnd();
				rt::UintrTimerSummary();
				wg.Done();
     		}
    	});
	}
	
	wg.Wait();

	printf("results:");
	for (int t = 0; t < task_num; ++t) {
		printf(" %lld", task_result[t]);
	}
	printf("\n");
}

void MainHandler2(void *arg)  {
	rt::UintrTimerStart();
	sum();
	rt::UintrTimerEnd();
	rt::UintrTimerSummary();
}
}  // anonymous namespace


int main(int argc, char *argv[]) {
	// m5op_addr = 0xFFFF0000;
    // map_m5_mem();	
	// m5_switch_cpu_addr();

	int ret;
	
	if (argc != 3) {
		std::cerr << "usage: [config_file] [worker_spec]"
              << std::endl;
		return -EINVAL;
	}

	worker_spec = std::string(argv[2]);
	parse(worker_spec);
	
	ret = runtime_init(argv[1], MainHandler, NULL);
	
	if (ret) {
		printf("failed to start runtime\n");
		return ret;
	}
	
	// m5_exit();
	return 0;
}

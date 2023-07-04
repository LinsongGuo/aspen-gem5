extern "C" {
#include "programs/programs.h"
}

#include "uintr.h"

#include "runtime.h"
#include "sync.h"
#include "thread.h"
#include "timer.h"
// #include <base/log.h>

#include <chrono>
#include <iostream>
#include <sstream>

barrier_t barrier;

bool synth_barrier_wait() { return barrier_wait(&barrier); }

namespace {

typedef long long (*bench_type)(void);

int threads;
uint64_t n;
std::string worker_spec;
// volatile 

const int BENCH_NUM = 6;
std::string bench_name[BENCH_NUM] = {"mcf", "linpack", "base64", "matmul", "matmul_int", "sum"};
bench_type bench_ptr[BENCH_NUM] = {mcf, linpack, base64, matmul, matmul_int, sum};
std::vector<std::string> task_name;
std::vector<bench_type> task_ptr;
long long task_result[BENCH_NUM];

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

// #pragma GCC optimize("off")
// void measure() {
//   long long ts1 = now();
//   int N = 100000;
//   int *tm = (int*) malloc(sizeof(int) * N);
//   for (int i = 0; i < N; ++i) {
// 	long long start = now();
// 	rt::Yield();
// 	long long end = now();
// 	tm[i] = end - start;
//   }	
//   long long ts2 = now();
//   printf("%lld\n", (ts2 - ts1) / N);
//   for (int i = 0; i < 100; ++i) {
// 	printf("%d ", tm[i]);
//   }	
//   printf("\n");
//   free(tm);
// }
// #pragma GCC optimize("on")

void MainHandler(void *arg) {

//   printf("enter handler\n");
  rt::WaitGroup wg(1);
  barrier_init(&barrier, threads);
  
//   measure();

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
		else {
			uintr_timer_start();
		}	

		_stui();
			
		long long t1 = now();
		task_result[i] = task_ptr[i]();
		long long t2 = now();
  		
		_clui();
			
		finished += 1;
		if (finished == task_num) {
			uintr_timer_end();
			printf("results:");
			for (int t = 0; t < task_num; ++t) {
				printf(" %lld", task_result[t]);
			}
			printf("\n");

			uintr_summary();
 			wg.Done();
		}

    	// printf("%s end: %d %.3f\n", task_name[i].c_str(), finished, 1.*(t2-t1)/1e9);
  	});
  }

  // never returns
  wg.Wait();
}

}  // anonymous namespace


void measure_uif() {
	unsigned long long t1, t2;

	t1 = now();
	t2 = now();
	printf("timing overhead: %lld ns\n", t2 - t1);

	t1 = now();
	int i, count = 100;
	for (i = 0; i < count; ++i) {
		_stui();
	}
	t2 = now();
	printf("stui: %lld ns\n", (t2 - t1) / count);

	t1 = now();
	for (i = 0; i < count; ++i) {
		_clui();
	}
	t2 = now();
	printf("clui: %lld ns\n", (t2 - t1) / count);
}

int main(int argc, char *argv[]) {
  // measure_uif();
 
  int ret;

  if (argc != 5) {
    std::cerr << "usage: [config_file] [#threads] [#n] [worker_spec]"
              << std::endl;
    return -EINVAL;
  }

  threads = std::stoi(argv[2], nullptr, 0);
  n = std::stoul(argv[3], nullptr, 0);
  worker_spec = std::string(argv[4]);
  parse(worker_spec);
 
  uintr_init();

  ret = runtime_init(argv[1], MainHandler, NULL);

  printf("runtime_init ends\n");

  if (ret) {
    printf("failed to start runtime\n");
    return ret;
  }

  return 0;
}

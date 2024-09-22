#include "stdio.h"
#include "programs.h"

#include <x86intrin.h>

// const int N = 5e4;
// long long sum() {
// 	long long res = 0;
// 	int i, j;
// 	for (i = 0; i < N; ++i) {
// 		for (j = N - 1; j >= 0; --j) {
// 			res += i | j;
// 		}
// 	}
// 	// _clui();
// 	// printf("%lld\n", res);
// 	// _stui();
// 	return res;
// }

const int N = 1024;

long long sum() {
// int main() {
	long long sum = 0;
	
	int *data = (int*) malloc(sizeof(int) * N);
	
	int i, j;
	for (i = 0; i < N; ++i) {
		data[i] = (i * i) & 1023;
	}

	for (j = 0; j < N; ++j) {
		for (i = 0; i < N; ++i) {
			sum += data[i] * data[j];
		}
	}

	free(data);

	return sum;
}

// long long sum() {
//     unsigned n = 200000, s = 0;
//     for (int i = 1; i <= n; ++i) {
//         s += i*i;
//     }

// 	return (long long )s;
// }
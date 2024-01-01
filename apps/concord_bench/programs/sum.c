#include "stdio.h"
#include "programs.h"

// #include <x86intrin.h>

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

const int N = 4e4;

long long sum() {
// int main() {
	long long sum = 0;
	
	// _clui();
	int *data = (int*) malloc(sizeof(int) * N);
	// _stui();
	
	int i, j;
	for (i = 0; i < N; ++i) {
		data[i] = (i * i) & 1023;
	}

	for (j = 0; j < N; ++j) {
		for (i = 0; i < N; ++i) {
			sum += data[i] * data[j];
		}
	}

	// _clui();
	// printf("%lld\n", sum);
	free(data);
	// _stui();

	return sum;
}
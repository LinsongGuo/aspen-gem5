#include "stdio.h"
#include "programs.h"

#include <x86intrin.h>

const int N = 4e4;
const int M = 10007;

int *init() {
	_clui();
	int *a = malloc(N * sizeof(int));
	_stui();

	int i = 0, c = 0;
	for (i = 0; i < N; ++i) {
		c = (c + 1LL*i*i) % M;
		a[i] = c;
	}
	return a; 
}

long long sum() {
	//_clui();
	int *a = init();
	//_stui();

	long long res = 0;
	int i, j;
	for (i = 0; i < N; ++i) {
		for (j = N - 1; j >= 0; --j) {
			res += a[i] | a[j];
		}
	}
	// _clui();
	// printf("%lld\n", res);
	// _stui();
	return res;
}

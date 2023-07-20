#include <stdio.h>
#include <time.h>
#include <stdlib.h>

long long now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

int main() {
    int N = 1024 * 1024, M = 64, i, j;
    int** array = (int**) malloc(N * sizeof(int*));

    /* malloc */
    long long start = now();
    for (i = 0; i < N; ++i) {
        array[i] = (int*) malloc(M * sizeof(int));
    }
    long long end = now();
    printf("malloc: %lld ns\n", end - start);

    for (i = 0; i < N; ++i) {
        for (j = 0; j < M; ++j) {
            array[i][j] = i + j;
        }
    }
    unsigned sum = 0;
    for (j = 0; j < M; ++j) {
        for (i = 0; i < N; ++i) {
            sum += array[i][j];
        }
    }
    printf("sum: %d\n", sum);

    /* free */
    start = now();
    for (i = 0; i < N; ++i) {
        free(array[i]);
    }
    end = now();
    printf("free: %lld ns\n", end - start);

    free(array);
    return 0;
}
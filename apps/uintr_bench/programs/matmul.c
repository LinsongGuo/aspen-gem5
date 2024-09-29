// Writen by Attractive Chaos; distributed under the MIT license

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>

#include "programs.h"
//#include <runtime/smalloc.h>

double **mm_init(int n) {
  // _clui();
  double **m = (double **)malloc(n * sizeof(void *));
  for (int i = 0; i < n; ++i) {
    m[i] = calloc(n, sizeof(double));
    // m[i] = smalloc(n * sizeof(double));
  }
  // _stui();
  return m;
}

void mm_destroy(int n, double **m) {
  // _clui();
  for (int i = 0; i < n; ++i) {
    free(m[i]);
  }
  free(m);
  // _stui();
}

double **mm_gen(int n, double seed) {
  double tmp = seed / n / n;
  // _clui();
  double **m = mm_init(n);
  // _stui();
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      m[i][j] = tmp * (i - j) * (i + j);
    }
  }
  return m;
}

// better cache performance by transposing the second matrix
double **mm_mul(int n, double *const *a, double *const *b) {
  //_clui();
  double **m = mm_init(n);
  double **c = mm_init(n);
  // _stui();
  for (int i = 0; i < n; ++i) { // transpose
    for (int j = 0; j < n; ++j) {
      c[i][j] = b[j][i];
    }
  }
  for (int i = 0; i < n; ++i) {
    double *p = a[i], *q = m[i];
    for (int j = 0; j < n; ++j) {
      double t = 0.0, *r = c[j];
      for (int k = 0; k < n; ++k) {
        t += p[k] * r[k];
      }
      q[j] = t;
    }
  }
  mm_destroy(n, c);
  return m;
}

double calc(int n) {
  n = n / 2 * 2;
  double **a = mm_gen(n, 1.0);
  double **b = mm_gen(n, 2.0);
  double **m = mm_mul(n, a, b);
  double result = m[n / 2][n / 2];
  mm_destroy(n, a);
  mm_destroy(n, b);
  mm_destroy(n, m);
  return result;
}

long long matmul() {
  // printf("**************** matmul start\n");
  int n = 1100;
/*
  _clui();
  // printf("p1\n");
  double left = calc(101);
  double right = -18.67; 
  // printf("p2\n");
  // _stui();

  // _clui();
  if (fabs(left - right) > 0.1) {
    fprintf(stderr, "%f != %f\n", left, right);
    exit(EXIT_FAILURE);
  }
  _stui();
*/
  // notify_with_pid("C/" COMPILER);
 // _clui();
  double results = calc(n);
  //_stui();
  // notify("stop");
  // printf("**************** matmul ends\n");
  
  return (long long)results*1e6;
  // return fabs(results + 191.1) < 0.1; 
  // _clui();
 //  printf("%f\n", results);
 // _stui();

}

#include <stdio.h>
#include <string.h>
#include <assert.h>

// #include <x86intrin.h>
#include "programs.h"

char *generateString(int n, int x) {
    if (n <= 0) {
        return NULL; // Return NULL for invalid input
    }
    
    char *result = (char *)malloc(n + 1); // +1 for the null terminator
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    for (int i = 0; i < n; ++i) {
        result[i] = 'a' + (i % x); // Fill with 'A', 'B', 'C' in a repeating pattern
    }
    result[n] = '\0'; // Null-terminate the string

    return result;
}

#define charsetLength 100000
char charset[charsetLength + 10];

void init() {
    srand(100);
    for (int i = 0; i < charsetLength; ++i) {
        charset[i] = 'a' + rand() % 3; 
    }
    charset[charsetLength] = '\0';
}

char *generateRandomString(int n, int x) {
    if (n <= 0) {
        return NULL; // Return NULL for invalid input
    }

    // Seed the random number generator using the current time

    char *result = (char *)malloc(n + 1); // +1 for the null terminator
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }

    int randomIndex = x % charsetLength;
    for (int i = 0; i < n; ++i) {
        result[i] = charset[randomIndex];
        randomIndex = (randomIndex + 1) % charsetLength;
    }
    result[n] = '\0'; // Null-terminate the string

    return result;
}

void cmp_init() {
    init();
}

long long cmp() {
    const int loopCount = 1e6;  

    int res = 0;
    for (int i = 0; i < loopCount; ++i) {
        char* s1 = generateRandomString(100, i);
        char* s2 = generateRandomString(100, loopCount - i);
        
        int comparisonResult = memcmp(s1, s2, sizeof(s1));
        
        // assert(comparisonResult != 0);
        res += (comparisonResult < 0);

        free(s1);
        free(s2);
    }
    return res;
}

// int main() {
//     printf("%lld\n", cmp());
// }
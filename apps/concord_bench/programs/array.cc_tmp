#include <cstdio>
#include <iostream>
#include <array>
#include <memory>


class test_t {
public:
   
    struct Buffer {
        Buffer() : buf_start_offset(0), buf_len(0), buf_capacity(0) {}
        Buffer(uint32_t value) : buf_start_offset(value), buf_len(0), buf_capacity(0) {}
        std::unique_ptr<char[]> buf;
        uint32_t buf_start_offset;
        uint32_t buf_len;
        uint32_t buf_capacity;
    };

    test_t(unsigned int value) {
        buffers_[0].reset(new Buffer(value));
    }

    int get() {
        return buffers_[0].get()->buf_start_offset;
    }
    std::array<std::unique_ptr<Buffer>, 2> buffers_;
};

const int N = 1e7;
long long array() {
    // test_t a(1  );
    // printf("buffer: %ld, %p, %p\n", a.buffers_.size(), a.buffers_[0].get(), a.buffers_[1].get());
    
    // printf("array\n");
    unsigned int sum = 0;
    for ( int i = 0; i < N; ++i) {
        //printf("for %d\n", i);
        test_t a(i);
        sum += a.get();
    }

    // printf("%u\n", sum);
    
    return (long long) sum;
}

// int main() {
//     // printf("array\n");
//     unsigned int sum = 0;
//     for ( int i = 0; i < N; ++i) {
//         //printf("for %d\n", i);
//         // test_t a(i);
//         sum += i; // a.value;
//     }

//     printf("%u\n", sum);
    
//     return 0;
// }
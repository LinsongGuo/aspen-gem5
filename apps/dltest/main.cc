#include "sync.h"

int _main() {
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    // Attempt to lock the mutex using pthread_mutex_trylock
    if (pthread_mutex_trylock(&mutex) == 0) {
        printf("Mutex locked\n");
        pthread_mutex_unlock(&mutex);
    } else {
        printf("Mutex already locked\n");
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "usage: [cfg_file]" << std::endl;
        return -EINVAL;    
    }
    
    printf("runtime_init starts\n");
    int ret = runtime_init(argv[1], _main, NULL);
    printf("runtime_init ends\n");
    
    return 0;
}
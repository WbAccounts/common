#include "thread.hpp"
#include <stdio.h>
#include <unistd.h>

int main() {
    pthread_t tid = pthread_self();
    printf("main : %lu\n", tid);
    int i;
    bool first = true;
    thread::CThread thread([&] (void* arg) {
        pthread_t id = pthread_self();
        printf("thread : %lu\n", id);
        for (i=0; i<10; ++i) {
            printf("thread run %d\n", i);
            sleep(1);
            if (i == 3 && first) {
                first = false;
                thread.wait();
            }
        }
    });
    thread.run();
    sleep(10);
    i=0;
    thread.signal();
    return 0;
}
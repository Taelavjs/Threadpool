#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

typedef struct {
    pthread_t *threads;
    int num_threads;
} threadpool_t;

threadpool_t *init_threadpool(int num_threads, void *(*thread_work)(void *)) {
    threadpool_t *pool = (threadpool_t *) malloc(sizeof(threadpool_t));
    pool->num_threads = num_threads;
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, thread_work, (void *)&pool);
    }

    return pool;
};

void wait_all_threads(threadpool_t *pool) {
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], nullptr);
    }
}

void *test(void *arg) {
    printf("Hello, World!\n");
    return nullptr;
}

int main(void) {
    threadpool_t *tp = init_threadpool(16, &test);
    wait_all_threads(tp);
    return 0;
}

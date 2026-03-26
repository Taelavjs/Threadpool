#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>


struct thread_buffer_data {
    int id;
    int time;
};

typedef struct {
    pthread_t *threads;
    int num_threads;
    size_t buffer_size;
    struct thread_buffer_data *buffer;
    int head;
    int tail;
    sem_t producer;
    sem_t consumer;
    int count;
} threadpool_t;

threadpool_t *init_threadpool(int num_threads, void *(*producer_work)(void *), void *(*consumer_work)(void *)) {
    threadpool_t *pool = (threadpool_t *) malloc(sizeof(threadpool_t));
    pool->num_threads = num_threads;
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    pool->buffer_size = 10;
    pool->buffer = malloc(sizeof(struct thread_buffer_data) * 10);;
    pool->head = 0;
    pool->tail = 0;
    pool->count = 0;
    sem_init(&pool->producer, 0, 10);
    sem_init(&pool->consumer, 0, 0);
    for (int i = 0; i < num_threads; i++) {
        int r = rand() % 20;
        if (r < 5) {
            // producer
            pthread_create(&pool->threads[i], NULL, producer_work, (void *)pool);
        } else {
            // consumer
            pthread_create(&pool->threads[i], NULL, consumer_work, (void *)pool);
        }
    }
    return pool;
};

void *grab_data(void *tpool) {
    threadpool_t *pool = tpool;
    do{
    sem_wait(&pool->consumer);
    struct thread_buffer_data data = pool->buffer[pool->tail];
        sleep(data.time);
    printf("[Consumer] Thread %lu: Grabbed data id=%d, sleep=%d from buffer index %d\n",
       pthread_self(), data.id, data.time, pool->tail);
    pool->tail = (pool->tail + 1) % 10;


    sem_post(&pool->producer);
    }while (1);
    return nullptr;
}

void *put_data(void *tpool) {
    threadpool_t *pool = tpool;
    do {
        sem_wait(&pool->producer);
        int r = rand() % 20;
        struct thread_buffer_data sample_data;;
        sample_data.id = pool->count++;
        sample_data.time = r;
        pool->buffer[pool->head] = sample_data;
        printf("[Producer] Thread %lu: Placed data id=%d, sleep=%d at buffer index %d\n",
            pthread_self(), sample_data.id, sample_data.time, pool->head);
        pool->head = (pool->head + 1) % 10;

        sem_post(&pool->consumer);
    }while (1);
    return nullptr;
}

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
    srand(time(NULL));   // Initialization, should only be called once.
    threadpool_t *tp = init_threadpool(16, put_data, grab_data);
    wait_all_threads(tp);
    return 0;
}

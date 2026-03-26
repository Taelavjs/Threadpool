#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>

struct buff_data {
    int id;
    int time;
};

typedef struct {
    pthread_t *threads;
    int num_threads;
    size_t buffer_size;
    void **buffer;
    int head;
    int tail;
    sem_t producer;
    sem_t consumer;
    pthread_mutex_t buffer_lock;
    bool *is_done;
    int count;
    void (*pool_finish_work)(bool *);
} threadpool_t;

void pool_finish_work(bool *is_done) {
    *is_done = true;
}
threadpool_t *init_threadpool(int num_threads, void *buffer_struct, void *(*producer_work)(void *)
    , void *(*consumer_work)(void *), int thread_number, int producerCount) {
    threadpool_t *pool = (threadpool_t *) malloc(sizeof(threadpool_t));
    pool->num_threads = num_threads;
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads);
    pool->buffer_size = thread_number;
    void *ptr;
    pool->buffer = malloc(sizeof(ptr) * pool->buffer_size);;
    pool->head = 0;
    pool->tail = 0;
    pool->count = pool->buffer_size;
    pool->is_done = malloc(sizeof(bool));
    *pool->is_done = false;
    pool->pool_finish_work = pool_finish_work;
    sem_init(&pool->producer, 0, pool->buffer_size);
    sem_init(&pool->consumer, 0, 0);
    pthread_mutex_init(&pool->buffer_lock, nullptr);
    for (int i = 0; i < num_threads; i++) {
        if (i < producerCount) {
            // producer
            pthread_create(&pool->threads[i],NULL, producer_work, (void *)pool);
        } else {
            // consumer
            pthread_create(&pool->threads[i],NULL, consumer_work, (void *)pool);
        }
    }
    return pool;
};
void wait_all_threads(threadpool_t *pool) {
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], nullptr);
    }
}

void destroy_threadpool(threadpool_t *pool) {
    free(pool->buffer);
    free(pool->is_done);
    free(pool->threads);
    free(pool);
}


// void *grab_data(void *tpool) {
//     int run_count = 500;
//     threadpool_t *pool = tpool;
//     do{
//
//         sem_wait(&pool->consumer);
//         pthread_mutex_lock(&pool->buffer_lock);
//
//         struct buff_data *data = (struct buff_data *)pool->buffer[pool->tail];
//         if (*(pool->is_done) && pool->count < 0 && pool->tail == pool->head) {
//             sem_post(&pool->consumer);
//             pthread_mutex_unlock(&pool->buffer_lock);
//             printf("[Consumer] Thread %lu: End \n", pthread_self());
//             return nullptr;
//         };
//
//         pool->tail = (pool->tail + 1) % pool->buffer_size;
//         int temp = pool->tail;
//         pthread_mutex_unlock(&pool->buffer_lock);
//         sleep(data->time);
//         sem_post(&pool->producer);
//     }while (1);
// }
//
// void buffer_add(threadpool_t *pool) {
//     int r = rand() % 3;
//     struct buff_data *sample_data = malloc(sizeof(struct buff_data));
//     sample_data->id = pool->count--;
//     sample_data->time = r;
//     pthread_mutex_lock(&pool->buffer_lock);
//     if (*pool->is_done == true) {
//         pthread_mutex_unlock(&pool->buffer_lock);
//         free(sample_data);
//         return;
//     }
//     pool->buffer[pool->head] = sample_data;
//     pool->head = (pool->head + 1) % pool->buffer_size;
//     pthread_mutex_unlock(&pool->buffer_lock);
//     sem_post(&pool->consumer);
// }
// void *put_data(void *tpool) {
//     threadpool_t *pool = tpool;
//     int run_count = 50;
//
//     do {
//         sem_wait(&pool->producer);
//         if (pool->count <= -1) {
//             pool->pool_finish_work(pool->is_done);
//             buffer_add(pool);
//             return nullptr;
//         }
//
//         buffer_add(pool);
//     }while (1);
// }

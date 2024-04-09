#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define MAX_VALUE 100

int buffer[BUFFER_SIZE];
sem_t full, empty;
pthread_mutex_t mutex;

void* producer(void* arg) {
    int item;
    while (1) {
        item = rand() % MAX_VALUE; // Produce a random item
        sem_wait(&empty);
        pthread_mutex_lock(&mutex);
        // Critical section: Add item to the buffer
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (buffer[i] == -1) {
                buffer[i] = item;
                printf("Producer produced item: %d\n", item);
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
        sem_post(&full);
        usleep(rand() % 1000000); // Sleep for a random amount of time
    }
}

void* consumer(void* arg) {
    int item;
    while (1) {
        sem_wait(&full);
        pthread_mutex_lock(&mutex);
        // Critical section: Remove item from the buffer
        for (int i = BUFFER_SIZE - 1; i >= 0; i--) {
            if (buffer[i] != -1) {
                item = buffer[i];
                buffer[i] = -1;
                printf("Consumer consumed item: %d\n", item);
                break;
            }
        }
        pthread_mutex_unlock(&mutex);
        sem_post(&empty);
        usleep(rand() % 1000000); // Sleep for a random amount of time
    }
}

int main() {
    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL);

    // Initialize buffer
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = -1;
    }

    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_create(&producers[i], NULL, producer, NULL);
    }

    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    // Join producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    // Join consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Clean up resources
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}

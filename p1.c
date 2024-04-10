#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_DINERS 100
#define BATCH_SIZE 10

int num_diners = 0;           // Number of diners currently in the restaurant
int num_finished_diners = 0;  // Number of diners that have finished their meal

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t front_door_sem;
sem_t back_door_sem;

void* diner_thread(void* arg) {
    int diner_id = *(int*)arg;

    sem_wait(&front_door_sem);

    pthread_mutex_lock(&mutex);
    num_diners++;
    printf("Diner %d entered the restaurant. Total diners: %d\n", diner_id, num_diners);

    if (num_diners == BATCH_SIZE) {
        sem_post(&back_door_sem);
    }

    pthread_mutex_unlock(&mutex);

    // Simulating diner having a meal
    sleep(rand() % 5 + 1);

    pthread_mutex_lock(&mutex);
    num_finished_diners++;
    printf("Diner %d finished their meal. Total finished diners: %d\n", diner_id, num_finished_diners);

    if (num_finished_diners == BATCH_SIZE) {
        num_diners = 0;
        num_finished_diners = 0;
        sem_post(&front_door_sem);
        sem_wait(&back_door_sem);
        printf("All diners have exited. Next batch is ready.\n");
    }

    pthread_mutex_unlock(&mutex);

    sem_post(&front_door_sem);
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    pthread_t diner_threads[MAX_DINERS];
    int diner_ids[MAX_DINERS];

    sem_init(&front_door_sem, 0, BATCH_SIZE);
    sem_init(&back_door_sem, 0, 0);

    // Create diner threads
    for (int i = 0; i < MAX_DINERS; i++) {
        diner_ids[i] = i + 1;
        if (pthread_create(&diner_threads[i], NULL, diner_thread, &diner_ids[i]) != 0) {
            fprintf(stderr, "Error creating diner thread %d\n", i + 1);
            exit(1);
        }
    }

    // Wait for the main thread to be canceled (run infinitely)
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_cancel(pthread_self());

    // Join diner threads (won't be reached in this case)
    for (int i = 0; i < MAX_DINERS; i++) {
        pthread_join(diner_threads[i], NULL);
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 5 // Number of diners

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_front_door = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_back_door = PTHREAD_COND_INITIALIZER;

int diners_inside = 0; // Number of diners currently inside the restaurant
int served_diners = 0; // Number of diners served in the current batch

void *diner(void *arg) {
    int id = *((int *)arg);

    // Wait for the front door to open
    pthread_mutex_lock(&mutex);
    while (diners_inside == N) {
        pthread_cond_wait(&cond_front_door, &mutex);
    }

    // Enter the restaurant
    diners_inside++;
    printf("Diner %d entered the restaurant.\n", id);

    // If all N diners have entered, notify the restaurant to start serving
    if (diners_inside == N) {
        pthread_cond_signal(&cond_back_door);
    }

    pthread_mutex_unlock(&mutex);

    // Wait for the back door to open
    pthread_mutex_lock(&mutex);
    while (served_diners < N) {
        pthread_cond_wait(&cond_back_door, &mutex);
    }

    // Exit the restaurant
    diners_inside--;
    served_diners--;
    printf("Diner %d exited the restaurant.\n", id);

    // If all diners have exited, notify the restaurant to open the front door
    if (diners_inside == 0) {
        pthread_cond_signal(&cond_front_door);
    }

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void *restaurant(void *arg) {
    while (1) {
        // Open the front door
        pthread_mutex_lock(&mutex);
        while (diners_inside > 0) {
            pthread_cond_wait(&cond_front_door, &mutex);
        }

        // Serve the diners
        printf("Restaurant serving the diners.\n");
        served_diners = N;

        // Notify the diners that the back door is open
        pthread_cond_broadcast(&cond_back_door);

        // Wait for all diners to exit
        while (diners_inside < N) {
            pthread_cond_wait(&cond_front_door, &mutex);
        }

        // Prepare for the next batch
        printf("Restaurant preparing for the next batch.\n");
        served_diners = 0;

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main() {
    pthread_t restaurant_thread;
    pthread_t diner_threads[N];
    int diner_ids[N];
    int i;

    // Create the restaurant thread
    pthread_create(&restaurant_thread, NULL, restaurant, NULL);

    // Create the diner threads
    for (i = 0; i < N; i++) {
        diner_ids[i] = i + 1;
        pthread_create(&diner_threads[i], NULL, diner, &diner_ids[i]);
    }

    // Wait for all threads to finish
    pthread_join(restaurant_thread, NULL);
    for (i = 0; i < N; i++) {
        pthread_join(diner_threads[i], NULL);
    }

    return 0;
}

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_STUDENTS 5
#define PIZZA_SLICES 8

sem_t pizza_mutex;          // Mutex to control access to pizza
sem_t pizza_available;      // Semaphore to signal availability of pizza
int pizza_slices = PIZZA_SLICES; // Number of slices available

void* student(void* arg) {
    int student_id = *(int*)arg;
    while (1) {
        sem_wait(&pizza_mutex); // Wait for pizza mutex
        if (pizza_slices == 0) { // If pizza is exhausted
            sem_post(&pizza_mutex); // Release pizza mutex
            sem_wait(&pizza_available); // Wait for pizza to arrive
        } else {
            pizza_slices--; // Consume a slice of pizza
            printf("Student %d takes a slice of pizza, remaining slices: %d\n", student_id, pizza_slices);
            sem_post(&pizza_mutex); // Release pizza mutex
            usleep(2000000); // Sleep to simulate eating time
        }
    }
    return NULL;
}

void* kamal_pizza_delivery(void* arg) {
    while (1) {
        sem_wait(&pizza_mutex); // Wait for pizza mutex
        if (pizza_slices > 0) { // If pizza is not exhausted, release mutex and continue
            sem_post(&pizza_mutex);
            continue;
        }
        // If pizza is exhausted, call Kamal's Pizza
        printf("Pizza is exhausted. Calling Kamal's Pizza for another pizza...\n");
        pizza_slices = PIZZA_SLICES; // Reset pizza slices
        sem_post(&pizza_available); // Signal availability of pizza
        sem_post(&pizza_mutex); // Release pizza mutex
        usleep(2000000); // Sleep to simulate pizza delivery time
    }
    return NULL;
}

int main() {
    pthread_t students[NUM_STUDENTS], kamal_thread;
    int student_ids[NUM_STUDENTS];

    sem_init(&pizza_mutex, 0, 1); // Initialize pizza mutex
    sem_init(&pizza_available, 0, 0); // Initialize pizza available semaphore

    // Create student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        student_ids[i] = i + 1;
        pthread_create(&students[i], NULL, student, &student_ids[i]);
    }

    // Create Kamal's pizza delivery thread
    pthread_create(&kamal_thread, NULL, kamal_pizza_delivery, NULL);

    // Join student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        pthread_join(students[i], NULL);
    }

    // Join Kamal's pizza delivery thread
    pthread_join(kamal_thread, NULL);

    // Clean up resources
    sem_destroy(&pizza_mutex);
    sem_destroy(&pizza_available);

    return 0;
}

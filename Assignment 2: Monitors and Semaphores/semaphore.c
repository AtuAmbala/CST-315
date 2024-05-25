// This program demonstrates the producer-consumer problem using a semaphore
// The producer thread produces items and adds them to a shared buffer
// The consumer thread consumes items from the buffer
// The producer and consumer threads are synchronized using semaphores
// The buffer is protected by a mutex
// The producer thread produces items at random intervals
// The consumer thread consumes items at random intervals
//
// Progrmed by: Ricardo Escarcega and Atuhaire Ambala




#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> AZs

#define BUFFER_SIZE 10

int buffer[BUFFER_SIZE];
int count = 0; // Number of items in the buffer
int in = 0;    // Index for the next produced item
int out = 0;   // Index for the next consumed item

// Semaphores to track the number of empty and full slots in the buffer
sem_t empty; // Semaphore to count empty slots
sem_t full;  // Semaphore to count filled slots
// Mutex to protect access to the shared buffer
pthread_mutex_t mutex;

void* producer(void* arg) {
    int item;
    while (1) {
        item = rand() % 100; // Produce an item (generate a random number)

        sem_wait(&empty); // Wait for an empty slot
        pthread_mutex_lock(&mutex); // Lock the buffer

        // Add the produced item to the buffer
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE; // Update the index for the next item
        count++;

        printf("Produced: %d\n", item);
        pthread_mutex_unlock(&mutex); // Unlock the buffer
        sem_post(&full); // Signal that a new item is available

        sleep(1); // Sleep to simulate time taken to produce an item
    }
}

void* consumer(void* arg) {
    int item;
    while (1) {
        sem_wait(&full); // Wait for a filled slot
        pthread_mutex_lock(&mutex); // Lock the buffer

        // Remove the item from the buffer
        item = buffer[out];
        out = (out + 1) % BUFFER_SIZE; // Update the index for the next item
        count--;

        printf("Consumed: %d\n", item);
        pthread_mutex_unlock(&mutex); // Unlock the buffer
        sem_post(&empty); // Signal that an empty slot is available

        sleep(1); // Sleep to simulate time taken to consume an item
    }
}

int main() {
    pthread_t producers[3], consumers[3]; // Create arrays for producer and consumer threads

    // Initialize the semaphores and mutex
    sem_init(&empty, 0, BUFFER_SIZE); // Initially, all slots are empty
    sem_init(&full, 0, 0); // Initially, no slots are full
    pthread_mutex_init(&mutex, NULL); // Initialize the mutex

    // Create producer and consumer threads
    for (int i = 0; i < 3; i++) {
        pthread_create(&producers[i], NULL, producer, NULL);
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }

    // Wait for all producer and consumer threads to finish
    for (int i = 0; i < 3; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }

    // Destroy the semaphores and mutex
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}

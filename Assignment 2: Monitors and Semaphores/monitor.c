
// This program demonstrates the producer-consumer problem using a monitor
// The producer thread produces items and adds them to a shared buffer
// The consumer thread consumes items from the buffer
// The producer and consumer threads are synchronized using condition variables
// The buffer is protected by a mutex
// The producer thread produces items at random intervals
// The consumer thread consumes items at random intervals
//
// Progrmed by: Ricardo Escarcega and Atuhaire Ambala


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define BUFFER_SIZE 10

int buffer[BUFFER_SIZE];
int count = 0; // Number of items in the buffer
int in = 0;    // Index for the next produced item
int out = 0;   // Index for the next consumed item

// Mutex to protect access to the shared buffer
pthread_mutex_t mutex;
// Condition variables to signal the state of the buffer
pthread_cond_t not_empty; // Signaled when the buffer is not empty
pthread_cond_t not_full;  // Signaled when the buffer is not full

void* producer(void* arg) {
    int item;
    while (1) {
        item = rand() % 100; // Produce an item (generate a random number)

        pthread_mutex_lock(&mutex); // Lock the buffer

        // Wait while the buffer is full
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex); // Wait for not_full condition
        }

        // Add the produced item to the buffer
        buffer[in] = item;
        in = (in + 1) % BUFFER_SIZE; // Update the index for the next item
        count++;

        printf("Produced: %d\n", item);

        pthread_cond_signal(&not_empty); // Signal that the buffer is not empty
        pthread_mutex_unlock(&mutex); // Unlock the buffer

        sleep(1); // Sleep to simulate time taken to produce an item
    }
}

void* consumer(void* arg) {
    int item;
    while (1) {
        pthread_mutex_lock(&mutex); // Lock the buffer

        // Wait while the buffer is empty
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex); // Wait for not_empty condition
        }

        // Remove the item from the buffer
        item = buffer[out];
        out = (out + 1) % BUFFER_SIZE; // Update the index for the next item
        count--;

        printf("Consumed: %d\n", item);

        pthread_cond_signal(&not_full); // Signal that the buffer is not full
        pthread_mutex_unlock(&mutex); // Unlock the buffer

        sleep(1); // Sleep to simulate time taken to consume an item
    }
}

int main() {
    pthread_t producers[3], consumers[3]; // Create arrays for producer and consumer threads

    // Initialize the mutex and condition variables
    pthread_mutex_init(&mutex, NULL); // Initialize the mutex
    pthread_cond_init(&not_empty, NULL); // Initialize the not_empty condition variable
    pthread_cond_init(&not_full, NULL);  // Initialize the not_full condition variable

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

    // Destroy the mutex and condition variables
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return 0;
}

/**
 * Producer-Consumer Problem Solution
 * 
 * Programmers: Atu Ambala and Ricardo Escarcega
 * 
 * This program demonstrates a solution to the classic Producer-Consumer problem
 * using threads in C, with synchronization achieved through a mutex. The producer 
 * thread generates items and places them into a shared buffer, while the consumer 
 * thread retrieves and processes these items. The buffer is protected by a mutex to 
 * ensure thread-safe access, preventing race conditions and ensuring proper synchronization.
 * 
 * Functions:
 * - produce: Generates an item to be consumed.
 * - put: Adds an item to the buffer. Waits if the buffer is full.
 * - get: Retrieves an item from the buffer. Waits if the buffer is empty.
 * - consume: Processes an item retrieved from the buffer.
 * - producer: Function run by the producer thread, continuously producing items.
 * - consumer: Function run by the consumer thread, continuously consuming items.
 * 
 * Global Variables:
 * - buffer: The shared buffer for items, implemented as a single-element array.
 * - count: The current number of items in the buffer.
 * - mutex: A mutex for synchronizing access to the buffer and count variable.
 * 
 * Thread Management:
 * - producer_thread: Thread handle for the producer thread.
 * - consumer_thread: Thread handle for the consumer thread.
 * 
 * Synchronization:
 * - The mutex is used to lock critical sections in the put and get functions to 
 *   ensure that only one thread can modify the buffer and count variable at a time.
 * 
 * Usage:
 * - Compile the program using a C compiler, e.g., gcc.
 * - Run the executable to see the producer and consumer threads in action.
 * 
 * Note:
 * - This implementation uses sleep to simulate the time taken for producing and consuming items.
 * - This approach ensures that the producer does not overtake the consumer and the consumer does 
 *   not have to wait unnecessarily, maintaining a smooth flow of item production and consumption.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// import the libraries

//Define the buffer , its size , a buffer count
#define BUFFER_SIZE 1

// Define buffer size and its count
int buffer[BUFFER_SIZE];
int count = 0; // Tracks the number of items in the buffer

// producer, consumer and mutex threads
pthread_t producer_thread, consumer_thread;
pthread_mutex_t mutex;

// The mutex is initialized and destroyed to free any resources allocated for the mutex.
// Mutex locking and unlocking are used around the critical sections in put() and get().
//This ensures that only one thread can modify the buffer and count variable at a time.


int produce() {
    static int item = 0;
    return item++;
}

// The put function adds an item to the buffer if there is space, ensuring thread safety by using a mutex.
void put(int item) {
    // If buffer is full, sleeps for 1 second before checking again. 
    // This ensures the producer waits until there is space in the buffer.

    while (count == BUFFER_SIZE) { // Buffer full when equal to size i.e 1
        sleep(1);
    }
    pthread_mutex_lock(&mutex);
    buffer[count++] = item; // Add the item to the buffer and increment the count
    pthread_mutex_unlock(&mutex);
}

//The get function retrieves an item from the buffer if there is an item available, ensuring thread safety by using a mutex
int get() {
    // If buffer is empty, it sleeps for 1 second before checking again. 
    // This ensures the consumer waits until there is an item in the buffer
    while (count == 0) { // Buffer empty when zero
        sleep(1);
    }
    pthread_mutex_lock(&mutex);
    // item is removed from the buffer at the position count - 1 and count is decremented
    int item = buffer[--count]; // Remove the item from the buffer and decrement the count
    pthread_mutex_unlock(&mutex);
    return item; // Return the retrieved item
}

void consume(int item) {
    printf("Consumed: %d\n", item);
}


void* producer(void* arg) {
    while (1) {
        int item = produce();
        put(item);
        printf("Produced: %d\n", item);
        sleep(1); // Simulate time taken to produce
    }
    return NULL;
}

void* consumer(void* arg) {
    while (1) {
        int item = get();
        consume(item);
        sleep(2); // Simulate time taken to consume
    }
    return NULL;
}


int main() {
    pthread_mutex_init(&mutex, NULL);

    pthread_create(&producer_thread, NULL, producer, NULL);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    pthread_mutex_destroy(&mutex);

    return 0;
}


// In this scenario, multiple processes (threads) compete for access to a shared resource.
// To handle resource contention and prevent starvation, a timer mechanism is used. 
// If a process fails to access the resource within a specified timeout period, it is considered starved and is restarted. 
//This approach ensures that processes do not indefinitely wait for the resource, thus preventing deadlock.

// Programmers: Atuhaire Ambala and Ricardo Escarcega

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Define constants
#define NUM_PROCESSES 50
#define TIMEOUT 5 // Time in seconds before a process is considered starved
#define MAX_RETRIES 3

// Mutex to control access to the shared resource
pthread_mutex_t resource_mutex;
int resource_in_use = 0;

// Structure to hold process data
typedef struct {
    int id;
    int retries;
    time_t start_time;
    FILE *log_file;
} process_data;

// Function to simulate process behavior
void* process_function(void* arg) {
    process_data *pdata = (process_data*) arg;

    while (pdata->retries < MAX_RETRIES) {
        time(&pdata->start_time); // Record start time
        fprintf(pdata->log_file, "Process %d is trying to access the resource.\n", pdata->id);
        fflush(pdata->log_file);

        // Attempt to lock the mutex
        if (pthread_mutex_trylock(&resource_mutex) == 0) {
            resource_in_use = 1;
            fprintf(pdata->log_file, "Process %d has gained access to the resource.\n", pdata->id);
            fflush(pdata->log_file);
            sleep(1); // Simulate resource usage
            fprintf(pdata->log_file, "Process %d is releasing the resource.\n", pdata->id);
            fflush(pdata->log_file);
            resource_in_use = 0;
            pthread_mutex_unlock(&resource_mutex);
            break;
        } else {
            fprintf(pdata->log_file, "Process %d could not access the resource. It is currently in use.\n", pdata->id);
            fflush(pdata->log_file);
        }

        sleep(1); // Wait before retrying
        time_t current_time;
        time(&current_time);

        // Check if the process has been starved
        if (difftime(current_time, pdata->start_time) >= TIMEOUT) {
            fprintf(pdata->log_file, "Process %d is starved and will be restarted.\n", pdata->id);
            fflush(pdata->log_file);
            pdata->retries++;
            time(&pdata->start_time); // Restart timer
        }
    }

    if (pdata->retries == MAX_RETRIES) {
        fprintf(pdata->log_file, "Process %d has reached the maximum number of retries and will not be restarted.\n", pdata->id);
        fflush(pdata->log_file);
    }

    return NULL;
}

int main() {
    pthread_t processes[NUM_PROCESSES];
    process_data pdata[NUM_PROCESSES];

    // Open the log file
    FILE *log_file = fopen("activity_log.txt", "w");
    if (!log_file) {
        perror("Failed to open log file");
        return 1;
    }

    // Initialize the mutex
    pthread_mutex_init(&resource_mutex, NULL);

    // Create threads
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pdata[i].id = i + 1;
        pdata[i].retries = 0;
        pdata[i].log_file = log_file;
        pthread_create(&processes[i], NULL, process_function, (void*)&pdata[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pthread_join(processes[i], NULL);
    }

    // Destroy the mutex
    pthread_mutex_destroy(&resource_mutex);

    // Close the log file
    fclose(log_file);
    return 0;
}

Producer and Consumer

Programmers: Atu Ambala and Ricardo Escarcega

This program demonstrates a solution to the classic Producer-Consumer problem using threads in C, with synchronization achieved through a mutex. The producer thread generates items and places them into a shared buffer, while the consumer thread retrieves and processes these items. The buffer is protected by a mutex to ensure thread-safe access, preventing race conditions and ensuring proper synchronization.

Functions:
produce: Generates an item to be consumed.
put: Adds an item to the buffer. Waits if the buffer is full.
get: Retrieves an item from the buffer. Waits if the buffer is empty.
consume: Processes an item retrieved from the buffer.
producer: Function run by the producer thread, continuously producing items.
consumer: Function run by the consumer thread, continuously consuming items.

Global Variables:
buffer: The shared buffer for items, implemented as a single-element array.
count: The current number of items in the buffer.
mutex: A mutex for synchronizing access to the buffer and count variable.

Thread Management:
producer_thread: Thread handle for the producer thread.
consumer_thread: Thread handle for the consumer thread.
Synchronization:

The mutex is used to lock critical sections in the put and get functions to ensure that only one thread can modify the buffer and count variable at a time.

Usage:
Compile the program using a C compiler, e.g., gcc.
Run the executable to see the producer and consumer threads in action.
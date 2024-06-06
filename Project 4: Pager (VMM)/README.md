# Custom Shell with Memory Management and LRU Page Replacement

## Programmers
Atuhaire Amabla and Ricardo Escarcega

## Introduction

This project implements a custom Unix/Linux command line interpreter (shell) with advanced features such as memory management, page fault handling, and Least Recently Used (LRU) page replacement. The shell supports both interactive and batch modes, maintains a command history, and includes built-in commands like `cd`. Additionally, it handles resource management upon process termination, ensuring that allocated memory and open file descriptors are properly cleaned up.

## Software Requirements

- GCC (GNU Compiler Collection)
- Unix/Linux operating system
- Make (optional, for ease of compilation)

## Features

- **Interactive Mode**: Allows users to enter commands directly.
- **Batch Mode**: Executes commands from a file.
- **Command History**: Maintains a history of commands entered.
- **Built-in Commands**: Supports commands like `cd` to change directories.
- **Signal Handling**: Gracefully handles SIGINT (Ctrl+C) and SIGQUIT (Ctrl+\).
- **Memory Management**: Implements virtual memory and paging.
- **Page Fault Handling**: Manages page faults and page replacement using the LRU algorithm.
- **Resource Management**: Cleans up resources on process termination.

## Software Requirements

- GCC (GNU Compiler Collection)
- Unix/Linux operating system
- Make (optional, for ease of compilation)

## Compilation Instructions

To compile the project, use the following commands:

```sh
gcc -o lopeShell lopeShell.c
gcc -o page_fault_test page_fault_test.c
gcc -o resource_management_test resource_management_test.c
gcc -o lru_test lru_test.c


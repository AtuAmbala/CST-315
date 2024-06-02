Unix Command Line Interpreter

Programmers:
Atu Ambala
Ricardo Escarcega

Description: 
The Unix/Linux Command Line Interpreter is a simplified shell program designed to execute user commands in both interactive and batch modes. The shell accepts user input, executes commands, and supports concurrent execution of multiple commands separated by semicolons, command history and directory navigation. This project lays the foundation for a full-featured shell by focusing on basic command execution and signal handling.

Prerequisites:
- Unix/Linux environment
- GCC compiler

To compile the LopeShell program, use the following gcc command:
gcc -o lopeShell lopeShell.c

Run LopeShell without any arguments to enter interactive mode:
./lopeShell 

In interactive mode, the shell displays a prompt and waits for the user to enter commands: 
$lopeShell > ls
$lopeShell > /bin/ls
$lopeShell > ls -l
$lopeShell > ls -l; touch file

Features:
Custom prompt for command input.
Execute single and multiple commands.
Concurrent execution of multiple commands.
Signal handling for graceful exit (CTRL+C) and immediate termination (CTRL+).
Batch mode for executing commands from a file.
Directory Navigation
Command History

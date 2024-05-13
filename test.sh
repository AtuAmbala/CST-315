#!/bin/bash

# Display the first line of Project1.txt
echo "First line of Project1.txt:"
head -n 1 Project1.txt

# Display the last two lines of Project1_copy_2.txt
echo "Last two lines of Project1_copy_2.txt:"
tail -n 2 Project1_copy_2.txt

# Display the disk usage of the current directory
echo "Disk usage of the current directory:"
du -h

# Find all .txt files in the current directory and its subdirectories
echo "All .txt files in the current directory and its subdirectories:"
find . -name "*.txt"

# Count the number of lines in Project1.txt
echo "Number of lines in Project1.txt:"
wc -l Project1.txt

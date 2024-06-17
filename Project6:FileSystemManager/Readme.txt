
## File Management Shell

### Introduction

This project implements a custom shell with a hierarchical file management system, similar to Unix/Linux.   
The shell allows users to perform a variety of file and directory operations interactively.  
It supports 13 different commands, including creating, renaming, deleting, moving, duplicating files and directories,   
searching for files, displaying directory trees, and retrieving file and directory information.

### Software Requirements

- **Operating System:** Unix/Linux  
- **Compiler:** GCC (GNU Compiler Collection)  
- **Libraries:** Standard C libraries (`stdio.h`, `stdlib.h`, `string.h`, `unistd.h`, `sys/types.h`, `sys/stat.h`, `dirent.h`, `time.h`)  
- **Development Environment:** Any Unix/Linux terminal or integrated development environment (IDE) that supports C programming  

### Instructions for Running the Program

1. **Clone the Repository:**

   ```sh
   git clone https://github.com/AtuAmbala/CST-315/tree/main/Project6%3AFileSystemManager.git
   cd Project6:FileSystemManager
   ```

2. **Compile the Program:**

   Use the GCC compiler to compile the source code.

   ```sh
   gcc lopeShell.c -o lopeShell
   ```

3. **Run the Shell:**

   Start the shell by executing the compiled binary.

   ```sh
   ./lopeShell
   ```

4. **Interactive Mode:**

   The shell operates in interactive mode by default. You can enter the following commands:

   - **Create Directory:**
     ```sh
     create dir <directory_path>
     ```

   - **Rename Directory:**
     ```sh
     rename dir <current_directory_path> <new_directory_path>
     ```

   - **Delete Directory:**
     ```sh
     delete dir <directory_path>
     ```

   - **Create File:**
     ```sh
     create file <file_path> <size_in_bytes>
     ```

   - **Rename File:**
     ```sh
     rename file <current_file_path> <new_file_path>
     ```

   - **Delete File:**
     ```sh
     delete file <file_path>
     ```

   - **Move File:**
     ```sh
     move file <source_file_path> <destination_directory_path>
     ```

   - **Duplicate File:**
     ```sh
     duplicate file <source_file_path> <destination_file_path>
     ```

   - **Duplicate Directory:**
     ```sh
     duplicate dir <source_directory_path> <destination_directory_path>
     ```

   - **Search for a File:**
     ```sh
     search file <directory_path> <file_name>
     ```

   - **Display Directory Tree:**
     ```sh
     list recursive <directory_path>
     ```

   - **Get Basic Information About a File:**
     ```sh
     info file <file_path>
     ```

   - **Get Detailed Information About a File:**
     ```sh
     info file <file_path> --detailed
     ```

   - **Get Basic Information About a Directory:**
     ```sh
     info dir <directory_path>
     ```

   - **Get Detailed Information About a Directory:**
     ```sh
     info dir <directory_path> --detailed
     ```

5. **Batch Mode:**

   To run the shell in batch mode, create a text file containing the commands to be executed, one per line. Then start the shell with the filename as an argument.

   ```sh
   ./lopeShell commands.txt
   ```

   Example `commands.txt`:
   ```
   create dir root_directory/documents
   create file root_directory/documents/example.txt 100
   list dir root_directory
   ```

### Example Usage

1. **Creating and Renaming Directories:**

   ```sh
   $lopeShell > create dir root_directory/documents
   $lopeShell > create dir root_directory/media
   $lopeShell > rename dir root_directory/media root_directory/images
   ```

2. **Deleting Directories:**

   ```sh
   $lopeShell > create dir root_directory/temp
   $lopeShell > delete dir root_directory/temp
   ```

3. **Creating, Renaming, and Deleting Files:**

   ```sh
   $lopeShell > create file root_directory/documents/example.txt 100
   $lopeShell > rename file root_directory/documents/example.txt root_directory/documents/sample.txt
   $lopeShell > delete file root_directory/documents/sample.txt
   ```

4. **Moving and Duplicating Files:**

   ```sh
   $lopeShell > move file root_directory/documents/example.txt root_directory/images/example.txt
   $lopeShell > duplicate file root_directory/images/example.txt root_directory/images/example_copy.txt
   ```

5. **Searching and Displaying Directory Trees:**

   ```sh
   $lopeShell > search file root_directory example.txt
   $lopeShell > list recursive root_directory
   ```

6. **Retrieving File and Directory Information:**

   ```sh
   $lopeShell > info file root_directory/images/example.txt
   $lopeShell > info dir root_directory/images
   $lopeShell > info file root_directory/images/example.txt --detailed
   $lopeShell > info dir root_directory/images --detailed
   ```

By following these instructions, you can effectively use the shell to manage files and directories, ensuring an organized and efficient file system.
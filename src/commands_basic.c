#include "cshell.h"

// Help command - Display help information
int cmd_help(char **args) {
    printf("\n");
    printf("Custom-CShell - A custom shell with various features\n");
    printf("----------------------------------------------\n\n");
    
    printf("Built-in commands:\n");
    
    // Display all available commands and their descriptions
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        printf("  %-15s %s\n", builtin_commands[i].name, builtin_commands[i].description);
    }
    
    printf("\nFor more information on a specific command, use: command --help\n\n");
    
    return 1;
}

// Exit command - Exit the shell
int cmd_exit(char **args) {
    shell_running = 0;
    return 0;
}

// Change directory command
int cmd_cd(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: cd [directory]\n");
        printf("Change the current working directory.\n");
        printf("If no directory is provided, changes to the home directory.\n");
        return 1;
    }
    
    if (chdir(args[1]) != 0) {
        perror("cd");
    }
    
    return 1;
}

// Print working directory command
int cmd_pwd(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: pwd\n");
        printf("Prints the current working directory.\n");
        return 1;
    }
    
    char cwd[MAX_PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("pwd");
    }
    
    return 1;
}

// Clear screen command
int cmd_clear(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: clear\n");
        printf("Clears the terminal screen.\n");
        return 1;
    }
    
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
#endif
    
    return 1;
}

// Echo command
int cmd_echo(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: echo [text]\n");
        printf("Display a line of text.\n");
        return 1;
    }
    
    // Start from args[1] to skip the command name
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s", args[i]);
        if (args[i + 1] != NULL) {
            printf(" ");
        }
    }
    printf("\n");
    
    return 1;
}

// List directory command
int cmd_ls(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: ls [directory]\n");
        printf("List the contents of a directory.\n");
        printf("If no directory is provided, lists the current directory.\n");
        return 1;
    }
    
    const char *directory = args[1] ? args[1] : ".";
    
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    char search_path[MAX_PATH_LENGTH];
    
    // Prepare the search path
    snprintf(search_path, sizeof(search_path), "%s\\*", directory);
    
    hFind = FindFirstFile(search_path, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error: Could not open directory %s\n", directory);
        return 1;
    }
    
    printf("\nContents of %s:\n", directory);
    do {
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            printf("%s [DIR]  \n", findData.cFileName);
        } else {
            LARGE_INTEGER fileSize;
            fileSize.LowPart = findData.nFileSizeLow;
            fileSize.HighPart = findData.nFileSizeHigh;
            printf("%s %lld bytes\n", findData.cFileName, fileSize.QuadPart);
        }
    } while (FindNextFile(hFind, &findData));
    
    FindClose(hFind);
#else
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(directory);
    if (dir == NULL) {
        perror("ls");
        return 1;
    }
    
    printf("\nContents of %s:\n", directory);
    
    while ((entry = readdir(dir)) != NULL) {
        char full_path[MAX_PATH_LENGTH];
        struct stat file_stat;
        
        snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
        
        if (stat(full_path, &file_stat) == 0) {
            if (S_ISDIR(file_stat.st_mode)) {
                printf("%s \t[DIR]  \n", entry->d_name);
            } else {
                printf("%s \t%ld bytes\n", entry->d_name, file_stat.st_size);
            }
        } else {
            printf("%s\n", entry->d_name);
        }
    }
    
    closedir(dir);
#endif
    
    printf("\n");
    return 1;
}

// Make directory command
int cmd_mkdir(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: mkdir [directory]\n");
        printf("Create a new directory.\n");
        return 1;
    }
    
#ifdef _WIN32
    if (_mkdir(args[1]) != 0) {
        perror("mkdir");
    } else {
        printf("Created directory: %s\n", args[1]);
    }
#else
    if (mkdir(args[1], 0755) != 0) {
        perror("mkdir");
    } else {
        printf("Created directory: %s\n", args[1]);
    }
#endif
    
    return 1;
}

// Remove file/directory command
int cmd_rm(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: rm [file/directory]\n");
        printf("Remove a file or directory.\n");
        return 1;
    }
    
    // Check if the target exists
    struct stat path_stat;
    if (stat(args[1], &path_stat) != 0) {
        printf("Error: %s does not exist\n", args[1]);
        return 1;
    }
    
#ifdef _WIN32
    if (S_ISDIR(path_stat.st_mode)) {
        if (_rmdir(args[1]) != 0) {
            perror("rmdir");
        } else {
            printf("Removed directory: %s\n", args[1]);
        }
    } else {
        if (remove(args[1]) != 0) {
            perror("remove");
        } else {
            printf("Removed file: %s\n", args[1]);
        }
    }
#else
    if (S_ISDIR(path_stat.st_mode)) {
        if (rmdir(args[1]) != 0) {
            perror("rmdir");
        } else {
            printf("Removed directory: %s\n", args[1]);
        }
    } else {
        if (unlink(args[1]) != 0) {
            perror("unlink");
        } else {
            printf("Removed file: %s\n", args[1]);
        }
    }
#endif
    
    return 1;
}

// Cat command - Display file content
int cmd_cat(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: cat [file]\n");
        printf("Display the contents of a file.\n");
        return 1;
    }
    
    FILE *file = fopen(args[1], "r");
    if (file == NULL) {
        perror("cat");
        return 1;
    }
    
    char buffer[MAX_LINE_LENGTH];
    
    printf("\n");
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }
    printf("\n");
    
    fclose(file);
    return 1;
}

// Count files command - Count files in current directory
int cmd_countfiles(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: countfiles [directory]\n");
        printf("Count the number of files and directories in the specified directory.\n");
        printf("If no directory is provided, counts files in the current directory.\n");
        return 1;
    }
    
    const char *directory = args[1] ? args[1] : ".";
    int file_count = 0;
    int dir_count = 0;
    
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind;
    char search_path[MAX_PATH_LENGTH];
    
    // Prepare the search path
    snprintf(search_path, sizeof(search_path), "%s\\*", directory);
    
    hFind = FindFirstFile(search_path, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Error: Could not open directory %s\n", directory);
        return 1;
    }
    
    do {
        // Skip "." and ".." directories
        if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                dir_count++;
            } else {
                file_count++;
            }
        }
    } while (FindNextFile(hFind, &findData));
    
    FindClose(hFind);
#else
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(directory);
    if (dir == NULL) {
        perror("countfiles");
        return 1;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." directories
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char full_path[MAX_PATH_LENGTH];
            struct stat file_stat;
            
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
            
            if (stat(full_path, &file_stat) == 0) {
                if (S_ISDIR(file_stat.st_mode)) {
                    dir_count++;
                } else {
                    file_count++;
                }
            }
        }
    }
    
    closedir(dir);
#endif
    
    printf("\nIn %s:\n", directory);
    printf("Files: %d\n", file_count);
    printf("Directories: %d\n", dir_count);
    printf("Total items: %d\n\n", file_count + dir_count);
    
    return 1;
}

// Uptime command - Show system uptime
int cmd_uptime(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: uptime\n");
        printf("Display system uptime information.\n");
        return 1;
    }
    
    printf("\n");
    
#ifdef _WIN32
    // Windows uptime implementation
    ULONGLONG uptime_ms = GetTickCount64();
    DWORD uptime_sec = uptime_ms / 1000;
    
    int days = uptime_sec / (60 * 60 * 24);
    int hours = (uptime_sec % (60 * 60 * 24)) / (60 * 60);
    int minutes = (uptime_sec % (60 * 60)) / 60;
    int seconds = uptime_sec % 60;
    
    printf("System uptime: %d days, %d hours, %d minutes, %d seconds\n", days, hours, minutes, seconds);
#else
    // Unix uptime implementation using the uptime command
    FILE *pipe = popen("uptime", "r");
    if (pipe != NULL) {
        char buffer[MAX_LINE_LENGTH];
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            printf("%s", buffer);
        }
        pclose(pipe);
    } else {
        printf("Error: Could not get uptime information\n");
    }
#endif
    
    printf("\n");
    return 1;
}

// Processes command - Display running processes
int cmd_processes(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: processes\n");
        printf("Display information about running processes.\n");
        return 1;
    }
    
    printf("\nRunning Processes:\n");
    
#ifdef _WIN32
    // Windows implementation using tasklist
    system("tasklist | findstr /v \"System Idle Process\" | head -20");
#else
    // Unix implementation using ps
    FILE *pipe = popen("ps aux | head -20", "r");
    if (pipe != NULL) {
        char buffer[MAX_LINE_LENGTH];
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            printf("%s", buffer);
        }
        pclose(pipe);
    } else {
        printf("Error: Could not get process information\n");
    }
#endif
    
    printf("\nNote: Showing only first 20 processes. For more details, use your system's process viewer.\n\n");
    return 1;
}

// Memory command - Display memory usage
int cmd_memory(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: memory\n");
        printf("Display system memory usage information.\n");
        return 1;
    }
    
    printf("\nMemory Usage:\n");
    
#ifdef _WIN32
    // Windows implementation
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    
    if (GlobalMemoryStatusEx(&memInfo)) {
        double total_gb = (double)memInfo.ullTotalPhys / (1024 * 1024 * 1024);
        double used_gb = (double)(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024 * 1024 * 1024);
        double free_gb = (double)memInfo.ullAvailPhys / (1024 * 1024 * 1024);
        
        printf("Total Memory: %.2f GB\n", total_gb);
        printf("Used Memory:  %.2f GB (%.1f%%)\n", used_gb, (used_gb / total_gb) * 100);
        printf("Free Memory:  %.2f GB\n", free_gb);
    } else {
        printf("Error: Could not get memory information\n");
    }
#else
    // Unix implementation using free command
    FILE *pipe = popen("free -h", "r");
    if (pipe != NULL) {
        char buffer[MAX_LINE_LENGTH];
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            printf("%s", buffer);
        }
        pclose(pipe);
    } else {
        printf("Error: Could not get memory information\n");
    }
#endif
    
    printf("\n");
    return 1;
}

// Thank you message command
int cmd_thankyou(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: thankyou [name]\n");
        printf("Display a thank you message. If a name is provided, it will be personalized.\n");
        return 1;
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║                                                          ║\n");
    
    if (args[1] != NULL) {
        // Center the message with the name
        int name_len = strlen(args[1]);
        int spaces = (48 - name_len - 15) / 2; // 15 is the length of "Thank you, !"
        
        printf("║");
        for (int i = 0; i < spaces; i++) printf(" ");
        printf("Thank you, %s!", args[1]);
        for (int i = 0; i < spaces + ((48 - name_len - 15) % 2); i++) printf(" ");
        printf("║\n");
    } else {
        printf("║                  Thank you for using                  ║\n");
        printf("║                  Custom-CShell v1.0!                  ║\n");
    }
    
    printf("║                                                          ║\n");
    printf("║         Your support is greatly appreciated!             ║\n");
    printf("║                                                          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return 1;
}

// Open file explorer command
int cmd_explorer(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: explorer [directory]\n");
        printf("Open file explorer in the specified directory.\n");
        printf("If no directory is provided, opens in the current directory.\n");
        return 1;
    }
    
    // Get the directory to open
    char path[MAX_PATH_LENGTH];
    if (args[1] != NULL) {
        strcpy(path, args[1]);
    } else {
        if (getcwd(path, sizeof(path)) == NULL) {
            perror("explorer");
            return 1;
        }
    }
    
    printf("Opening file explorer in: %s\n", path);
    
#ifdef _WIN32
    // Windows implementation
    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "explorer \"%s\"", path);
    system(command);
#else
    // Check if running in WSL
    FILE *proc = fopen("/proc/version", "r");
    char version[MAX_LINE_LENGTH];
    int is_wsl = 0;
    
    if (proc) {
        if (fgets(version, sizeof(version), proc)) {
            if (strstr(version, "Microsoft") || strstr(version, "WSL")) {
                is_wsl = 1;
            }
        }
        fclose(proc);
    }
    
    char command[MAX_COMMAND_LENGTH];
    
    if (is_wsl) {
        // WSL implementation using explorer.exe
        snprintf(command, sizeof(command), "explorer.exe $(wslpath -w \"%s\")", path);
        system(command);
    } else {
        // Try common file managers
        const char *file_managers[] = {
            "nautilus", "dolphin", "thunar", "pcmanfm", "nemo", "caja", NULL
        };
        
        int success = 0;
        for (int i = 0; file_managers[i] != NULL; i++) {
            snprintf(command, sizeof(command), "which %s >/dev/null 2>&1", file_managers[i]);
            if (system(command) == 0) {
                snprintf(command, sizeof(command), "%s \"%s\" &", file_managers[i], path);
                system(command);
                success = 1;
                break;
            }
        }
        
        if (!success) {
            printf("Error: Could not find a file manager to open\n");
        }
    }
#endif
    
    return 1;
} 
#include "cshell.h"

// Global variables
TodoItem todo_list[MAX_TODO_ITEMS];
int todo_count = 0;
Note notes[MAX_NOTES];
int note_count = 0;
Reminder reminders[MAX_REMINDERS];
int reminder_count = 0;
int shell_running = 1;
int debug_mode = DEBUG_OFF; // Default debug mode is off
char shell_directory[MAX_PATH_LENGTH];
char *command_history[MAX_HISTORY] = {NULL};
int history_count = 0;
int history_position = 0;

// Built-in command array
BuiltinCommand builtin_commands[] = {
    {"help", cmd_help, "Display help information"},
    {"exit", cmd_exit, "Exit the shell"},
    {"quit", cmd_exit, "Exit the shell"},
    {"cd", cmd_cd, "Change directory"},
    {"pwd", cmd_pwd, "Print working directory"},
    {"clear", cmd_clear, "Clear the screen"},
    {"echo", cmd_echo, "Echo a message"},
    {"ls", cmd_ls, "List directory contents"},
    {"mkdir", cmd_mkdir, "Create a directory"},
    {"rm", cmd_rm, "Remove a file or directory"},
    {"cat", cmd_cat, "Display file content"},
    {"history", cmd_history, "Display command history"},
    
    // Feature commands
    {"todo", cmd_todo, "Manage a to-do list"},
    {"note", cmd_note, "Create and manage notes"},
    {"weather", cmd_weather, "Display weather information"},
    {"timer", cmd_timer, "Set a timer"},
    {"reminder", cmd_reminder, "Set and manage reminders"},
    {"quote", cmd_quote, "Display a random quote"},
    {"search", cmd_search, "Search the web"},
    {"news", cmd_news, "Display news headlines"},
    {"joke", cmd_joke, "Display a random joke"},
    {"ascii", cmd_ascii, "Generate ASCII art"},
    {"sysinfo", cmd_sysinfo, "Display system information"},
    {"meme", cmd_meme, "Fetch a random meme"},
    {"wordcount", cmd_wordcount, "Count words in text"},
    {"mathquiz", cmd_mathquiz, "Take a math quiz"},
    {"dayfact", cmd_dayfact, "Display a fact about today"},
    {"colorize", cmd_colorize, "Colorize text output"},
    {"debug", cmd_debug, "Toggle debug mode"},
    
    // New custom commands
    {"countfiles", cmd_countfiles, "Count files in directory"},
    {"uptime", cmd_uptime, "Show system uptime"},
    {"processes", cmd_processes, "Display running processes"},
    {"memory", cmd_memory, "Show memory usage information"},
    {"thankyou", cmd_thankyou, "Display a thank you message"},
    {"explorer", cmd_explorer, "Open file explorer"},
    
    {NULL, NULL, NULL} // End marker
};

// Debug command - Toggle debug mode
int cmd_debug(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: debug [on|off]\n");
        printf("Toggle debug mode for the shell.\n");
        printf("Without arguments, toggles the current state.\n");
        return 1;
    }
    
    if (args[1] != NULL) {
        if (strcmp(args[1], "on") == 0) {
            debug_mode = DEBUG_ON;
            printf("Debug mode enabled\n");
        } else if (strcmp(args[1], "off") == 0) {
            debug_mode = DEBUG_OFF;
            printf("Debug mode disabled\n");
        } else {
            printf("Unknown option: %s\n", args[1]);
            printf("Usage: debug [on|off]\n");
        }
    } else {
        // Toggle debug mode
        debug_mode = !debug_mode;
        printf("Debug mode %s\n", debug_mode ? "enabled" : "disabled");
    }
    
    return 1;
}

// Main function
int main(void) {
    // Initialize shell
    init_shell();
    
    // Run the shell
    run_shell();
    
    // Cleanup before exit
    cleanup_shell();
    
    return 0;
}

// Initialize the shell
void init_shell(void) {
    // Save the shell directory
    getcwd(shell_directory, MAX_PATH_LENGTH);
    
    // Set up signal handling
    signal(SIGINT, signal_handler);
    
    // Initialize the todo list, notes, and reminders
    todo_count = 0;
    note_count = 0;
    reminder_count = 0;
    
    // Load history from file
    load_history();
    
    printf(COLOR_CYAN "\n");
    printf(" ------------------------------------------\n");
    printf("|                                          |\n");
    printf("|          Welcome to Custom CShell        |\n");
    printf("|                 Version 1.0              |\n");
    printf("|                                          |\n");
    printf(" ------------------------------------------\n");
    printf("\n" COLOR_RESET);
    
    // Print the prompt
    printf(COLOR_GREEN "cshell> " COLOR_RESET);
}

// Main shell loop
void run_shell(void) {
    char *line;
    
    while (shell_running) {
        // Get input with history support
        line = get_input_with_history();
        
        if (line == NULL) {
            continue;
        }
        
        // Process the command
        process_command(line);
        
        // Free the line
        free(line);
    }
}

// Add command to history
void add_to_history(const char *command) {
    // Don't add empty commands or duplicates of the last command
    if (command[0] == '\0' || 
        (history_count > 0 && strcmp(command, command_history[history_count - 1]) == 0)) {
        return;
    }
    
    // If history is full, remove the oldest entry
    if (history_count >= MAX_HISTORY) {
        free(command_history[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++) {
            command_history[i] = command_history[i + 1];
        }
        history_count--;
    }
    
    // Add the command to history
    command_history[history_count] = strdup(command);
    history_count++;
    history_position = history_count;
    
    // Save history to file
    save_history();
}

// Save command history to file
void save_history(void) {
    char history_path[MAX_PATH_LENGTH];
    
    // Create the history file path in the user's home directory
#ifdef _WIN32
    char *home_dir = getenv("USERPROFILE");
#else
    char *home_dir = getenv("HOME");
#endif
    
    if (home_dir == NULL) {
        if (debug_mode) printf("Error: Could not get home directory\n");
        return;
    }
    
    snprintf(history_path, sizeof(history_path), "%s/.cshell_history", home_dir);
    
    FILE *history_file = fopen(history_path, "w");
    if (history_file == NULL) {
        if (debug_mode) printf("Error: Could not save history to %s\n", history_path);
        return;
    }
    
    for (int i = 0; i < history_count; i++) {
        fprintf(history_file, "%s\n", command_history[i]);
    }
    
    fclose(history_file);
}

// Load command history from file
void load_history(void) {
    char history_path[MAX_PATH_LENGTH];
    
    // Create the history file path in the user's home directory
#ifdef _WIN32
    char *home_dir = getenv("USERPROFILE");
#else
    char *home_dir = getenv("HOME");
#endif
    
    if (home_dir == NULL) {
        if (debug_mode) printf("Error: Could not get home directory\n");
        return;
    }
    
    snprintf(history_path, sizeof(history_path), "%s/.cshell_history", home_dir);
    
    FILE *history_file = fopen(history_path, "r");
    if (history_file == NULL) {
        // It's okay if the file doesn't exist yet
        return;
    }
    
    char buffer[MAX_COMMAND_LENGTH];
    while (fgets(buffer, sizeof(buffer), history_file) != NULL && history_count < MAX_HISTORY) {
        // Remove newline character
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        
        // Add to history array
        command_history[history_count] = strdup(buffer);
        history_count++;
    }
    
    history_position = history_count;
    
    fclose(history_file);
}

// Get completions for tab completion
char **get_completions(const char *partial_cmd) {
    // Allocate array for completions (max number of completions + NULL terminator)
    char **completions = malloc(sizeof(char*) * (MAX_ARGS + 1));
    if (completions == NULL) return NULL;
    
    int count = 0;
    
    // Check for built-in commands
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (strncmp(builtin_commands[i].name, partial_cmd, strlen(partial_cmd)) == 0) {
            completions[count++] = strdup(builtin_commands[i].name);
        }
    }
    
    // TODO: Add completion for filenames and directories if needed
    
    // NULL terminate the array
    completions[count] = NULL;
    
    return completions;
}

// Free memory used by completions
void free_completions(char **completions) {
    if (completions == NULL) return;
    
    for (int i = 0; completions[i] != NULL; i++) {
        free(completions[i]);
    }
    
    free(completions);
}

// Handle tab completion
int handle_tab_completion(char *input, int *position) {
    // Save the current position
    int pos = *position;
    
    // If the input is empty, do nothing
    if (pos == 0) return 0;
    
    // Get the partial command
    char partial_cmd[MAX_COMMAND_LENGTH];
    strncpy(partial_cmd, input, pos);
    partial_cmd[pos] = '\0';
    
    // Get completions
    char **completions = get_completions(partial_cmd);
    if (completions == NULL || completions[0] == NULL) {
        free_completions(completions);
        return 0;
    }
    
    // If there's only one completion, use it
    if (completions[1] == NULL) {
        // Clear the input line
        printf("\r" COLOR_GREEN "cshell> " COLOR_RESET);
        for (int i = 0; i < strlen(input); i++) {
            printf(" ");
        }
        
        // Copy the completion to the input
        strcpy(input, completions[0]);
        *position = strlen(input);
        
        // Redisplay the input
        printf("\r" COLOR_GREEN "cshell> " COLOR_RESET "%s", input);
    } else {
        // Display all completions
        printf("\n");
        for (int i = 0; completions[i] != NULL; i++) {
            printf("%s  ", completions[i]);
        }
        printf("\n" COLOR_GREEN "cshell> " COLOR_RESET "%s", input);
    }
    
    free_completions(completions);
    return 1;
}

// Display command history
void print_history(void) {
    printf("\nCommand History:\n");
    for (int i = 0; i < history_count; i++) {
        printf("%3d  %s\n", i + 1, command_history[i]);
    }
    printf("\n");
}

// History command implementation
int cmd_history(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: history\n");
        printf("Display the command history.\n");
        return 1;
    }
    
    print_history();
    return 1;
}

// Get input with history and tab completion support
char *get_input_with_history(void) {
    char *input = malloc(MAX_COMMAND_LENGTH);
    if (input == NULL) return NULL;
    
    input[0] = '\0';  // Empty string
    int position = 0;
    
#ifdef _WIN32
    // Windows implementation
    int ch;
    while ((ch = _getch()) != '\r') {  // '\r' is Enter key on Windows
        if (ch == 224 || ch == 0) {  // Special key prefix
            ch = _getch();  // Get the actual key code
            
            // Handle arrow keys
            if (ch == 72) {  // Up arrow
                if (history_position > 0) {
                    history_position--;
                    
                    // Clear the current line
                    printf("\r" COLOR_GREEN "cshell> " COLOR_RESET);
                    for (int i = 0; i < strlen(input); i++) {
                        printf(" ");
                    }
                    
                    // Copy the command from history
                    strcpy(input, command_history[history_position]);
                    position = strlen(input);
                    
                    // Redisplay the line
                    printf("\r" COLOR_GREEN "cshell> " COLOR_RESET "%s", input);
                }
            } else if (ch == 80) {  // Down arrow
                if (history_position < history_count) {
                    history_position++;
                    
                    // Clear the current line
                    printf("\r" COLOR_GREEN "cshell> " COLOR_RESET);
                    for (int i = 0; i < strlen(input); i++) {
                        printf(" ");
                    }
                    
                    // If at the end of history, clear the line
                    if (history_position == history_count) {
                        input[0] = '\0';
                        position = 0;
                    } else {
                        // Copy the command from history
                        strcpy(input, command_history[history_position]);
                        position = strlen(input);
                    }
                    
                    // Redisplay the line
                    printf("\r" COLOR_GREEN "cshell> " COLOR_RESET "%s", input);
                }
            }
        } else if (ch == '\b' || ch == 127) {  // Backspace
            if (position > 0) {
                input[--position] = '\0';
                printf("\b \b");  // Erase character on screen
            }
        } else if (ch == KEY_TAB) {  // Tab for completion
            handle_tab_completion(input, &position);
        } else if (ch >= 32 && ch <= 126) {  // Printable characters
            if (position < MAX_COMMAND_LENGTH - 1) {
                input[position++] = ch;
                input[position] = '\0';
                printf("%c", ch);
            }
        }
    }
    
    printf("\n");
#else
    // Unix implementation
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= ~(ICANON | ECHO);  // Turn off canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    
    int ch;
    while ((ch = getchar()) != '\n') {
        if (ch == KEY_ESCAPE) {
            // Handle escape sequences for arrow keys
            if (getchar() == '[') {
                ch = getchar();
                
                if (ch == KEY_UP) {  // Up arrow
                    if (history_position > 0) {
                        history_position--;
                        
                        // Clear the current line
                        printf("\r" COLOR_GREEN "cshell> " COLOR_RESET);
                        for (int i = 0; i < strlen(input); i++) {
                            printf(" ");
                        }
                        
                        // Copy the command from history
                        strcpy(input, command_history[history_position]);
                        position = strlen(input);
                        
                        // Redisplay the line
                        printf("\r" COLOR_GREEN "cshell> " COLOR_RESET "%s", input);
                    }
                } else if (ch == KEY_DOWN) {  // Down arrow
                    if (history_position < history_count) {
                        history_position++;
                        
                        // Clear the current line
                        printf("\r" COLOR_GREEN "cshell> " COLOR_RESET);
                        for (int i = 0; i < strlen(input); i++) {
                            printf(" ");
                        }
                        
                        // If at the end of history, clear the line
                        if (history_position == history_count) {
                            input[0] = '\0';
                            position = 0;
                        } else {
                            // Copy the command from history
                            strcpy(input, command_history[history_position]);
                            position = strlen(input);
                        }
                        
                        // Redisplay the line
                        printf("\r" COLOR_GREEN "cshell> " COLOR_RESET "%s", input);
                    }
                }
            }
        } else if (ch == KEY_BACKSPACE) {  // Backspace
            if (position > 0) {
                input[--position] = '\0';
                printf("\b \b");  // Erase character on screen
            }
        } else if (ch == KEY_TAB) {  // Tab for completion
            handle_tab_completion(input, &position);
        } else if (ch >= 32 && ch <= 126) {  // Printable characters
            if (position < MAX_COMMAND_LENGTH - 1) {
                input[position++] = ch;
                input[position] = '\0';
                printf("%c", ch);
            }
        }
    }
    
    // Restore terminal settings
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    printf("\n");
#endif
    
    // Add command to history if not empty
    if (strlen(input) > 0) {
        add_to_history(input);
    }
    
    return input;
}

// Process a command
void process_command(char *command) {
    if (debug_mode) printf(COLOR_YELLOW "Debug: Processing command: %s\n" COLOR_RESET, command);
    
    // Parse the command and arguments
    char *args[MAX_ARGS];
    char *token;
    int arg_count = 0;
    
    token = strtok(command, " \t\n");
    while (token != NULL && arg_count < MAX_ARGS - 1) {
        args[arg_count++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[arg_count] = NULL;
    
    // If no command was entered, just return
    if (arg_count == 0) {
        printf(COLOR_GREEN "cshell> " COLOR_RESET);
        return;
    }
    
    // Execute the command
    execute_command(args);
    
    // Print the prompt
    printf(COLOR_GREEN "cshell> " COLOR_RESET);
}

// Execute a command
void execute_command(char **args) {
    if (debug_mode) printf(COLOR_YELLOW "Debug: Executing command: %s\n" COLOR_RESET, args[0]);
    
    // Check for built-in commands
    for (int i = 0; builtin_commands[i].name != NULL; i++) {
        if (strcmp(args[0], builtin_commands[i].name) == 0) {
            builtin_commands[i].func(args);
            return;
        }
    }
    
    // If not a built-in command, try to execute as an external command
#ifdef _WIN32
    // Windows implementation
    char command[MAX_COMMAND_LENGTH] = "";
    
    // Construct the full command
    for (int i = 0; args[i] != NULL; i++) {
        strcat(command, args[i]);
        if (args[i + 1] != NULL) {
            strcat(command, " ");
        }
    }
    
    int result = system(command);
    
    if (result != 0) {
        printf("Error: Command not found or could not be executed: %s\n", args[0]);
    }
#else
    // Unix implementation
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        // Fork error
        perror("fork");
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("Error: Command not found or could not be executed: %s\n", args[0]);
        }
    }
#endif
}

// Signal handler
void signal_handler(int signo) {
    if (signo == SIGINT) {
        printf("\nUse 'exit' to quit the shell\n");
        printf(COLOR_GREEN "cshell> " COLOR_RESET);
        fflush(stdout);
    }
}

// Clean up resources
void cleanup_shell(void) {
    // Free command history
    for (int i = 0; i < history_count; i++) {
        free(command_history[i]);
    }
    
    printf(COLOR_CYAN "\nThank you for using Custom CShell!\n" COLOR_RESET);
} 
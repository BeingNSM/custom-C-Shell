#ifndef CSHELL_H
#define CSHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <signal.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <direct.h>
    #include <process.h>
    #define chdir _chdir
    #define getcwd _getcwd
    #define mkdir(path, mode) _mkdir(path)
    #define access _access
    #define F_OK 0
    #define PATH_MAX 260
    #define sleep(x) Sleep(x * 1000)
    #include <pthread.h>    // Include pthread for Windows too
    #include <curl/curl.h>  // Include curl for Windows too
    #include <conio.h>      // For _getch() on Windows
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #include <limits.h>
    #include <sys/ioctl.h>
    #include <pthread.h>
    #include <curl/curl.h>
    #include <termios.h>    // For terminal settings on Unix
#endif

// Constants
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64
#define MAX_PATH_LENGTH 256
#define MAX_LINE_LENGTH 1024
#define MAX_TODO_ITEMS 100
#define MAX_NOTES 100
#define MAX_REMINDERS 20
#define MAX_HISTORY 100    // Maximum number of commands to store in history

// Key codes
#define KEY_UP      65      // Up arrow (Unix: 65 after escape sequence)
#define KEY_DOWN    66      // Down arrow
#define KEY_RIGHT   67      // Right arrow
#define KEY_LEFT    68      // Left arrow
#define KEY_TAB     9       // Tab key
#define KEY_ENTER   13      // Enter key
#define KEY_ESCAPE  27      // Escape key
#define KEY_BACKSPACE 127   // Backspace

// Debug mode
#define DEBUG_OFF 0
#define DEBUG_ON 1

// ANSI color codes
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"
#define COLOR_BOLD "\033[1m"

// Built-in command structure
typedef struct {
    char *name;
    int (*func)(char **args);
    char *description;
} BuiltinCommand;

// Data structures
typedef struct {
    char content[MAX_LINE_LENGTH];
    int completed;
} TodoItem;

typedef struct {
    char title[MAX_LINE_LENGTH];
    char category[MAX_LINE_LENGTH];
    char content[MAX_LINE_LENGTH * 10];
    time_t timestamp;
} Note;

typedef struct {
    char message[MAX_LINE_LENGTH];
    time_t timestamp;
    int active;
    pthread_t thread_id;
} Reminder;

typedef struct {
    size_t size;
    char *data;
} ResponseData;

// Function declarations
// Shell core functions
void init_shell(void);
void run_shell(void);
void process_command(char *command);
void execute_command(char **args);
void signal_handler(int signo);
void cleanup_shell(void);

// Built-in commands
int cmd_help(char **args);
int cmd_exit(char **args);
int cmd_cd(char **args);
int cmd_pwd(char **args);
int cmd_clear(char **args);
int cmd_echo(char **args);
int cmd_ls(char **args);
int cmd_mkdir(char **args);
int cmd_rm(char **args);
int cmd_cat(char **args);

// Feature commands
int cmd_todo(char **args);
int cmd_note(char **args);
int cmd_weather(char **args);
int cmd_timer(char **args);
int cmd_reminder(char **args);
int cmd_quote(char **args);
int cmd_search(char **args);
int cmd_news(char **args);
int cmd_joke(char **args);
int cmd_ascii(char **args);
int cmd_sysinfo(char **args);
int cmd_meme(char **args);
int cmd_wordcount(char **args);
int cmd_mathquiz(char **args);
int cmd_dayfact(char **args);
int cmd_colorize(char **args);
int cmd_calculator(char **args);

// New custom commands
int cmd_countfiles(char **args);
int cmd_uptime(char **args);
int cmd_processes(char **args);
int cmd_memory(char **args);
int cmd_thankyou(char **args);
int cmd_explorer(char **args);

// Debug functions
int cmd_debug(char **args);

// History and Tab completion functions
void add_to_history(const char *command);
void save_history(void);
void load_history(void);
char *get_input_with_history(void);
char **get_completions(const char *partial_cmd);
void free_completions(char **completions);
int handle_tab_completion(char *input, int *position);
void print_history(void);
int cmd_history(char **args);

// Utility functions
char *get_input(void);
char **parse_command(char *command);
int count_tokens(char *str, const char *delim);
void print_colored(const char *text, const char *color);
void save_todo_list(void);
void load_todo_list(void);
void save_notes(void);
void load_notes(void);
int check_file_exists(const char *filename);
void create_directory_if_not_exists(const char *dirname);
void trim_whitespace(char *str);
size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp);
int open_url_in_browser(const char *url);
void ensure_data_directory(void);
char* wsl_to_windows_path(const char* wsl_path, char* win_path, size_t win_path_size);
int open_html_in_browser(const char *html_path);
int system_check_command_exists(const char* command);

// Global variables (defined in cshell.c)
extern TodoItem todo_list[MAX_TODO_ITEMS];
extern int todo_count;
extern Note notes[MAX_NOTES];
extern int note_count;
extern Reminder reminders[MAX_REMINDERS];
extern int reminder_count;
extern int shell_running;
extern int debug_mode;
extern char shell_directory[MAX_PATH_LENGTH];
extern BuiltinCommand builtin_commands[];
extern char *command_history[MAX_HISTORY];
extern int history_count;
extern int history_position;

#endif /* CSHELL_H */ 
#include "cshell.h"

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif

// Curl callback function to handle response data
size_t curl_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    ResponseData *response = (ResponseData *)userp;

    // Allocate or resize the memory for the response data
    char *ptr = realloc(response->data, response->size + realsize + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Error: Not enough memory to store response\n");
        return 0;
    }

    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = 0;

    return realsize;
}

// Open a URL in the default browser
int open_url_in_browser(const char *url) {
#ifdef _WIN32
    // Windows implementation - using direct Windows API calls
    // First try ShellExecute if available

    // Try to load the shell32.dll dynamically
    HMODULE shell32 = LoadLibrary("shell32.dll");
    if (shell32) {
        // Get the ShellExecute function pointer
        typedef HINSTANCE (WINAPI *ShellExecuteFunc)(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, INT);
        ShellExecuteFunc shellExecute = (ShellExecuteFunc)GetProcAddress(shell32, "ShellExecuteA");
        
        if (shellExecute) {
            // Use ShellExecute directly
            HINSTANCE result = shellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
            FreeLibrary(shell32);
            
            // Check if successful (value > 32 means success)
            if ((intptr_t)result > 32) {
                return 0;
            }
        }
        
        FreeLibrary(shell32);
    }
    
    // If ShellExecute failed or was not available, try command line methods
    char command[MAX_COMMAND_LENGTH];
    int result = -1;
    
    // Method 1: ShellExecute via CMD
    snprintf(command, sizeof(command), "cmd /c start \"\" \"%s\"", url);
    result = system(command);
    if (result == 0) return 0;
    
    // Method 2: Use rundll32 directly
    snprintf(command, sizeof(command), "rundll32 url.dll,FileProtocolHandler \"%s\"", url);
    result = system(command);
    if (result == 0) return 0;
    
    // Method 3: Use PowerShell
    snprintf(command, sizeof(command), "powershell -command \"Start-Process '%s'\"", url);
    result = system(command);
    if (result == 0) return 0;
    
    // Method 4: Try explorer directly
    snprintf(command, sizeof(command), "explorer \"%s\"", url);
    result = system(command);
    
    // If all methods fail, create a simple HTML file that redirects to the URL
    if (result != 0) {
        // Create in the current directory
        FILE *html_file = fopen("open_url.html", "w");
        if (html_file) {
            fprintf(html_file, "<html><head><meta http-equiv=\"refresh\" content=\"0; url=%s\"></head><body><p>Redirecting to %s...</p><p><a href=\"%s\">Click here if not redirected</a></p></body></html>", url, url, url);
            fclose(html_file);
            printf("Created HTML file 'open_url.html' - please open this file manually.\n");
            
            // Try to open the HTML file directly
            snprintf(command, sizeof(command), "cmd /c start \"\" \"open_url.html\"");
            system(command);
        }
    }
    
    return result;
#else
    // Linux/macOS implementation
    char command[MAX_COMMAND_LENGTH];
    
    // Check if we're in WSL
    int is_wsl = 0;
    FILE *proc = fopen("/proc/version", "r");
    if (proc) {
        char version[MAX_LINE_LENGTH];
        if (fgets(version, sizeof(version), proc) && 
            (strstr(version, "Microsoft") || strstr(version, "WSL"))) {
            is_wsl = 1;
        }
        fclose(proc);
    }
    
    if (is_wsl) {
        // WSL-specific methods to open browser in Windows
        
        // Create a properly escaped URL for different shells
        char escaped_url[MAX_COMMAND_LENGTH * 2];
        
        // Replace single quotes with escaped version for PowerShell
        int j = 0;
        for (int i = 0; url[i] != '\0' && j < MAX_COMMAND_LENGTH * 2 - 1; i++) {
            if (url[i] == '\'') {
                escaped_url[j++] = '\'';
                escaped_url[j++] = '\'';
            } else {
                escaped_url[j++] = url[i];
            }
        }
        escaped_url[j] = '\0';
        
        // Method 1: Try wslview (part of wslu package) - most direct for browser opening
        if (system_check_command_exists("wslview")) {
            snprintf(command, sizeof(command), "wslview \"%s\"", url);
            int result = system(command);
            if (result == 0) return 0;
        }
        
        // Method 2: Use cmd.exe directly with start command (most reliable)
        snprintf(command, sizeof(command), "cmd.exe /c start \"\" \"%s\"", url);
        int result = system(command);
        if (result == 0) return 0;
        
        // Method 3: Try PowerShell with double quotes (browser-specific)
        snprintf(command, sizeof(command), "powershell.exe -Command \"Start-Process \\\"%s\\\"\"", url);
        result = system(command);
        if (result == 0) return 0;
        
        // Method 4: Use explorer.exe only as a last resort
        snprintf(command, sizeof(command), "explorer.exe \"%s\"", url);
        result = system(command);
        if (result == 0) return 0;
        
        return -1;  // All methods failed
    }
    
    // Standard Linux browser opening methods
    const char *browsers[] = {
        "xdg-open",
        "open",
        "firefox",
        "google-chrome",
        "chromium-browser",
        NULL
    };
    
    for (int i = 0; browsers[i] != NULL; i++) {
        snprintf(command, sizeof(command), "which %s >/dev/null 2>&1", browsers[i]);
        if (system(command) == 0) {
            snprintf(command, sizeof(command), "%s '%s' &", browsers[i], url);
            return system(command);
        }
    }
    
    fprintf(stderr, "Error: No supported browser found\n");
    return -1;
#endif
}

// Helper function to ensure data directory exists
void ensure_data_directory() {
    // Create the data directory if it doesn't exist
#ifdef _WIN32
    if (_access("data", 0) != 0) {
        _mkdir("data");
    }
#else
    // For Unix/Linux/WSL
    struct stat st = {0};
    if (stat("data", &st) == -1) {
        system("mkdir -p data");
        system("chmod 755 data");
    }
#endif
}

// Convert a WSL path to a Windows path if possible
char* wsl_to_windows_path(const char* wsl_path, char* win_path, size_t win_path_size) {
    if (!win_path || win_path_size <= 0 || !wsl_path) {
        return NULL;
    }
    
    // Default to the input path
    strncpy(win_path, wsl_path, win_path_size - 1);
    win_path[win_path_size - 1] = '\0';
    
#ifndef _WIN32
    // Check if we're in WSL
    FILE *proc = fopen("/proc/version", "r");
    if (proc) {
        char version[MAX_LINE_LENGTH];
        if (fgets(version, sizeof(version), proc) && 
            (strstr(version, "Microsoft") || strstr(version, "WSL"))) {
            
            // Try to get Windows path using wslpath
            if (system("which wslpath > /dev/null 2>&1") == 0) {
                char command[MAX_COMMAND_LENGTH];
                snprintf(command, sizeof(command), "wslpath -w \"%s\"", wsl_path);
                
                FILE *path_pipe = popen(command, "r");
                if (path_pipe) {
                    char temp_path[MAX_PATH_LENGTH] = {0};
                    if (fgets(temp_path, sizeof(temp_path), path_pipe)) {
                        // Remove newline if present
                        char *newline = strchr(temp_path, '\n');
                        if (newline) *newline = '\0';
                        
                        // Copy to output path
                        strncpy(win_path, temp_path, win_path_size - 1);
                        win_path[win_path_size - 1] = '\0';
                    }
                    pclose(path_pipe);
                }
            }
        }
        fclose(proc);
    }
#endif
    
    return win_path;
}

// Open an HTML file in the default browser
int open_html_in_browser(const char *html_path) {
    ensure_data_directory();
    
    char command[MAX_COMMAND_LENGTH];
    char win_path[MAX_PATH_LENGTH];
    
#ifdef _WIN32
    // Windows implementation
    snprintf(command, sizeof(command), "start \"\" \"%s\"", html_path);
    return system(command);
#else
    // Check if we're in WSL
    int is_wsl = 0;
    FILE *proc = fopen("/proc/version", "r");
    if (proc) {
        char version[MAX_LINE_LENGTH];
        if (fgets(version, sizeof(version), proc) && 
            (strstr(version, "Microsoft") || strstr(version, "WSL"))) {
            is_wsl = 1;
        }
        fclose(proc);
    }
    
    if (is_wsl) {
        // Get Windows path for html_file
        wsl_to_windows_path(html_path, win_path, sizeof(win_path));
        
        // Try wslview first (most reliable for browser opening)
        if (system_check_command_exists("wslview")) {
            snprintf(command, sizeof(command), "wslview \"%s\"", html_path);
            if (system(command) == 0) {
                return 0;
            }
        }
        
        // Try cmd.exe with start command 
        snprintf(command, sizeof(command), "cmd.exe /c start \"\" \"%s\"", html_path);
        if (system(command) == 0) {
            return 0;
        }
        
        // Try PowerShell
        snprintf(command, sizeof(command), "powershell.exe -Command \"Start-Process \\\"%s\\\"\"", html_path);
        if (system(command) == 0) {
            return 0;
        }
        
        // Only use explorer.exe as a last resort, as it opens File Explorer 
        snprintf(command, sizeof(command), "explorer.exe \"%s\"", html_path);
        if (system(command) == 0) {
            return 0;
        }
        
        return -1;
    } else {
        // Standard Linux/Mac browser opening
        const char *browsers[] = {
            "xdg-open",
            "open",
            "firefox",
            "google-chrome",
            "chromium-browser",
            NULL
        };
        
        for (int i = 0; browsers[i] != NULL; i++) {
            char check_cmd[MAX_COMMAND_LENGTH];
            snprintf(check_cmd, sizeof(check_cmd), "which %s > /dev/null 2>&1", browsers[i]);
            if (system(check_cmd) == 0) {
                snprintf(command, sizeof(command), "%s '%s' &", browsers[i], html_path);
                if (system(command) == 0) {
                    return 0;
                }
            }
        }
        
        return -1;
    }
#endif
}

// Trim whitespace from a string
void trim_whitespace(char *str) {
    if (str == NULL) return;
    
    // Trim leading whitespace
    char *start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
    
    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';
}

// Check if a file exists
int check_file_exists(const char *filename) {
    return access(filename, F_OK) == 0;
}

// Create a directory if it doesn't exist
void create_directory_if_not_exists(const char *dirname) {
#ifdef _WIN32
    if (access(dirname, F_OK) != 0) {
        mkdir(dirname);
    }
#else
    if (access(dirname, F_OK) != 0) {
        mkdir(dirname, 0755);
    }
#endif
}

// Checks if a command exists in the system
int system_check_command_exists(const char* command) {
    char check_cmd[MAX_COMMAND_LENGTH];
    
#ifdef _WIN32
    // Windows implementation
    snprintf(check_cmd, sizeof(check_cmd), "where %s >nul 2>nul", command);
#else
    // Unix implementation
    snprintf(check_cmd, sizeof(check_cmd), "which %s >/dev/null 2>&1", command);
#endif
    
    return (system(check_cmd) == 0);
} 
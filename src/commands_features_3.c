#include "cshell.h"

#ifndef _WIN32
#include <sys/utsname.h>
#endif

#include <ctype.h>  // For isalnum function used in URL encoding

// Web search command - Search the web
int cmd_search(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: search [query]\n");
        printf("Search the web for the specified query.\n");
        return 1;
    }
    
    // Build the search query
    char query[MAX_LINE_LENGTH] = "";
    for (int i = 1; args[i] != NULL; i++) {
        strcat(query, args[i]);
        if (args[i + 1] != NULL) {
            strcat(query, " ");  // Use space for display
        }
    }
    
    // Build URL-friendly query (replace spaces with +)
    char url_query[MAX_LINE_LENGTH * 3] = "";
    char *src = query;
    char *dst = url_query;
    
    while (*src) {
        if (*src == ' ') {
            *dst++ = '+';
        } else if (isalnum((unsigned char)*src) || *src == '-' || *src == '_' || *src == '.' || *src == '~') {
            *dst++ = *src;
        } else {
            sprintf(dst, "%%%02X", (unsigned char)*src);
            dst += 3;
        }
        src++;
    }
    *dst = '\0';
    
    // Build Google search URL
    char search_url[MAX_LINE_LENGTH];
    snprintf(search_url, sizeof(search_url), "https://www.google.com/search?q=%s", url_query);
    
    printf("Searching for: %s\n", query);
    printf("Search URL: %s\n", search_url);
    
    // Ensure data directory exists
    ensure_data_directory();
    
    // Try to open the search URL directly first for a more seamless experience
    printf("\nAttempting to open search results...\n");
    
    // Try direct URL opening first
    int result = open_url_in_browser(search_url);
    
    if (result == 0) {
        printf("Search results should open in your browser.\n");
        return 1;
    }
    
    // If direct URL opening failed, create and use an HTML file as fallback
    // Create an HTML file with auto-redirect to the search URL
    char html_path[MAX_PATH_LENGTH];
    
    // Create simplified HTML file with direct redirect
    snprintf(html_path, sizeof(html_path), "data/search.html");
    FILE *html_file = fopen(html_path, "w");
    if (!html_file) {
        printf("Error: Could not create HTML file.\n");
        printf("Please visit this search URL manually: %s\n", search_url);
        return 1;
    }
    
    // Write a simple HTML file that immediately redirects to search URL
    fprintf(html_file, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(html_file, "<meta http-equiv=\"refresh\" content=\"0; url=%s\">\n", search_url);
    fprintf(html_file, "<title>Redirecting to search results...</title>\n</head>\n");
    fprintf(html_file, "<body>\n<h2>Redirecting to Google search results for: %s</h2>\n", query);
    fprintf(html_file, "<p>If not automatically redirected, <a href=\"%s\">click here</a></p>\n", search_url);
    fprintf(html_file, "<script>window.location.href=\"%s\";</script>\n", search_url);
    fprintf(html_file, "</body>\n</html>");
    fclose(html_file);
    
    // Try to open the HTML file in a browser as fallback
    result = open_html_in_browser(html_path);
    
    if (result == 0) {
        printf("Search results should open in your browser.\n");
        return 1;
    }
    
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
    
    // If open_html_in_browser failed, try direct URL opening methods
    char command[MAX_COMMAND_LENGTH];
    int search_opened = 0;
    
    if (is_wsl) {
        printf("Trying additional methods to open search results...\n");
        
        // Method 1: Direct cmd.exe with start command to URL
        snprintf(command, sizeof(command), "cmd.exe /c start \"\" \"%s\"", search_url);
        if (system(command) == 0) {
            printf("Search results opened using cmd.exe start command.\n");
            search_opened = 1;
        }
        
        // Method 2: Try using wslview if available
        if (!search_opened && system_check_command_exists("wslview")) {
            snprintf(command, sizeof(command), "wslview \"%s\"", search_url);
            if (system(command) == 0) {
                printf("Search results opened using wslview.\n");
                search_opened = 1;
            }
        }
    } else {
        // For Linux/Mac, try standard browser opening methods with direct URL
        const char *browsers[] = {
            "xdg-open",
            "open",
            "firefox",
            "google-chrome",
            "chromium-browser",
            NULL
        };
        
        for (int i = 0; browsers[i] != NULL && !search_opened; i++) {
            if (system_check_command_exists(browsers[i])) {
                snprintf(command, sizeof(command), "%s '%s' &", browsers[i], search_url);
                if (system(command) == 0) {
                    printf("Search results opened in %s.\n", browsers[i]);
                    search_opened = 1;
                }
            }
        }
    }
    
    // If all automatic methods failed, provide manual methods
    if (!search_opened) {
        printf("\nCould not automatically open search results.\n");
        printf("\nMANUAL METHODS TO VIEW SEARCH RESULTS:\n");
        printf("----------------------------------------\n");
        printf("1. Copy and paste this URL in your browser:\n   %s\n\n", search_url);
        
        if (is_wsl) {
            printf("2. WSL commands to try:\n");
            printf("   a. explorer.exe %s\n", html_path);
            printf("   b. cmd.exe /c start \"\" \"%s\"\n", search_url); 
            
            // Create bat file for easy execution
            char bat_path[MAX_PATH_LENGTH];
            snprintf(bat_path, sizeof(bat_path), "data/search_open.bat");
            FILE *bat_file = fopen(bat_path, "w");
            if (bat_file) {
                fprintf(bat_file, "@echo off\n");
                fprintf(bat_file, "start \"\" \"%s\"\n", search_url);
                fclose(bat_file);
                printf("   c. Run this command: cmd.exe /c %s\n", bat_path);
            }
            
            if (!system_check_command_exists("wslview")) {
                printf("\n3. Install WSL browser integration tools:\n");
                printf("   sudo apt update && sudo apt install -y wslu\n");
                printf("   Then use: wslview %s\n", html_path);
            }
        }
    }
    
    return 1;
}

// Helper function to check if a command exists in the system path
// Declared in utility.c
extern int system_check_command_exists(const char* command);

// News command - Display real-time news headlines
int cmd_news(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: news [category]\n");
        printf("Display latest news headlines. Optional categories: technology, business, science, health, world, sports\n");
        return 1;
    }
    
    // News category
    const char *category = "general";
    if (args[1] != NULL) {
        category = args[1];
    }
    
    // Build the URL for fetching news from reliable RSS feeds
    char url[MAX_LINE_LENGTH];
    if (strcmp(category, "technology") == 0) {
        snprintf(url, sizeof(url), "https://feeds.skynews.com/feeds/rss/technology.xml");
    } else if (strcmp(category, "business") == 0) {
        snprintf(url, sizeof(url), "https://feeds.skynews.com/feeds/rss/business.xml");
    } else if (strcmp(category, "science") == 0) {
        snprintf(url, sizeof(url), "https://rss.nytimes.com/services/xml/rss/nyt/Science.xml");
    } else if (strcmp(category, "health") == 0) {
        snprintf(url, sizeof(url), "https://rss.nytimes.com/services/xml/rss/nyt/Health.xml");
    } else if (strcmp(category, "world") == 0) {
        snprintf(url, sizeof(url), "https://feeds.skynews.com/feeds/rss/world.xml");
    } else if (strcmp(category, "sports") == 0) {
        snprintf(url, sizeof(url), "https://feeds.skynews.com/feeds/rss/sports.xml");
    } else {
        snprintf(url, sizeof(url), "https://feeds.skynews.com/feeds/rss/home.xml");
    }
    
    printf("Fetching news from: %s\n", url);
    
    // Initialize CURL
    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Error: Failed to initialize CURL\n");
        return 1;
    }
    
    // Set up response data
    ResponseData resp;
    resp.size = 0;
    resp.data = malloc(1);
    resp.data[0] = '\0';
    
    // Set CURL options
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&resp);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    
    // Check for errors
    if (res != CURLE_OK) {
        printf("Error: %s\n", curl_easy_strerror(res));
        
        // Fall back to fake news data if real fetch fails
        printf("\n");
        printf("Latest %s News (Offline Mode):\n", category);
        printf("-----------------------------\n");
        
        // Fake news headlines based on category
        if (strcmp(category, "technology") == 0) {
            printf("1. New Breakthrough in Quantum Computing Achieved\n");
            printf("2. Tech Giants Announce Collaboration on AI Ethics\n");
            printf("3. Revolutionary Battery Technology Extends Phone Life to One Week\n");
            printf("4. Global Chip Shortage Expected to Ease by Next Quarter\n");
            printf("5. Security Researchers Discover Critical Vulnerability in Popular Software\n");
        } else if (strcmp(category, "business") == 0) {
            printf("1. Stock Markets Reach New All-Time High\n");
            printf("2. Major Merger Announced Between Industry Leaders\n");
            printf("3. Startup Secures Record-Breaking Investment Round\n");
            printf("4. Global Supply Chain Issues Continue to Affect Retail Sector\n");
            printf("5. New Economic Policies Aimed at Reducing Inflation\n");
        } else if (strcmp(category, "science") == 0) {
            printf("1. Astronomers Discover Potentially Habitable Exoplanet\n");
            printf("2. New Species of Deep-Sea Creature Found in Pacific Ocean\n");
            printf("3. Climate Research Indicates Faster Warming Than Previously Predicted\n");
            printf("4. Breakthrough in Renewable Energy Storage Technology\n");
            printf("5. International Space Station Celebrates 25 Years in Orbit\n");
        } else if (strcmp(category, "health") == 0) {
            printf("1. New Treatment Shows Promise for Chronic Condition\n");
            printf("2. Study Reveals Benefits of Mediterranean Diet for Longevity\n");
            printf("3. Mental Health Awareness Campaign Launched Globally\n");
            printf("4. Breakthrough in Early Detection of Common Disease\n");
            printf("5. Exercise Found to Have More Benefits Than Previously Known\n");
        } else {
            printf("1. Global Leaders Meet for Climate Summit\n");
            printf("2. Major Sports Team Wins Championship After Dramatic Comeback\n");
            printf("3. Cultural Festival Attracts Record Number of Visitors\n");
            printf("4. Education Reform Bill Passed After Long Debate\n");
            printf("5. Historic Landmark Reopens After Renovation\n");
        }
    } else {
        // Parse the XML RSS feed to extract headlines
        printf("\n");
        printf("Latest %s News:\n", category);
        printf("---------------\n");
        
        // Save response to file for debugging if needed
        FILE *debug_file = fopen("data/news_debug.xml", "w");
        if (debug_file) {
            fprintf(debug_file, "%s", resp.data);
            fclose(debug_file);
        }
        
        // Various patterns to try for different RSS formats
        const char *title_patterns[] = {
            "<title><![CDATA[", "]]></title>",
            "<title>", "</title>"
        };
        
        // Try multiple parsing methods
        int count = 0;
        int method;
        
        for (method = 0; method < 2 && count == 0; method++) {
            char *ptr = resp.data;
            char *item_ptr = NULL;
            
            // Find the first <item> tag to skip channel title
            item_ptr = strstr(ptr, "<item>");
            
            if (item_ptr != NULL) {
                ptr = item_ptr;
                
                // Now parse each item
                while (count < 10) {
                    char *title_start = strstr(ptr, title_patterns[method*2]);
                    if (!title_start) break;
                    
                    title_start += strlen(title_patterns[method*2]);
                    char *title_end = strstr(title_start, title_patterns[method*2+1]);
                    
                    if (title_end) {
                        size_t title_len = title_end - title_start;
                        
                        if (title_len > 0 && title_len < MAX_LINE_LENGTH - 1) {
                            char title[MAX_LINE_LENGTH] = {0};
                            strncpy(title, title_start, title_len);
                            title[title_len] = '\0';
                            
                            // Special handling for HTML entities
                            char *ampersand;
                            while ((ampersand = strstr(title, "&amp;")) != NULL) {
                                *ampersand = '&';
                                memmove(ampersand + 1, ampersand + 5, strlen(ampersand + 5) + 1);
                            }
                            
                            // Convert HTML entities to special characters
                            char *html_entity;
                            while ((html_entity = strstr(title, "&quot;")) != NULL) {
                                *html_entity = '"';
                                memmove(html_entity + 1, html_entity + 6, strlen(html_entity + 6) + 1);
                            }
                            
                            // Convert &lt; and &gt;
                            while ((html_entity = strstr(title, "&lt;")) != NULL) {
                                *html_entity = '<';
                                memmove(html_entity + 1, html_entity + 4, strlen(html_entity + 4) + 1);
                            }
                            
                            while ((html_entity = strstr(title, "&gt;")) != NULL) {
                                *html_entity = '>';
                                memmove(html_entity + 1, html_entity + 4, strlen(html_entity + 4) + 1);
                            }
                            
                            // Skip if it's a channel title or contains "RSS"
                            if (strstr(title, "RSS") == NULL &&
                                strstr(title, "News") != title &&
                                strstr(title, "Latest") != title) {
                                printf("%d. %s\n", count + 1, title);
                                count++;
                            }
                        }
                        
                        ptr = title_end + strlen(title_patterns[method*2+1]);
                    } else {
                        break;
                    }
                    
                    // Find next item
                    char *next_item = strstr(ptr, "<item>");
                    if (!next_item) break;
                    ptr = next_item;
                }
            }
        }
        
        // If still no results, try a more aggressive approach
        if (count == 0) {
            // Look for item sections first
            char *ptr = resp.data;
            char *item_start;
            
            while ((item_start = strstr(ptr, "<item>")) != NULL && count < 10) {
                char *item_end = strstr(item_start, "</item>");
                if (!item_end) break;
                
                // Extract a complete item section for examination
                size_t item_len = item_end - item_start + 7; // +7 for "</item>"
                char *item = malloc(item_len + 1);
                if (!item) break;
                
                strncpy(item, item_start, item_len);
                item[item_len] = '\0';
                
                // Now look for a title in this item
                char *title_start = NULL;
                char *title_end = NULL;
                
                // Try various title formats
                if ((title_start = strstr(item, "<title><![CDATA[")) != NULL) {
                    title_start += 16;
                    title_end = strstr(title_start, "]]></title>");
                } else if ((title_start = strstr(item, "<title>")) != NULL) {
                    title_start += 7;
                    title_end = strstr(title_start, "</title>");
                }
                
                if (title_start && title_end) {
                    size_t title_len = title_end - title_start;
                    if (title_len < MAX_LINE_LENGTH - 1) {
                        char title[MAX_LINE_LENGTH] = {0};
                        strncpy(title, title_start, title_len);
                        title[title_len] = '\0';
                        
                        // Skip site titles
                        if (strstr(title, "RSS") == NULL &&
                            strstr(title, "News") != title &&
                            strstr(title, "Latest") != title) {
                            printf("%d. %s\n", count + 1, title);
                            count++;
                        }
                    }
                }
                
                free(item);
                ptr = item_end + 7;
            }
        }
        
        if (count == 0) {
            printf("Could not parse news headlines. Try again later.\n");
            printf("Response data length: %zu bytes\n", resp.size);
            printf("Debug file saved to data/news_debug.xml\n");
            
            // Fallback to displaying some generic headlines
            printf("\nFallback Headlines:\n");
            printf("1. Major World Event Shakes Global Markets\n");
            printf("2. New Scientific Discovery Announced\n");
            printf("3. Technology Leaders Unveil Next-Generation Products\n");
            printf("4. Health Experts Report Breakthrough in Treatment\n");
            printf("5. Sports Team Wins Championship in Dramatic Fashion\n");
        }
    }
    
    printf("\n");
    
    // Cleanup
    curl_easy_cleanup(curl);
    free(resp.data);
    
    return 1;
}

// Array of jokes
const char *jokes[] = {
    "Why do programmers prefer dark mode? Because light attracts bugs!",
    "Why did the programmer quit his job? Because he didn't get arrays.",
    "How many programmers does it take to change a light bulb? None, that's a hardware problem.",
    "A SQL query walks into a bar, walks up to two tables and asks, 'Can I join you?'",
    "What's a programmer's favorite hangout place? The Foo Bar.",
    "Why do programmers always mix up Halloween and Christmas? Because Oct 31 == Dec 25.",
    "Why was the JavaScript developer sad? Because he didn't Node how to Express himself.",
    "Why did the developer go broke? Because he used up all his cache.",
    "What's the object-oriented way to become wealthy? Inheritance.",
    "Why do programmers always confuse Halloween and Christmas? Because 31 OCT = 25 DEC.",
    "Why was the computer cold? It left its Windows open.",
    "What do you call a programming language with a good sense of humor? Python!",
    "Why did the programmer go broke? Because he lost his domain in the server crash.",
    "Why do Java developers wear glasses? Because they can't C#.",
    "Why did the programmer go home? Work was complied.",
    "What do you call 8 hobbits? A hobbyte.",
    "Why don't programmers like nature? It has too many bugs.",
    "What do cats and programmers have in common? When either one is unusually happy and excited, its a cause for concern.",
    "Why did the programmer get kicked out of school? Because he kept breaking the class rules.",
    "What do you call a programmer from Finland? Nerdic."
};

// Joke command - Display a random joke
int cmd_joke(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: joke\n");
        printf("Display a random programming joke.\n");
        return 1;
    }
    
    // Seed the random number generator if it's the first time
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    // Select a random joke
    int num_jokes = sizeof(jokes) / sizeof(jokes[0]);
    int joke_index = rand() % num_jokes;
    
    printf("\n");
    printf("😂 %s\n", jokes[joke_index]);
    printf("\n");
    
    return 1;
}

// Array of ASCII art
const char *ascii_art[] = {
    // Cat
    "  /\\_/\\\n ( o.o )\n  > ^ <\n",
    
    // Dog
    "  / \\__\n (    @\\___\n /         O\n/   (_____/\n/_____/   U\n",
    
    // Heart
    " _____   _____\n/      \\/      \\\n\\               /\n \\             /\n   \\         /\n     \\     /\n       \\ /\n        V\n",
    
    // House
    "    /\\\n   /  \\\n  /____\\\n |    |\n |[]  |\n |    |\n |____|\n",
    
    // Flower
    "    _\n   / \\\n /   \\\n(_____)   @\n   |      |\n   |      |\n",
    
    // Computer
    " ______________\n|  __________  |\n| |          | |\n| |          | |\n| |__________| |\n|______________|",
    
    // Rocket
    "    /\\\n   /  \\\n  |    |\n  |    |\n  |    |\n /      \\\n/        \\\n|   __   |\n|  |__|  |\n\\________/\n |      |\n |      |\n |      |\n |      |\n"
};

// ASCII art command - Generate ASCII art
int cmd_ascii(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: ascii [name]\n");
        printf("Generate ASCII art. Available art: cat, dog, heart, house, flower, computer, rocket\n");
        printf("If no name is provided, a random art will be displayed.\n");
        return 1;
    }
    
    // Seed the random number generator if it's the first time
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    int art_index = -1;
    int num_arts = sizeof(ascii_art) / sizeof(ascii_art[0]);
    
    if (args[1] == NULL) {
        // Select random art if no argument provided
        art_index = rand() % num_arts;
    } else {
        // Match argument to art name
        if (strcmp(args[1], "cat") == 0) art_index = 0;
        else if (strcmp(args[1], "dog") == 0) art_index = 1;
        else if (strcmp(args[1], "heart") == 0) art_index = 2;
        else if (strcmp(args[1], "house") == 0) art_index = 3;
        else if (strcmp(args[1], "flower") == 0) art_index = 4;
        else if (strcmp(args[1], "computer") == 0) art_index = 5;
        else if (strcmp(args[1], "rocket") == 0) art_index = 6;
        else {
            printf("Unknown art name: %s\n", args[1]);
            printf("Available art: cat, dog, heart, house, flower, computer, rocket\n");
            return 1;
        }
    }
    
    printf("\n%s\n", ascii_art[art_index]);
    
    return 1;
}

// System info command - Display system information
int cmd_sysinfo(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: sysinfo\n");
        printf("Display system information.\n");
        return 1;
    }
    
    printf("\n");
    printf("System Information\n");
    printf("-----------------\n");
    
#ifdef _WIN32
    // Windows implementation
    SYSTEM_INFO sysInfo;
    MEMORYSTATUSEX memInfo;
    DWORD version = GetVersion();
    DWORD majorVersion = (DWORD)(LOBYTE(LOWORD(version)));
    DWORD minorVersion = (DWORD)(HIBYTE(LOWORD(version)));
    
    GetSystemInfo(&sysInfo);
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    
    printf("OS: Windows %d.%d\n", majorVersion, minorVersion);
    printf("CPU Cores: %d\n", sysInfo.dwNumberOfProcessors);
    printf("Architecture: ");
    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            printf("x64 (AMD or Intel)\n");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            printf("x86 (Intel)\n");
            break;
        default:
            printf("Other\n");
    }
    
    printf("Memory (RAM): %.2f GB total, %.2f GB available\n",
           (float)memInfo.ullTotalPhys / (1024 * 1024 * 1024),
           (float)memInfo.ullAvailPhys / (1024 * 1024 * 1024));
    
    // Get hostname
    char hostname[256];
    DWORD size = sizeof(hostname);
    if (GetComputerNameA(hostname, &size)) {
        printf("Hostname: %s\n", hostname);
    }
    
    // Get username
    char username[256];
    size = sizeof(username);
    if (GetUserNameA(username, &size)) {
        printf("Username: %s\n", username);
    }
#else
    // Unix implementation
    struct utsname buf;
    
    if (uname(&buf) == 0) {
        printf("OS: %s %s\n", buf.sysname, buf.release);
        printf("Hostname: %s\n", buf.nodename);
        printf("Architecture: %s\n", buf.machine);
    }
    
    // Get number of processors
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (nprocs > 0) {
        printf("CPU Cores: %ld\n", nprocs);
    }
    
    // Get memory information
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    if (pages > 0 && page_size > 0) {
        printf("Memory (RAM): %.2f GB\n", (float)(pages * page_size) / (1024 * 1024 * 1024));
    }
    
    // Get username
    char *username = getenv("USER");
    if (username) {
        printf("Username: %s\n", username);
    }
#endif
    
    // Get current time
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("Current time: %s\n", time_str);
    
    printf("\n");
    
    return 1;
} 
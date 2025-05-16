#include "cshell.h"

// Meme command - Fetch a random meme
int cmd_meme(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: meme [category] [newtab]\n");
        printf("Fetch a random meme. Optional categories: programming, cat, dog, wholesome\n");
        printf("Add 'newtab' as the last parameter to open image directly in a new tab.\n");
        printf("If no category is provided, a random meme will be fetched.\n");
        return 1;
    }
    
    // Check if the newtab option is specified
    int open_in_new_tab = 1; // Default to direct open now
    int last_arg = 0;
    while (args[last_arg] != NULL) last_arg++;
    
    if (last_arg > 1 && strcmp(args[last_arg-1], "newtab") == 0) {
        args[last_arg-1] = NULL; // Remove the newtab parameter so it doesn't affect category
    }
    
    // More reliable meme URLs that don't require authentication or have hotlink protection
    const char *meme_urls[][5] = {
        // Programming memes
        {
            "https://i.imgur.com/QMboUJ8.jpg", // Debug meme
            "https://i.imgur.com/MZmKOZW.jpg", // Programmer joke meme
            "https://i.imgur.com/tZP7U6h.jpg", // Stackoverflow meme
            "https://i.imgur.com/oAu4xmL.jpg", // Code quality meme
            "https://i.imgur.com/3YF8Kpk.jpg"  // Bug fixing meme
        },
        // Cat memes
        {
            "https://i.imgur.com/cWBI6xz.jpg", // Cat surprise
            "https://i.imgur.com/Bw0QMKe.jpg", // Cat with glasses
            "https://i.imgur.com/m39irTe.jpg", // Cat loaf
            "https://i.imgur.com/kxPtst1.jpg", // Cat sitting
            "https://i.imgur.com/fZ0fUxj.jpg"  // Cat looking away
        },
        // Dog memes
        {
            "https://i.imgur.com/CW3KfJa.jpg", // Dog smile
            "https://i.imgur.com/oUk3AHx.jpg", // Dog waiting
            "https://i.imgur.com/T0axI0j.jpg", // Dog on bed
            "https://i.imgur.com/LLPnFk2.jpg", // Dog confused
            "https://i.imgur.com/eut45Ce.jpg"  // Dog with toy
        },
        // Wholesome memes
        {
            "https://i.imgur.com/NzIjbJs.jpg", // Friends
            "https://i.imgur.com/JER1gkg.jpg", // Support
            "https://i.imgur.com/DUlLX1q.jpg", // Motivation
            "https://i.imgur.com/ueBQww0.jpg", // Kindness
            "https://i.imgur.com/pdkd5Fx.jpg"  // Happiness
        }
    };
    
    // Seed the random number generator if it's the first time
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    int category = -1;
    if (args[1] == NULL) {
        // Random category
        category = rand() % 4;
    } else {
        // Match category
        if (strcmp(args[1], "programming") == 0) category = 0;
        else if (strcmp(args[1], "cat") == 0) category = 1;
        else if (strcmp(args[1], "dog") == 0) category = 2;
        else if (strcmp(args[1], "wholesome") == 0) category = 3;
        else {
            printf("Unknown category: %s\n", args[1]);
            printf("Available categories: programming, cat, dog, wholesome\n");
            return 1;
        }
    }
    
    // Select a random meme URL from the category
    int meme_index = rand() % 5;
    const char *meme_url = meme_urls[category][meme_index];
    
    printf("\n");
    printf("Here's your meme:\n");
    printf("%s\n", meme_url);
    
    // Ensure data directory exists
    ensure_data_directory();
    
    // Verify URL accessibility using curl before opening
    printf("Verifying URL accessibility...\n");
    
    CURL *curl = curl_easy_init();
    if (curl) {
        // Set up curl to just check the headers without downloading content
        curl_easy_setopt(curl, CURLOPT_URL, meme_url);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // HEAD request
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // Short timeout
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        
        // Use a more standard user agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
        
        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK || (http_code >= 400 && http_code < 600)) {
            printf("Warning: URL check failed with code %ld (curl result: %d)\n", http_code, res);
            printf("The image may not be accessible or the server might be blocking direct access.\n");
            printf("Here's a direct link to view in your browser: %s\n\n", meme_url);
            
            // Suggest an alternative method to view
            printf("Instead of opening directly, you can copy and paste the URL into your browser.\n");
            return 1;
        }
        
        printf("URL is accessible (HTTP code: %ld).\n", http_code);
    }
    
    // Create an HTML file with the meme image and auto-redirect
    char html_path[MAX_PATH_LENGTH];
    
    // Create simplified HTML file that displays the meme and immediately redirects
    snprintf(html_path, sizeof(html_path), "data/meme.html");
    FILE *html_file = fopen(html_path, "w");
    if (!html_file) {
        printf("Error: Could not create HTML file.\n");
        printf("Please open this meme URL manually: %s\n", meme_url);
        return 1;
    }
    
    // Write a simple HTML file that immediately redirects to the image URL
    fprintf(html_file, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(html_file, "<meta http-equiv=\"refresh\" content=\"0; url=%s\">\n", meme_url);
    fprintf(html_file, "<title>Redirecting to Meme...</title>\n");
    fprintf(html_file, "<script>window.location.href = \"%s\";</script>\n", meme_url);
    fprintf(html_file, "</head>\n<body>\n");
    fprintf(html_file, "<p>If you are not redirected automatically, <a href=\"%s\">click here</a>.</p>\n", meme_url);
    fprintf(html_file, "</body>\n</html>");
    fclose(html_file);
    
    printf("\nAttempting to open meme in browser...\n");
    
    // Try to open the direct URL first, fallback to HTML file if that fails
    int result = open_url_in_browser(meme_url);
    
    if (result == 0) {
        printf("Meme should open in your browser.\n");
        printf("\n");
        return 1;
    }
    
    // If direct URL opening failed, try the HTML file
    result = open_html_in_browser(html_path);
    
    if (result == 0) {
        printf("Meme should open in your browser.\n");
        printf("\n");
        return 1;
    }
    
    // If opening failed, try direct URL opening methods
    char command[MAX_COMMAND_LENGTH];
    int meme_opened = 0;
    
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
        printf("Trying additional methods to open the meme...\n");
        
        // Method 1: Direct cmd.exe with start command to URL
        printf("Trying cmd.exe...\n");
        snprintf(command, sizeof(command), "cmd.exe /c start \"\" \"%s\"", meme_url);
        if (system(command) == 0) {
            printf("Meme opened using cmd.exe start command.\n");
            meme_opened = 1;
        }
        
        // Method 2: Try using wslview if available
        if (!meme_opened) {
            printf("Trying wslview...\n");
            if (system_check_command_exists("wslview")) {
                snprintf(command, sizeof(command), "wslview \"%s\"", meme_url);
                if (system(command) == 0) {
                    printf("Meme opened using wslview.\n");
                    meme_opened = 1;
                }
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
        
        for (int i = 0; browsers[i] != NULL && !meme_opened; i++) {
            if (system_check_command_exists(browsers[i])) {
                snprintf(command, sizeof(command), "%s '%s' &", browsers[i], meme_url);
                if (system(command) == 0) {
                    printf("Meme opened in %s.\n", browsers[i]);
                    meme_opened = 1;
                }
            }
        }
    }
    
    // If all automatic methods failed, provide manual methods
    if (!meme_opened) {
        printf("\nCould not automatically open the meme.\n");
        printf("\nMANUAL METHODS TO VIEW THE MEME:\n");
        printf("--------------------------------\n");
        printf("1. Copy and paste this URL in your browser:\n   %s\n\n", meme_url);
        printf("2. Try running the command again with the 'newtab' option:\n   meme %s newtab\n\n", 
               args[1] != NULL ? args[1] : "");
        
        if (is_wsl) {
            printf("3. WSL commands to try:\n");
            printf("   a. explorer.exe %s\n", html_path);
            printf("   b. cmd.exe /c start \"\" \"%s\"\n", meme_url); 
            
            // Create bat file for easy execution
            char bat_path[MAX_PATH_LENGTH];
            snprintf(bat_path, sizeof(bat_path), "data/meme_open.bat");
            FILE *bat_file = fopen(bat_path, "w");
            if (bat_file) {
                fprintf(bat_file, "@echo off\n");
                fprintf(bat_file, "start \"\" \"%s\"\n", meme_url);
                fclose(bat_file);
                printf("   c. Run this command: cmd.exe /c %s\n", bat_path);
            }
            
            if (!system_check_command_exists("wslview")) {
                printf("\n4. Install WSL browser integration tools:\n");
                printf("   sudo apt update && sudo apt install -y wslu\n");
                printf("   Then use: wslview %s\n", html_path);
            } else {
                printf("\nOr try: wslview \"%s\"\n", meme_url);
            }
        }
    }
    
    printf("\n");
    
    return 1;
}

// Word count command - Count words in text
int cmd_wordcount(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: wordcount [text | -f filename]\n");
        printf("Count the number of characters, words, and lines in the text.\n");
        printf("Use -f flag to count from a file.\n");
        return 1;
    }
    
    char text[MAX_LINE_LENGTH * 10] = "";
    int from_file = 0;
    
    if (strcmp(args[1], "-f") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing filename\n");
            return 1;
        }
        
        from_file = 1;
        FILE *file = fopen(args[2], "r");
        if (file == NULL) {
            perror("wordcount");
            return 1;
        }
        
        size_t total_read = 0;
        size_t bytes_read;
        while ((bytes_read = fread(text + total_read, 1, 
                                   (MAX_LINE_LENGTH * 10) - total_read - 1, file)) > 0) {
            total_read += bytes_read;
            if (total_read >= (MAX_LINE_LENGTH * 10) - 1) {
                break;
            }
        }
        
        text[total_read] = '\0';
        fclose(file);
    } else {
        // Concatenate all arguments
        for (int i = 1; args[i] != NULL; i++) {
            strcat(text, args[i]);
            if (args[i + 1] != NULL) {
                strcat(text, " ");
            }
        }
    }
    
    // Count characters (excluding null terminator)
    int char_count = strlen(text);
    
    // Count words
    int word_count = 0;
    int in_word = 0;
    
    for (int i = 0; text[i] != '\0'; i++) {
        if (isspace((unsigned char)text[i])) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            word_count++;
        }
    }
    
    // Count lines
    int line_count = 0;
    for (int i = 0; text[i] != '\0'; i++) {
        if (text[i] == '\n') {
            line_count++;
        }
    }
    
    // Add one for the last line if there are characters and no trailing newline
    if (char_count > 0 && text[char_count - 1] != '\n') {
        line_count++;
    }
    
    printf("\n");
    if (from_file) {
        printf("Word count for file: %s\n", args[2]);
    } else {
        printf("Word count for input text:\n");
    }
    printf("Characters: %d\n", char_count);
    printf("Words: %d\n", word_count);
    printf("Lines: %d\n", line_count);
    printf("\n");
    
    return 1;
}

// Math quiz command - Take a math quiz
int cmd_mathquiz(char **args) {
    printf("DEBUG: mathquiz command started\n");
    
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: mathquiz [difficulty] [topic]\n");
        printf("Take a math quiz to test your skills.\n");
        printf("Difficulty levels: easy, medium, hard (default: medium)\n");
        printf("Topics: all, arithmetic, fractions, geometry, algebra, stats (default: all)\n");
        return 1;
    }
    
    // Seed the random number generator
    printf("DEBUG: Seeding random number generator\n");
    srand((unsigned int)time(NULL));
    
    // Set difficulty
    int difficulty = 1; // Default: medium
    const char *topic = "all"; // Default: all topics
    
    printf("DEBUG: Processing arguments\n");
    
    if (args[1] != NULL) {
        if (strcmp(args[1], "easy") == 0) {
            difficulty = 0;
        } else if (strcmp(args[1], "medium") == 0) {
            difficulty = 1;
        } else if (strcmp(args[1], "hard") == 0) {
            difficulty = 2;
        } else if (strcmp(args[1], "arithmetic") == 0 || strcmp(args[1], "fractions") == 0 || 
                   strcmp(args[1], "geometry") == 0 || strcmp(args[1], "algebra") == 0 || 
                   strcmp(args[1], "stats") == 0) {
            topic = args[1];
        } else if (strcmp(args[1], "all") != 0) {
            printf("Unknown difficulty level or topic: %s\n", args[1]);
            printf("Available difficulty levels: easy, medium, hard\n");
            printf("Available topics: all, arithmetic, fractions, geometry, algebra, stats\n");
            return 1;
        }
    }
    
    // Check if we have a second argument for topic
    if (args[1] != NULL && args[2] != NULL) {
        if (strcmp(args[2], "all") == 0 || strcmp(args[2], "arithmetic") == 0 || 
            strcmp(args[2], "fractions") == 0 || strcmp(args[2], "geometry") == 0 || 
            strcmp(args[2], "algebra") == 0 || strcmp(args[2], "stats") == 0) {
            topic = args[2];
        } else {
            printf("Unknown topic: %s\n", args[2]);
            printf("Available topics: all, arithmetic, fractions, geometry, algebra, stats\n");
            return 1;
        }
    }
    
    // Quiz settings
    int num_questions = 5; // Reduced from 10 for easier testing
    int correct_answers = 0;
    int max_num;
    
    // Set difficulty parameters
    switch (difficulty) {
        case 0: // Easy
            max_num = 20;
            break;
        case 1: // Medium
            max_num = 100;
            break;
        case 2: // Hard
            max_num = 1000;
            break;
    }
    
    printf("\n");
    printf("Math Quiz - %s Difficulty, Topic: %s\n", 
           difficulty == 0 ? "Easy" : (difficulty == 1 ? "Medium" : "Hard"),
           topic);
    printf("Answer %d questions. Type your answer after each question.\n", num_questions);
    printf("For decimal answers, round to 2 decimal places (e.g., 3.14).\n");
    printf("For fraction answers, use the format a/b (e.g., 1/2).\n\n");
    
    for (int i = 0; i < num_questions; i++) {
        // Choose a simple question type for better compatibility with the shell
        int question_type = rand() % 4; // Only use basic arithmetic for now
        
        // Variables for question generation
        int num1, num2, answer_int = 0;
        char question[MAX_LINE_LENGTH], answer_str[32] = {0};
        char user_input[100] = {0};
        int is_fraction = 0;
        
        // Generate a simple arithmetic question
        switch (question_type) {
            case 0: // Addition
                num1 = rand() % max_num + 1;
                num2 = rand() % max_num + 1;
                snprintf(question, sizeof(question), "What is %d + %d?", num1, num2);
                answer_int = num1 + num2;
                snprintf(answer_str, sizeof(answer_str), "%d", answer_int);
                break;
                
            case 1: // Subtraction
                num1 = rand() % max_num + 1;
                num2 = rand() % max_num + 1;
                // Ensure num1 >= num2 for easier subtraction
                if (num1 < num2) {
                    int temp = num1;
                    num1 = num2;
                    num2 = temp;
                }
                snprintf(question, sizeof(question), "What is %d - %d?", num1, num2);
                answer_int = num1 - num2;
                snprintf(answer_str, sizeof(answer_str), "%d", answer_int);
                break;
                
            case 2: // Multiplication
                if (difficulty == 0) {
                    num1 = rand() % 12 + 1; // Times tables for easy
                    num2 = rand() % 12 + 1;
                } else if (difficulty == 1) {
                    num1 = rand() % 20 + 1;
                    num2 = rand() % 20 + 1;
                } else {
                    num1 = rand() % max_num + 1;
                    num2 = rand() % 30 + 1; // Keep one number manageable
                }
                snprintf(question, sizeof(question), "What is %d × %d?", num1, num2);
                answer_int = num1 * num2;
                snprintf(answer_str, sizeof(answer_str), "%d", answer_int);
                break;
                
            case 3: // Division
                // Ensure clean division
                num2 = rand() % 10 + 1;
                answer_int = rand() % 10 + 1;
                num1 = answer_int * num2;
                snprintf(question, sizeof(question), "What is %d ÷ %d?", num1, num2);
                snprintf(answer_str, sizeof(answer_str), "%d", answer_int);
                break;
        }
        
        // Ask the question
        printf("Question %d: %s\n", i + 1, question);
        printf("Your answer: ");
        fflush(stdout); // Make sure prompt is displayed immediately
        
        // Get user input with clearer error handling
        if (fgets(user_input, sizeof(user_input), stdin) == NULL) {
            printf("Error reading input. Please try again.\n");
            i--; // Retry this question
            continue;
        }
        
        // Trim newline
        char *newline = strchr(user_input, '\n');
        if (newline) {
            *newline = '\0';
        }
        
        // Trim whitespace
        trim_whitespace(user_input);
        
        // Check if input is valid
        if (strlen(user_input) == 0) {
            printf("No input received. Please try again.\n");
            i--; // Retry this question
            continue;
        }
        
        // Check answer (simple string compare for now)
        int is_correct = (strcmp(user_input, answer_str) == 0);
        
        if (is_correct) {
            printf(COLOR_GREEN "Correct!" COLOR_RESET "\n\n");
            correct_answers++;
        } else {
            printf(COLOR_RED "Incorrect." COLOR_RESET " The answer is %s.\n\n", answer_str);
        }
    }
    
    printf("Quiz Completed!\n");
    printf("You got %d out of %d questions correct (%.1f%%)\n", 
           correct_answers, num_questions, 
           (float)correct_answers / num_questions * 100);
    
    if (correct_answers == num_questions) {
        printf("Perfect score! Excellent job!\n");
    } else if ((float)correct_answers / num_questions >= 0.8) {
        printf("Great job!\n");
    } else if ((float)correct_answers / num_questions >= 0.6) {
        printf("Good effort!\n");
    } else {
        printf("Keep practicing!\n");
    }
    
    printf("\n");
    
    return 1;
}

// Day fact command - Display a fact about today
int cmd_dayfact(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: dayfact\n");
        printf("Display a fact about today's date.\n");
        return 1;
    }
    
    // Get current date
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    
    int month = tm_info->tm_mon + 1; // tm_mon is 0-11
    int day = tm_info->tm_mday;
    
    // Format date string
    char date_str[20];
    strftime(date_str, sizeof(date_str), "%B %d", tm_info);
    
    printf("\n");
    printf("Today is %s\n", date_str);
    printf("Did you know?\n");
    
    // Match the month and day to a fact
    // These are just a few examples for each month
    if (month == 1) { // January
        if (day == 1) printf("On this day in 1863, Abraham Lincoln issued the Emancipation Proclamation.\n");
        else if (day == 15) printf("On this day in 1929, Martin Luther King Jr. was born.\n");
        else if (day == 27) printf("On this day in 1945, the Auschwitz concentration camp was liberated.\n");
        else printf("In January 1901, the first great Texas oil gusher was discovered at Spindletop.\n");
    } else if (month == 2) { // February
        if (day == 11) printf("On this day in 1990, Nelson Mandela was released from prison after 27 years.\n");
        else if (day == 14) printf("On this day in 270 AD, Saint Valentine was executed, leading to Valentine's Day.\n");
        else if (day == 29) printf("Today is a leap day! It only occurs every 4 years (with some exceptions).\n");
        else printf("February was named after the Roman festival of purification called Februa.\n");
    } else if (month == 3) { // March
        if (day == 14) printf("On this day we celebrate Pi Day (3.14).\n");
        else if (day == 17) printf("On this day, St. Patrick's Day is celebrated around the world.\n");
        else if (day == 31) printf("On this day in 1889, the Eiffel Tower was inaugurated in Paris.\n");
        else printf("March is named after Mars, the Roman god of war.\n");
    } else if (month == 4) { // April
        if (day == 1) printf("April Fools' Day is celebrated on this day in many countries.\n");
        else if (day == 22) printf("Today is Earth Day, first celebrated in 1970.\n");
        else if (day == 23) printf("On this day in 1564, William Shakespeare was born.\n");
        else printf("The name April comes from the Latin word 'aperire', meaning 'to open', referring to flowers and trees blooming.\n");
    } else if (month == 5) { // May
        if (day == 1) printf("May Day is celebrated as International Workers' Day in many countries.\n");
        else if (day == 5) printf("Cinco de Mayo celebrates the Mexican Army's victory at the Battle of Puebla in 1862.\n");
        else if (day == 25) printf("On this day in 1977, the first Star Wars film was released.\n");
        else printf("May is named after Maia, the Greek goddess of fertility.\n");
    } else if (month == 6) { // June
        if (day == 6) printf("On this day in 1944, D-Day, the Allied invasion of Normandy began in World War II.\n");
        else if (day == 19) printf("Juneteenth celebrates the emancipation of enslaved African Americans.\n");
        else if (day == 29) printf("On this day in 2007, the first iPhone was released.\n");
        else printf("June was named after Juno, the Roman goddess of marriage and childbirth.\n");
    } else if (month == 7) { // July
        if (day == 4) printf("On this day in 1776, the United States adopted the Declaration of Independence.\n");
        else if (day == 20) printf("On this day in 1969, Apollo 11 landed on the moon.\n");
        else if (day == 31) printf("On this day in 1980, Harry Potter (fictional character) was born.\n");
        else printf("July was named after Julius Caesar, who was born in this month.\n");
    } else if (month == 8) { // August
        if (day == 12) printf("The Perseid meteor shower typically peaks around this date each year.\n");
        else if (day == 21) printf("On this day in 1911, the Mona Lisa was stolen from the Louvre.\n");
        else if (day == 28) printf("On this day in 1963, Martin Luther King Jr. delivered his 'I Have a Dream' speech.\n");
        else printf("August was named after Augustus Caesar, the first Roman emperor.\n");
    } else if (month == 9) { // September
        if (day == 11) printf("On this day in 2001, the 9/11 terrorist attacks occurred in the United States.\n");
        else if (day == 22) printf("The autumn equinox typically falls on this day, marking the first day of fall.\n");
        else if (day == 29) printf("On this day in 1954, CERN (European Organization for Nuclear Research) was established.\n");
        else printf("September comes from the Latin word 'septem', meaning seven, as it was the seventh month in the ancient Roman calendar.\n");
    } else if (month == 10) { // October
        if (day == 14) printf("On this day in 1947, Chuck Yeager became the first person to break the sound barrier.\n");
        else if (day == 31) printf("Halloween is celebrated on this day in many countries.\n");
        else if (day == 29) printf("On this day in 1929, the stock market crashed, leading to the Great Depression.\n");
        else printf("October comes from the Latin word 'octo', meaning eight, as it was the eighth month in the ancient Roman calendar.\n");
    } else if (month == 11) { // November
        if (day == 11) printf("Veterans Day/Remembrance Day is observed on this day to honor military veterans.\n");
        else if (day == 19) printf("On this day in 1863, Abraham Lincoln delivered the Gettysburg Address.\n");
        else if (day == 30) printf("On this day in 1872, the first international football (soccer) match was played between England and Scotland.\n");
        else printf("November comes from the Latin word 'novem', meaning nine, as it was the ninth month in the ancient Roman calendar.\n");
    } else if (month == 12) { // December
        if (day == 10) printf("On this day, the Nobel Prizes are awarded annually.\n");
        else if (day == 25) printf("Christmas Day is celebrated on this day in many countries.\n");
        else if (day == 31) printf("New Year's Eve is celebrated on this day around the world.\n");
        else printf("December comes from the Latin word 'decem', meaning ten, as it was the tenth month in the ancient Roman calendar.\n");
    }
    
    printf("\n");
    
    return 1;
}

// Colorize command - Colorize text output
int cmd_colorize(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: colorize [color] [text]\n");
        printf("Display colored text.\n\n");
        printf("Available colors:\n");
        printf(COLOR_RED "red " COLOR_RESET);
        printf(COLOR_GREEN "green " COLOR_RESET);
        printf(COLOR_YELLOW "yellow " COLOR_RESET);
        printf(COLOR_BLUE "blue " COLOR_RESET);
        printf(COLOR_MAGENTA "magenta " COLOR_RESET);
        printf(COLOR_CYAN "cyan " COLOR_RESET);
        printf(COLOR_WHITE "white " COLOR_RESET);
        printf(COLOR_BOLD "bold\n" COLOR_RESET);
        return 1;
    }
    
    const char *color_code = COLOR_RESET;
    
    // Match color
    if (strcmp(args[1], "red") == 0) {
        color_code = COLOR_RED;
    } else if (strcmp(args[1], "green") == 0) {
        color_code = COLOR_GREEN;
    } else if (strcmp(args[1], "yellow") == 0) {
        color_code = COLOR_YELLOW;
    } else if (strcmp(args[1], "blue") == 0) {
        color_code = COLOR_BLUE;
    } else if (strcmp(args[1], "magenta") == 0) {
        color_code = COLOR_MAGENTA;
    } else if (strcmp(args[1], "cyan") == 0) {
        color_code = COLOR_CYAN;
    } else if (strcmp(args[1], "white") == 0) {
        color_code = COLOR_WHITE;
    } else if (strcmp(args[1], "bold") == 0) {
        color_code = COLOR_BOLD;
    } else {
        printf("Unknown color: %s\n", args[1]);
        printf("Available colors: red, green, yellow, blue, magenta, cyan, white, bold\n");
        return 1;
    }
    
    if (args[2] == NULL) {
        printf("Error: Missing text to colorize\n");
        return 1;
    }
    
    // Build the text
    char text[MAX_LINE_LENGTH] = "";
    for (int i = 2; args[i] != NULL; i++) {
        strcat(text, args[i]);
        if (args[i + 1] != NULL) {
            strcat(text, " ");
        }
    }
    
    // Print colored text
    printf("%s%s%s\n", color_code, text, COLOR_RESET);
    
    return 1;
}

// Calculator command - Perform calculations
int cmd_calculator(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: calc [expression]\n");
        printf("Perform mathematical calculations.\n\n");
        printf("Supported operations:\n");
        printf("  + Addition        e.g., calc 5+3\n");
        printf("  - Subtraction     e.g., calc 10-4\n");
        printf("  * Multiplication  e.g., calc 6*7\n");
        printf("  / Division        e.g., calc 20/4\n");
        printf("  %% Modulo          e.g., calc 10%%3\n");
        printf("  ^ Power           e.g., calc 2^3\n\n");
        printf("Advanced functions:\n");
        printf("  sqrt(x)    Square root         e.g., calc sqrt(16)\n");
        printf("  sin(x)     Sine (radians)      e.g., calc sin(0.5)\n");
        printf("  cos(x)     Cosine (radians)    e.g., calc cos(0.5)\n");
        printf("  tan(x)     Tangent (radians)   e.g., calc tan(0.5)\n");
        printf("  log(x)     Natural logarithm   e.g., calc log(10)\n");
        printf("  log10(x)   Base-10 logarithm   e.g., calc log10(100)\n");
        printf("  exp(x)     Exponential (e^x)   e.g., calc exp(2)\n");
        printf("  abs(x)     Absolute value      e.g., calc abs(-5)\n");
        printf("  floor(x)   Round down          e.g., calc floor(3.7)\n");
        printf("  ceil(x)    Round up            e.g., calc ceil(3.2)\n");
        printf("  round(x)   Round to nearest    e.g., calc round(3.5)\n\n");
        printf("Constants:\n");
        printf("  pi         3.14159...          e.g., calc 2*pi\n");
        printf("  e          2.71828...          e.g., calc e^2\n\n");
        printf("You can combine operations and functions: calc 2 + sin(pi/2) * 5\n");
        return 1;
    }

    // Build the expression string
    char expression[MAX_LINE_LENGTH] = "";
    for (int i = 1; args[i] != NULL; i++) {
        strcat(expression, args[i]);
        if (args[i + 1] != NULL) {
            strcat(expression, " ");
        }
    }
    
    // Handle some common functions and constants that aren't part of the standard C library
    char processed_expr[MAX_LINE_LENGTH * 2] = "";
    
    // Replace ^ with pow()
    int reading = 0;
    int j = 0;
    int in_func = 0;
    int in_paren = 0;
    char left_operand[MAX_LINE_LENGTH] = "";
    int left_idx = 0;
    
    for (int i = 0; expression[i] != '\0'; i++) {
        if (expression[i] == '(') {
            in_paren++;
        } else if (expression[i] == ')') {
            in_paren--;
        }
        
        // Handle power operator (^)
        if (expression[i] == '^' && !in_func) {
            processed_expr[j++] = '\0'; // Null terminate to use string functions
            
            // Pop the last number or expression off processed_expr
            int last_space = -1;
            int last_operator = -1;
            
            for (int k = 0; k < j - 1; k++) {
                if (processed_expr[k] == ' ') {
                    last_space = k;
                } else if (processed_expr[k] == '+' || processed_expr[k] == '-' ||
                          processed_expr[k] == '*' || processed_expr[k] == '/' ||
                          processed_expr[k] == '%') {
                    last_operator = k;
                }
            }
            
            int start_idx = 0;
            if (last_operator > last_space) {
                start_idx = last_operator + 1;
            } else if (last_space >= 0) {
                start_idx = last_space + 1;
            }
            
            strncpy(left_operand, processed_expr + start_idx, j - start_idx);
            left_operand[j - start_idx] = '\0';
            
            processed_expr[start_idx] = '\0';
            j = start_idx;
            
            strcat(processed_expr, "pow(");
            j += 4;
            
            strcat(processed_expr, left_operand);
            j += strlen(left_operand);
            
            strcat(processed_expr, ", ");
            j += 2;
            
            reading = 1;
            continue;
        } else if (reading) {
            if (!isalnum((unsigned char)expression[i]) && expression[i] != '.' && 
                expression[i] != '(' && expression[i] != ')' && 
                !(expression[i] == '-' && i > 0 && (expression[i-1] == '(' || isspace((unsigned char)expression[i-1])))) {
                processed_expr[j++] = ')';
                processed_expr[j++] = expression[i];
                reading = 0;
            } else {
                processed_expr[j++] = expression[i];
            }
            continue;
        }
        
        // Check for functions
        if (i + 4 < MAX_LINE_LENGTH && strncmp(expression + i, "sqrt", 4) == 0) {
            in_func = 1;
        } else if (i + 3 < MAX_LINE_LENGTH && (strncmp(expression + i, "sin", 3) == 0 || 
                                              strncmp(expression + i, "cos", 3) == 0 ||
                                              strncmp(expression + i, "tan", 3) == 0 ||
                                              strncmp(expression + i, "log", 3) == 0 ||
                                              strncmp(expression + i, "exp", 3) == 0 ||
                                              strncmp(expression + i, "abs", 3) == 0)) {
            in_func = 1;
        } else if (i + 5 < MAX_LINE_LENGTH && (strncmp(expression + i, "floor", 5) == 0 ||
                                              strncmp(expression + i, "ceil", 4) == 0 ||
                                              strncmp(expression + i, "round", 5) == 0)) {
            in_func = 1;
        } else if (expression[i] == '(') {
            if (in_func) in_func = 0;
        }
        
        // Replace constants
        if (i + 2 < MAX_LINE_LENGTH && strncmp(expression + i, "pi", 2) == 0 &&
            (i == 0 || !isalnum((unsigned char)expression[i-1])) &&
            (expression[i+2] == '\0' || !isalnum((unsigned char)expression[i+2]))) {
            strcat(processed_expr, "3.14159265358979");
            j += 16;
            i += 1; // Skip the next character
            continue;
        } else if (i + 1 < MAX_LINE_LENGTH && expression[i] == 'e' &&
            (i == 0 || !isalnum((unsigned char)expression[i-1])) &&
            (expression[i+1] == '\0' || !isalnum((unsigned char)expression[i+1]))) {
            strcat(processed_expr, "2.71828182845905");
            j += 16;
            continue;
        }
        
        processed_expr[j++] = expression[i];
    }
    
    if (reading) {
        processed_expr[j++] = ')';
    }
    
    processed_expr[j] = '\0';
    
    // Check for missing multiplications like "2(3+4)" -> "2*(3+4)"
    char final_expr[MAX_LINE_LENGTH * 2] = "";
    j = 0;
    
    for (int i = 0; processed_expr[i] != '\0'; i++) {
        final_expr[j++] = processed_expr[i];
        
        if (processed_expr[i] != '\0' && processed_expr[i+1] != '\0' &&
            ((isdigit((unsigned char)processed_expr[i]) && processed_expr[i+1] == '(') ||
             (processed_expr[i] == ')' && (isdigit((unsigned char)processed_expr[i+1]) || 
                                          processed_expr[i+1] == '(' ||
                                          isalpha((unsigned char)processed_expr[i+1]))))) {
            final_expr[j++] = '*';
        }
    }
    final_expr[j] = '\0';
    
    // Compile the expression
    void *mathLib = NULL;
    double (*evalExpr)(const char *) = NULL;
    
#ifdef _WIN32
    mathLib = LoadLibrary("msvcrt.dll");
    evalExpr = (double (*)(const char *))GetProcAddress(mathLib, "atof");
    
    // Since Windows doesn't have a direct eval function, we'll use a simple parser
    // for basic operations here. For complex expressions, use the GNU Scientific Library
    // or another more complete math library.
    
    double result = 0.0;
    
    // For demonstration, just compute using simple operators
    // This is a very simplified calculator
    sscanf(final_expr, "%lf", &result);
    
    if (strstr(final_expr, "+")) {
        double operand;
        sscanf(strstr(final_expr, "+") + 1, "%lf", &operand);
        result += operand;
    } else if (strstr(final_expr, "-")) {
        double operand;
        sscanf(strstr(final_expr, "-") + 1, "%lf", &operand);
        result -= operand;
    } else if (strstr(final_expr, "*")) {
        double operand;
        sscanf(strstr(final_expr, "*") + 1, "%lf", &operand);
        result *= operand;
    } else if (strstr(final_expr, "/")) {
        double operand;
        sscanf(strstr(final_expr, "/") + 1, "%lf", &operand);
        if (operand != 0) {
            result /= operand;
        } else {
            printf("Error: Division by zero\n");
            return 1;
        }
    }
    
    if (mathLib) {
        FreeLibrary(mathLib);
    }
#else
    // Use math.h functions to evaluate by compiling a temporary file
    FILE *tempFile = fopen("data/calc_temp.c", "w");
    if (tempFile == NULL) {
        printf("Error: Failed to create temporary file\n");
        return 1;
    }
    
    fprintf(tempFile, "#include <stdio.h>\n");
    fprintf(tempFile, "#include <math.h>\n");
    fprintf(tempFile, "int main() {\n");
    fprintf(tempFile, "    double result = %s;\n", final_expr);
    fprintf(tempFile, "    printf(\"%%lf\\n\", result);\n");
    fprintf(tempFile, "    return 0;\n");
    fprintf(tempFile, "}\n");
    fclose(tempFile);
    
    // Compile and run
    system("gcc -o data/calc_temp data/calc_temp.c -lm");
    FILE *output = popen("./data/calc_temp", "r");
    
    double result = 0.0;
    if (output) {
        char buffer[64];
        if (fgets(buffer, sizeof(buffer), output) != NULL) {
            result = atof(buffer);
        }
        pclose(output);
    }
    
    // Clean up
    remove("data/calc_temp.c");
    remove("data/calc_temp");
#endif

    // Display result
    printf("Expression: %s\n", expression);
    printf("Evaluated: %s\n", final_expr);
    printf("Result: %f\n", result);
    
    // Check if it's a whole number
    if (result == (int)result) {
        printf("Result (integer): %d\n", (int)result);
    }
    
    // Format in scientific notation if very large or small
    if (fabs(result) > 1000000 || (fabs(result) < 0.0001 && result != 0)) {
        printf("Result (scientific): %e\n", result);
    }
    
    return 1;
} 
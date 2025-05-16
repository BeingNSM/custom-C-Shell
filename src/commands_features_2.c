#include "cshell.h"

// Weather command - Display weather information
int cmd_weather(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: weather [location]\n");
        printf("Display current weather information for the specified location.\n");
        printf("Example: weather delhi\n");
        return 1;
    }
    
    // Build the location string
    char location[MAX_LINE_LENGTH] = "";
    for (int i = 1; args[i] != NULL; i++) {
        strcat(location, args[i]);
        if (args[i + 1] != NULL) {
            strcat(location, "+");
        }
    }
    
    // First, get the basic weather data without emojis (more reliable)
    char url[MAX_LINE_LENGTH];
    snprintf(url, sizeof(url), "https://wttr.in/%s?format=%%l:+%%C+%%t+%%w+%%h+%%p+%%m&m", location);
    
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
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
    
    // Perform the request
    CURLcode res = curl_easy_perform(curl);
    
    // Check for errors
    if (res != CURLE_OK) {
        printf("Error: %s\n", curl_easy_strerror(res));
    } else {
        if (resp.data && strlen(resp.data) > 0) {
            // Process the response to fix any encoding issues and add our own emojis
            char *processed = resp.data;
            
            // Extract components for custom formatting with ASCII symbols
            char location[MAX_LINE_LENGTH] = {0};
            char condition[MAX_LINE_LENGTH] = {0};
            char temperature[MAX_LINE_LENGTH] = {0};
            char wind[MAX_LINE_LENGTH] = {0};
            char humidity[MAX_LINE_LENGTH] = {0};
            char precipitation[MAX_LINE_LENGTH] = {0};
            
            // Use sscanf to parse the wttr.in output format
            sscanf(processed, "%[^:]: %[^+]%[^ ] %[^ ] %[^ ] %[^\n]", 
                   location, condition, temperature, wind, humidity, precipitation);
            
            // Trim whitespace
            trim_whitespace(location);
            trim_whitespace(condition);
            trim_whitespace(temperature);
            trim_whitespace(wind);
            trim_whitespace(humidity);
            trim_whitespace(precipitation);
            
            // Select weather symbol based on condition (ASCII only)
            const char *weather_symbol = "(*)"; // default: partly cloudy
            
            // Simple weather condition to ASCII mapping
            if (strstr(condition, "Clear") || strstr(condition, "Sunny") || strstr(condition, "Fair")) {
                weather_symbol = "☼"; // ASCII sun
            } else if (strstr(condition, "Partly cloudy")) {
                weather_symbol = "⊕"; // ASCII partly cloudy
            } else if (strstr(condition, "Cloudy") || strstr(condition, "Overcast")) {
                weather_symbol = "☁"; // ASCII cloud
            } else if (strstr(condition, "Rain") || strstr(condition, "Drizzle") || strstr(condition, "Showers")) {
                weather_symbol = "/"; // ASCII rain
            } else if (strstr(condition, "Thunder") || strstr(condition, "Lightning")) {
                weather_symbol = "⚡"; // ASCII lightning
            } else if (strstr(condition, "Snow") || strstr(condition, "Sleet")) {
                weather_symbol = "*"; // ASCII snow
            } else if (strstr(condition, "Fog") || strstr(condition, "Mist") || strstr(condition, "Haze") || strstr(condition, "Smoke")) {
                weather_symbol = "~"; // ASCII fog
            }
            
            // Temperature symbol
            const char *temp_symbol = "[TEMP]";
            
            // Wind direction and symbol
            const char *wind_symbol = "[WIND]";
            char wind_with_direction[32] = ""; // Will hold the formatted wind text
            
            if (strstr(wind, "km/h") != NULL) {
                // Extract wind speed and direction
                int wind_speed = 0;
                char wind_dir[8] = "";
                
                if (sscanf(wind, "%d%7s", &wind_speed, wind_dir) >= 1) {
                    // Check for direction characters and replace with arrow symbols
                    if (strstr(wind_dir, "N") != NULL) {
                        snprintf(wind_with_direction, sizeof(wind_with_direction), "%s %dkm/h (N)", wind_symbol, wind_speed);
                    } else if (strstr(wind_dir, "S") != NULL) {
                        snprintf(wind_with_direction, sizeof(wind_with_direction), "%s %dkm/h (S)", wind_symbol, wind_speed);
                    } else if (strstr(wind_dir, "E") != NULL) {
                        snprintf(wind_with_direction, sizeof(wind_with_direction), "%s %dkm/h (E)", wind_symbol, wind_speed);
                    } else if (strstr(wind_dir, "W") != NULL) {
                        snprintf(wind_with_direction, sizeof(wind_with_direction), "%s %dkm/h (W)", wind_symbol, wind_speed);
                    } else {
                        snprintf(wind_with_direction, sizeof(wind_with_direction), "%s %dkm/h", wind_symbol, wind_speed);
                    }
                } else {
                    // Fallback if parsing fails
                    snprintf(wind_with_direction, sizeof(wind_with_direction), "%s %s", wind_symbol, wind);
                }
            } else {
                // Fallback for unusual formats
                snprintf(wind_with_direction, sizeof(wind_with_direction), "%s %s", wind_symbol, wind);
            }
            
            // Humidity symbol
            const char *humidity_symbol = "H:";
            
            // Format the output with reliable ASCII symbols
            printf("\n");
            printf("Weather Information for %s:\n", location);
            printf("------------------------------\n");
            printf("Condition:   %s (%s)\n", condition, weather_symbol);
            printf("Temperature: %s\n", temperature);
            printf("Wind:        %dkm/h\n", atoi(wind));
            printf("Humidity:    %s\n", humidity);
            
            // Get a more detailed forecast just for precipitation
            char detailed_url[MAX_LINE_LENGTH];
            snprintf(detailed_url, sizeof(detailed_url), "https://wttr.in/%s?format=%%p&m", location);
            
            // Reset response data
            free(resp.data);
            resp.size = 0;
            resp.data = malloc(1);
            resp.data[0] = '\0';
            
            // Set new URL
            curl_easy_setopt(curl, CURLOPT_URL, detailed_url);
            
            // Perform the request
            res = curl_easy_perform(curl);
            
            // Get additional info for sunrise/sunset times
            if (res == CURLE_OK && resp.data && strlen(resp.data) > 0) {
                char precipitation_data[32] = {0};
                strncpy(precipitation_data, resp.data, sizeof(precipitation_data) - 1);
                trim_whitespace(precipitation_data);
                
                if (strcmp(precipitation_data, "0mm") != 0 && strlen(precipitation_data) > 0) {
                    printf("Precipitation: %s\n", precipitation_data);
                }
            }
            
            // Get sunrise and sunset times
            char times_url[MAX_LINE_LENGTH];
            snprintf(times_url, sizeof(times_url), "https://wttr.in/%s?format=%%S,%%s,%%D", location);
            
            // Reset response data
            free(resp.data);
            resp.size = 0;
            resp.data = malloc(1);
            resp.data[0] = '\0';
            
            // Set new URL
            curl_easy_setopt(curl, CURLOPT_URL, times_url);
            
            // Perform the request
            res = curl_easy_perform(curl);
            
            if (res == CURLE_OK && resp.data && strlen(resp.data) > 0) {
                char *details = resp.data;
                
                // Parse the sunrise/sunset/moon data
                char sunrise[32] = {0};
                char sunset[32] = {0};
                char moon_phase[32] = {0};
                
                sscanf(details, "%31[^,],%31[^,],%31s", sunrise, sunset, moon_phase);
                trim_whitespace(sunrise);
                trim_whitespace(sunset);
                trim_whitespace(moon_phase);
                
                if (strlen(sunrise) > 0 || strlen(sunset) > 0 || strlen(moon_phase) > 0) {
                    printf("\nAdditional Information:\n");
                    printf("---------------------\n");
                    
                    if (strlen(sunrise) > 0) {
                        printf("Sunrise: %s\n", sunrise);
                    }
                    
                    if (strlen(sunset) > 0) {
                        printf("Sunset:  %s\n", sunset);
                    }
                    
                    // Use text for moon phase
                    if (strlen(moon_phase) > 0) {
                        // Determine moon phase text based on the data
                        const char *moon_text = "Moon Phase";
                        
                        if (strstr(moon_phase, "New") != NULL) {
                            moon_text = "New Moon";
                        } else if (strstr(moon_phase, "Waxing") != NULL && strstr(moon_phase, "Crescent") != NULL) {
                            moon_text = "Waxing Crescent";
                        } else if (strstr(moon_phase, "First") != NULL) {
                            moon_text = "First Quarter";
                        } else if (strstr(moon_phase, "Waxing") != NULL && strstr(moon_phase, "Gibbous") != NULL) {
                            moon_text = "Waxing Gibbous";
                        } else if (strstr(moon_phase, "Full") != NULL) {
                            moon_text = "Full Moon";
                        } else if (strstr(moon_phase, "Waning") != NULL && strstr(moon_phase, "Gibbous") != NULL) {
                            moon_text = "Waning Gibbous";
                        } else if (strstr(moon_phase, "Last") != NULL) {
                            moon_text = "Last Quarter";
                        } else if (strstr(moon_phase, "Waning") != NULL && strstr(moon_phase, "Crescent") != NULL) {
                            moon_text = "Waning Crescent";
                        }
                        
                        printf("Moon:    %s\n", moon_text);
                    }
                }
            }
            
            printf("\nForecast: https://wttr.in/%s\n", location);
        } else {
            printf("Error: Could not retrieve weather data\n");
        }
    }
    
    // Cleanup
    curl_easy_cleanup(curl);
    free(resp.data);
    
    return 1;
}

// Reminder thread function
void *reminder_thread(void *arg) {
    int reminder_id = *((int *)arg);
    free(arg);
    
    Reminder *rem = &reminders[reminder_id];
    
    // Calculate sleep time
    time_t now = time(NULL);
    time_t sleep_seconds = rem->timestamp - now;
    
    if (sleep_seconds > 0) {
        sleep((unsigned int)sleep_seconds);
    }
    
    // Display reminder
    printf("\n");
    printf("⏰ REMINDER: %s\n", rem->message);
    printf("\a"); // Bell sound
    
    // Mark reminder as inactive
    rem->active = 0;
    
    return NULL;
}

// Reminder command - Set and manage reminders
int cmd_reminder(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: reminder [command] [arguments]\n");
        printf("Set and manage reminders.\n\n");
        printf("Commands:\n");
        printf("  add [minutes] [message]    Add a new reminder\n");
        printf("  list                       List all active reminders\n");
        printf("  delete [number]            Delete a reminder\n");
        return 1;
    }
    
    if (strcmp(args[1], "add") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            printf("Error: Missing arguments. Usage: reminder add [minutes] [message]\n");
            return 1;
        }
        
        if (reminder_count >= MAX_REMINDERS) {
            printf("Error: Maximum number of reminders reached\n");
            return 1;
        }
        
        int minutes = atoi(args[2]);
        if (minutes <= 0) {
            printf("Error: Please specify a positive number of minutes\n");
            return 1;
        }
        
        // Build the message
        char message[MAX_LINE_LENGTH] = "";
        for (int i = 3; args[i] != NULL; i++) {
            strcat(message, args[i]);
            if (args[i + 1] != NULL) {
                strcat(message, " ");
            }
        }
        
        // Set up the reminder
        Reminder *rem = &reminders[reminder_count];
        strcpy(rem->message, message);
        rem->timestamp = time(NULL) + (minutes * 60);
        rem->active = 1;
        
        // Create a thread for the reminder
        int *arg = malloc(sizeof(int));
        *arg = reminder_count;
        
        if (pthread_create(&rem->thread_id, NULL, reminder_thread, arg) != 0) {
            perror("Failed to create reminder thread");
            free(arg);
            return 1;
        }
        
        // Detach the thread
        pthread_detach(rem->thread_id);
        
        printf("Reminder set for %d minutes from now\n", minutes);
        reminder_count++;
        
    } else if (strcmp(args[1], "list") == 0) {
        int active_count = 0;
        
        printf("\n");
        printf("Active Reminders:\n");
        printf("----------------\n");
        
        for (int i = 0; i < reminder_count; i++) {
            if (reminders[i].active) {
                // Convert timestamp to readable format
                char time_str[64];
                struct tm *timeinfo = localtime(&reminders[i].timestamp);
                strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
                
                printf("[%d] %s - %s\n", i + 1, time_str, reminders[i].message);
                active_count++;
            }
        }
        
        if (active_count == 0) {
            printf("No active reminders\n");
        }
        printf("\n");
        
    } else if (strcmp(args[1], "delete") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing reminder number\n");
            return 1;
        }
        
        int reminder_num = atoi(args[2]) - 1;
        if (reminder_num < 0 || reminder_num >= reminder_count) {
            printf("Error: Invalid reminder number\n");
            return 1;
        }
        
        // Mark the reminder as inactive
        reminders[reminder_num].active = 0;
        
        printf("Reminder deleted\n");
        
    } else {
        printf("Error: Unknown command: %s\n", args[1]);
    }
    
    return 1;
}

// Array of quotes
const char *quotes[] = {
    "The only way to do great work is to love what you do. - Steve Jobs",
    "Life is what happens when you're busy making other plans. - John Lennon",
    "The future belongs to those who believe in the beauty of their dreams. - Eleanor Roosevelt",
    "Success is not final, failure is not fatal: It is the courage to continue that counts. - Winston Churchill",
    "Believe you can and you're halfway there. - Theodore Roosevelt",
    "It does not matter how slowly you go as long as you do not stop. - Confucius",
    "Everything you've ever wanted is on the other side of fear. - George Addair",
    "The only limit to our realization of tomorrow will be our doubts of today. - Franklin D. Roosevelt",
    "Do what you can, with what you have, where you are. - Theodore Roosevelt",
    "You miss 100% of the shots you don't take. - Wayne Gretzky",
    "The greatest glory in living lies not in never falling, but in rising every time we fall. - Nelson Mandela",
    "The way to get started is to quit talking and begin doing. - Walt Disney",
    "Education is the most powerful weapon which you can use to change the world. - Nelson Mandela",
    "Your time is limited, so don't waste it living someone else's life. - Steve Jobs",
    "If life were predictable it would cease to be life, and be without flavor. - Eleanor Roosevelt",
    "Life is really simple, but we insist on making it complicated. - Confucius",
    "The journey of a thousand miles begins with one step. - Lao Tzu",
    "Don't judge each day by the harvest you reap but by the seeds that you plant. - Robert Louis Stevenson",
    "It is during our darkest moments that we must focus to see the light. - Aristotle",
    "Yesterday is history. Tomorrow is a mystery. Today is a gift. That's why we call it 'The Present'. - Eleanor Roosevelt"
};

// Quote command - Display a random quote
int cmd_quote(char **args) {
    if (args[1] != NULL && strcmp(args[1], "--help") == 0) {
        printf("Usage: quote\n");
        printf("Display a random inspirational quote.\n");
        return 1;
    }
    
    // Seed the random number generator if it's the first time
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    // Select a random quote
    int num_quotes = sizeof(quotes) / sizeof(quotes[0]);
    int quote_index = rand() % num_quotes;
    
    printf("\n");
    printf("💭 %s\n", quotes[quote_index]);
    printf("\n");
    
    return 1;
} 
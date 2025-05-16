#include "cshell.h"

// Save the todo list to a file
void save_todo_list(void) {
    FILE *file = fopen("data/todo.txt", "w");
    if (file == NULL) {
        perror("Could not open todo file for writing");
        return;
    }
    
    for (int i = 0; i < todo_count; i++) {
        fprintf(file, "%d|%s\n", todo_list[i].completed, todo_list[i].content);
    }
    
    fclose(file);
}

// Load the todo list from a file
void load_todo_list(void) {
    FILE *file = fopen("data/todo.txt", "r");
    if (file == NULL) {
        // It's not an error if the file doesn't exist yet
        return;
    }
    
    todo_count = 0;
    char line[MAX_LINE_LENGTH];
    
    while (fgets(line, sizeof(line), file) != NULL && todo_count < MAX_TODO_ITEMS) {
        char *completed_str = strtok(line, "|");
        char *content = strtok(NULL, "\n");
        
        if (completed_str != NULL && content != NULL) {
            todo_list[todo_count].completed = atoi(completed_str);
            strcpy(todo_list[todo_count].content, content);
            todo_count++;
        }
    }
    
    fclose(file);
}

// Todo command - Manage a to-do list
int cmd_todo(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: todo [command] [arguments]\n");
        printf("Manage a to-do list.\n\n");
        printf("Commands:\n");
        printf("  add [text]      Add a new todo item\n");
        printf("  list            List all todo items\n");
        printf("  done [number]   Mark an item as done\n");
        printf("  undone [number] Mark an item as not done\n");
        printf("  delete [number] Delete an item\n");
        printf("  clear           Delete all items\n");
        return 1;
    }
    
    if (strcmp(args[1], "add") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing todo item content\n");
            return 1;
        }
        
        if (todo_count >= MAX_TODO_ITEMS) {
            printf("Error: Todo list is full\n");
            return 1;
        }
        
        // Concatenate all remaining arguments
        char content[MAX_LINE_LENGTH] = "";
        for (int i = 2; args[i] != NULL; i++) {
            strcat(content, args[i]);
            if (args[i + 1] != NULL) {
                strcat(content, " ");
            }
        }
        
        strcpy(todo_list[todo_count].content, content);
        todo_list[todo_count].completed = 0;
        todo_count++;
        
        printf("Todo item added: %s\n", content);
        save_todo_list();
        
    } else if (strcmp(args[1], "list") == 0) {
        if (todo_count == 0) {
            printf("Todo list is empty\n");
            return 1;
        }
        
        printf("\n");
        printf("Todo List:\n");
        printf("----------\n");
        
        for (int i = 0; i < todo_count; i++) {
            if (todo_list[i].completed) {
                printf("[%d] [%s] %s\n", i + 1, "✓", todo_list[i].content);
            } else {
                printf("[%d] [ ] %s\n", i + 1, todo_list[i].content);
            }
        }
        printf("\n");
        
    } else if (strcmp(args[1], "done") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing item number\n");
            return 1;
        }
        
        int item_num = atoi(args[2]) - 1;
        if (item_num < 0 || item_num >= todo_count) {
            printf("Error: Invalid item number\n");
            return 1;
        }
        
        todo_list[item_num].completed = 1;
        printf("Marked item %d as done\n", item_num + 1);
        save_todo_list();
        
    } else if (strcmp(args[1], "undone") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing item number\n");
            return 1;
        }
        
        int item_num = atoi(args[2]) - 1;
        if (item_num < 0 || item_num >= todo_count) {
            printf("Error: Invalid item number\n");
            return 1;
        }
        
        todo_list[item_num].completed = 0;
        printf("Marked item %d as not done\n", item_num + 1);
        save_todo_list();
        
    } else if (strcmp(args[1], "delete") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing item number\n");
            return 1;
        }
        
        int item_num = atoi(args[2]) - 1;
        if (item_num < 0 || item_num >= todo_count) {
            printf("Error: Invalid item number\n");
            return 1;
        }
        
        // Remove the item by shifting all items after it
        for (int i = item_num; i < todo_count - 1; i++) {
            todo_list[i] = todo_list[i + 1];
        }
        todo_count--;
        
        printf("Deleted item %d\n", item_num + 1);
        save_todo_list();
        
    } else if (strcmp(args[1], "clear") == 0) {
        todo_count = 0;
        printf("Todo list cleared\n");
        save_todo_list();
        
    } else {
        printf("Error: Unknown command: %s\n", args[1]);
    }
    
    return 1;
}

// Save notes to a file
void save_notes(void) {
    FILE *file = fopen("data/notes.txt", "w");
    if (file == NULL) {
        perror("Could not open notes file for writing");
        return;
    }
    
    for (int i = 0; i < note_count; i++) {
        // Use a special format that can handle multi-line content
        fprintf(file, "TITLE:%s\n", notes[i].title);
        fprintf(file, "TIMESTAMP:%ld\n", notes[i].timestamp);
        fprintf(file, "CATEGORY:%s\n", notes[i].category);
        fprintf(file, "CONTENT:\n%s\n", notes[i].content);
        fprintf(file, "END_NOTE\n");
    }
    
    fclose(file);
}

// Load notes from a file
void load_notes(void) {
    FILE *file = fopen("data/notes.txt", "r");
    if (file == NULL) {
        // It's not an error if the file doesn't exist yet
        return;
    }
    
    note_count = 0;
    char line[MAX_LINE_LENGTH];
    char content_buffer[MAX_LINE_LENGTH * 10] = "";
    int reading_content = 0;
    
    while (fgets(line, sizeof(line), file) != NULL && note_count < MAX_NOTES) {
        trim_whitespace(line);
        
        if (strncmp(line, "TITLE:", 6) == 0) {
            strcpy(notes[note_count].title, line + 6);
        } else if (strncmp(line, "TIMESTAMP:", 10) == 0) {
            notes[note_count].timestamp = atol(line + 10);
        } else if (strncmp(line, "CATEGORY:", 9) == 0) {
            // Handle category field (may not exist in older files)
            strcpy(notes[note_count].category, line + 9);
        } else if (strcmp(line, "CONTENT:") == 0) {
            reading_content = 1;
            content_buffer[0] = '\0';
        } else if (strcmp(line, "END_NOTE") == 0) {
            reading_content = 0;
            strcpy(notes[note_count].content, content_buffer);
            
            // Set default category if not present
            if (strlen(notes[note_count].category) == 0) {
                strcpy(notes[note_count].category, "General");
            }
            
            note_count++;
        } else if (reading_content) {
            strcat(content_buffer, line);
            strcat(content_buffer, "\n");
        }
    }
    
    fclose(file);
}

// Export note to a text file
void export_note_to_file(int note_num) {
    if (note_num < 0 || note_num >= note_count) {
        printf("Error: Invalid note number\n");
        return;
    }
    
    // Create filename from sanitized title
    char filename[MAX_PATH_LENGTH] = "data/";
    char *title_ptr = notes[note_num].title;
    int j = strlen(filename);
    
    // Copy title to filename, replacing invalid chars
    for (int i = 0; title_ptr[i] != '\0' && j < MAX_PATH_LENGTH - 5; i++) {
        if (isalnum((unsigned char)title_ptr[i]) || title_ptr[i] == ' ' || title_ptr[i] == '-' || title_ptr[i] == '_') {
            if (title_ptr[i] == ' ') {
                filename[j++] = '_';
            } else {
                filename[j++] = title_ptr[i];
            }
        }
    }
    
    // Add extension
    strcat(filename, ".txt");
    
    // Open file for writing
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Could not create export file");
        return;
    }
    
    // Format timestamp
    char time_str[64];
    struct tm *timeinfo = localtime(&notes[note_num].timestamp);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    // Write note data to file
    fprintf(file, "Title: %s\n", notes[note_num].title);
    fprintf(file, "Date: %s\n", time_str);
    fprintf(file, "Category: %s\n", notes[note_num].category);
    fprintf(file, "\n%s\n", notes[note_num].content);
    
    fclose(file);
    
    printf("Note exported to %s\n", filename);
}

// Search through notes
void search_notes(const char *query) {
    if (note_count == 0) {
        printf("No notes to search\n");
        return;
    }
    
    printf("\n");
    printf("Search Results for '%s':\n", query);
    printf("------------------------\n");
    
    int found = 0;
    for (int i = 0; i < note_count; i++) {
        if (strstr(notes[i].title, query) != NULL || 
            strstr(notes[i].content, query) != NULL ||
            strstr(notes[i].category, query) != NULL) {
            
            // Format timestamp
            char time_str[64];
            struct tm *timeinfo = localtime(&notes[i].timestamp);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
            
            printf("[%d] %s (Category: %s, %s)\n", i + 1, notes[i].title, notes[i].category, time_str);
            
            // Print a snippet of the content
            char snippet[81] = {0};
            strncpy(snippet, notes[i].content, 80);
            
            if (strlen(notes[i].content) > 80) {
                strcat(snippet, "...");
            }
            
            printf("    %s\n", snippet);
            found++;
        }
    }
    
    if (found == 0) {
        printf("No notes found matching '%s'\n", query);
    }
    
    printf("\n");
}

// Note command - Create and manage notes
int cmd_note(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: note [command] [arguments]\n");
        printf("Create and manage notes.\n\n");
        printf("Commands:\n");
        printf("  new [title]     Create a new note\n");
        printf("  list            List all notes\n");
        printf("  view [number]   View a specific note\n");
        printf("  edit [number]   Edit a note\n");
        printf("  delete [number] Delete a note\n");
        printf("  search [query]  Search through notes\n");
        printf("  category [number] [category] Set or change note category\n");
        printf("  categories      List all available categories\n");
        printf("  export [number] Export note to a text file\n");
        return 1;
    }
    
    if (strcmp(args[1], "new") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing note title\n");
            return 1;
        }
        
        if (note_count >= MAX_NOTES) {
            printf("Error: Maximum number of notes reached\n");
            return 1;
        }
        
        // Set the title
        strcpy(notes[note_count].title, args[2]);
        
        // Set the default category
        strcpy(notes[note_count].category, "General");
        
        // Check if a category was provided
        if (args[3] != NULL && strcmp(args[3], "--category") == 0 && args[4] != NULL) {
            strcpy(notes[note_count].category, args[4]);
        }
        
        // Set the timestamp
        notes[note_count].timestamp = time(NULL);
        
        // Prompt for content
        printf("Enter note content (end with a line containing only '.' or press Ctrl+D):\n");
        
        // Read multiple lines of content
        char content[MAX_LINE_LENGTH * 10] = "";
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), stdin) != NULL) {
            // Check for the end marker
            if (strcmp(line, ".\n") == 0 || strcmp(line, ".\r\n") == 0) {
                break;
            }
            
            // Add the line to the content, if there's room
            if (strlen(content) + strlen(line) < sizeof(content)) {
                strcat(content, line);
            } else {
                printf("Warning: Note content truncated (too long)\n");
                break;
            }
        }
        
        // Store the content
        strcpy(notes[note_count].content, content);
        
        printf("Note created with title: %s (Category: %s)\n", notes[note_count].title, notes[note_count].category);
        note_count++;
        
        // Save to disk
        save_notes();
        
    } else if (strcmp(args[1], "list") == 0) {
        if (note_count == 0) {
            printf("No notes found\n");
            return 1;
        }
        
        // Check if a category filter was specified
        const char *filter = NULL;
        if (args[2] != NULL) {
            filter = args[2];
        }
        
        printf("\n");
        if (filter != NULL) {
            printf("Notes (Category: %s):\n", filter);
        } else {
            printf("All Notes:\n");
        }
        printf("----------\n");
        
        int count = 0;
        for (int i = 0; i < note_count; i++) {
            // Apply category filter if specified
            if (filter != NULL && strcmp(notes[i].category, filter) != 0) {
                continue;
            }
            
            // Format timestamp
            char time_str[64];
            struct tm *timeinfo = localtime(&notes[i].timestamp);
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
            
            printf("[%d] %s (Category: %s, %s)\n", i + 1, notes[i].title, notes[i].category, time_str);
            count++;
        }
        
        if (count == 0 && filter != NULL) {
            printf("No notes found in category '%s'\n", filter);
        }
        
        printf("\n");
        
    } else if (strcmp(args[1], "view") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing note number\n");
            return 1;
        }
        
        int note_num = atoi(args[2]) - 1;
        if (note_num < 0 || note_num >= note_count) {
            printf("Error: Invalid note number\n");
            return 1;
        }
        
        // Format timestamp
        char time_str[64];
        struct tm *timeinfo = localtime(&notes[note_num].timestamp);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        printf("\n");
        printf("Title: %s\n", notes[note_num].title);
        printf("Date: %s\n", time_str);
        printf("Category: %s\n", notes[note_num].category);
        printf("\n%s\n", notes[note_num].content);
        
    } else if (strcmp(args[1], "edit") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing note number\n");
            return 1;
        }
        
        int note_num = atoi(args[2]) - 1;
        if (note_num < 0 || note_num >= note_count) {
            printf("Error: Invalid note number\n");
            return 1;
        }
        
        printf("Editing note: %s\n", notes[note_num].title);
        
        // Prompt for new title
        printf("New title (leave empty to keep current): ");
        char new_title[MAX_LINE_LENGTH];
        if (fgets(new_title, sizeof(new_title), stdin) != NULL) {
            // Remove newline
            char *newline = strchr(new_title, '\n');
            if (newline) {
                *newline = '\0';
            }
            
            // Update title if not empty
            if (strlen(new_title) > 0) {
                strcpy(notes[note_num].title, new_title);
            }
        }
        
        // Prompt for new category
        printf("New category (leave empty to keep current '%s'): ", notes[note_num].category);
        char new_category[MAX_LINE_LENGTH];
        if (fgets(new_category, sizeof(new_category), stdin) != NULL) {
            // Remove newline
            char *newline = strchr(new_category, '\n');
            if (newline) {
                *newline = '\0';
            }
            
            // Update category if not empty
            if (strlen(new_category) > 0) {
                strcpy(notes[note_num].category, new_category);
            }
        }
        
        // Prompt for new content
        printf("Enter new content (end with a line containing only '.' or press Ctrl+D):\n");
        printf("Current content:\n%s\n", notes[note_num].content);
        
        // Read multiple lines of content
        char content[MAX_LINE_LENGTH * 10] = "";
        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), stdin) != NULL) {
            // Check for the end marker
            if (strcmp(line, ".\n") == 0 || strcmp(line, ".\r\n") == 0) {
                break;
            }
            
            // Add the line to the content, if there's room
            if (strlen(content) + strlen(line) < sizeof(content)) {
                strcat(content, line);
            } else {
                printf("Warning: Note content truncated (too long)\n");
                break;
            }
        }
        
        // Store the content if not empty
        if (strlen(content) > 0) {
            strcpy(notes[note_num].content, content);
        }
        
        // Update timestamp
        notes[note_num].timestamp = time(NULL);
        
        printf("Note updated\n");
        
        // Save to disk
        save_notes();
        
    } else if (strcmp(args[1], "delete") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing note number\n");
            return 1;
        }
        
        int note_num = atoi(args[2]) - 1;
        if (note_num < 0 || note_num >= note_count) {
            printf("Error: Invalid note number\n");
            return 1;
        }
        
        // Remove the note by shifting all notes after it
        for (int i = note_num; i < note_count - 1; i++) {
            notes[i] = notes[i + 1];
        }
        note_count--;
        
        printf("Note deleted\n");
        
        // Save to disk
        save_notes();
        
    } else if (strcmp(args[1], "search") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing search query\n");
            return 1;
        }
        
        search_notes(args[2]);
        
    } else if (strcmp(args[1], "category") == 0) {
        if (args[2] == NULL || args[3] == NULL) {
            printf("Error: Missing note number or category\n");
            printf("Usage: note category [number] [category]\n");
            return 1;
        }
        
        int note_num = atoi(args[2]) - 1;
        if (note_num < 0 || note_num >= note_count) {
            printf("Error: Invalid note number\n");
            return 1;
        }
        
        // Update the category
        strcpy(notes[note_num].category, args[3]);
        
        printf("Category for note '%s' set to '%s'\n", notes[note_num].title, notes[note_num].category);
        
        // Save to disk
        save_notes();
        
    } else if (strcmp(args[1], "categories") == 0) {
        // Find unique categories
        char categories[MAX_NOTES][MAX_LINE_LENGTH];
        int category_count = 0;
        
        for (int i = 0; i < note_count; i++) {
            int found = 0;
            
            // Check if we've already seen this category
            for (int j = 0; j < category_count; j++) {
                if (strcmp(categories[j], notes[i].category) == 0) {
                    found = 1;
                    break;
                }
            }
            
            if (!found && category_count < MAX_NOTES) {
                strcpy(categories[category_count], notes[i].category);
                category_count++;
            }
        }
        
        // Display categories
        printf("\n");
        printf("Available Categories:\n");
        printf("--------------------\n");
        
        if (category_count == 0) {
            printf("No categories found\n");
        } else {
            for (int i = 0; i < category_count; i++) {
                printf("- %s\n", categories[i]);
            }
        }
        
        printf("\n");
        
    } else if (strcmp(args[1], "export") == 0) {
        if (args[2] == NULL) {
            printf("Error: Missing note number\n");
            return 1;
        }
        
        int note_num = atoi(args[2]) - 1;
        export_note_to_file(note_num);
        
    } else {
        printf("Error: Unknown command: %s\n", args[1]);
    }
    
    return 1;
}

// Timer thread function
void *timer_thread(void *arg) {
    int seconds = *((int *)arg);
    free(arg);
    
    for (int i = seconds; i > 0; i--) {
        printf("\rTime remaining: %d seconds", i);
        fflush(stdout);
        sleep(1);
    }
    
    printf("\rTimer completed!                \n");
    
#ifdef _WIN32
    // Play a beep sound on Windows
    printf("\a");
#else
    // Print bell character for terminal beep
    printf("\a");
#endif
    
    return NULL;
}

// Timer command - Set a countdown timer
int cmd_timer(char **args) {
    if (args[1] == NULL || strcmp(args[1], "--help") == 0) {
        printf("Usage: timer [seconds]\n");
        printf("Sets a countdown timer for the specified number of seconds.\n");
        return 1;
    }
    
    int seconds = atoi(args[1]);
    if (seconds <= 0) {
        printf("Error: Please specify a positive number of seconds\n");
        return 1;
    }
    
    // Create a thread to run the timer
    pthread_t thread_id;
    int *arg = malloc(sizeof(int));
    *arg = seconds;
    
    if (pthread_create(&thread_id, NULL, timer_thread, arg) != 0) {
        perror("Failed to create timer thread");
        free(arg);
        return 1;
    }
    
    // Detach the thread so it can clean up itself
    pthread_detach(thread_id);
    
    printf("Timer started for %d seconds\n", seconds);
    
    return 1;
} 
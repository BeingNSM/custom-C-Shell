#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

// Define necessary constants and structures needed by the mathquiz function
#define MAX_LINE_LENGTH 1024
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RESET "\033[0m"

// Stub function for trim_whitespace
void trim_whitespace(char *str) {
    if (str == NULL) return;
    
    // Trim leading whitespace
    char *p = str;
    while (isspace((unsigned char)*p)) p++;
    
    if (p != str) {
        memmove(str, p, strlen(p) + 1);
    }
    
    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

// Test implementation of the mathquiz function
int test_mathquiz(char **args) {
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
            printf("DEBUG: Set difficulty to easy\n");
        } else if (strcmp(args[1], "medium") == 0) {
            difficulty = 1;
            printf("DEBUG: Set difficulty to medium\n");
        } else if (strcmp(args[1], "hard") == 0) {
            difficulty = 2;
            printf("DEBUG: Set difficulty to hard\n");
        } else if (strcmp(args[1], "arithmetic") == 0 || strcmp(args[1], "fractions") == 0 || 
                   strcmp(args[1], "geometry") == 0 || strcmp(args[1], "algebra") == 0 || 
                   strcmp(args[1], "stats") == 0) {
            topic = args[1];
            printf("DEBUG: Set topic to %s\n", topic);
        } else if (strcmp(args[1], "all") != 0) {
            printf("Unknown difficulty level or topic: %s\n", args[1]);
            printf("Available difficulty levels: easy, medium, hard\n");
            printf("Available topics: all, arithmetic, fractions, geometry, algebra, stats\n");
            return 1;
        }
    }
    
    printf("DEBUG: Arguments processed successfully\n");
    
    // Quiz settings
    int num_questions = 2; // Reduced for testing
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
    
    // Test an addition question
    printf("DEBUG: Generating test question\n");
    
    // Variables for question generation
    int num1 = 5, num2 = 7;
    int answer_int = num1 + num2;
    char question[MAX_LINE_LENGTH], answer_str[32];
    
    snprintf(question, sizeof(question), "What is %d + %d?", num1, num2);
    snprintf(answer_str, sizeof(answer_str), "%d", answer_int);
    
    // Ask the question
    printf("Question 1: %s\n", question);
    printf("Your answer: ");
    
    // For testing, we'll simulate the correct answer
    printf("%s\n", answer_str);
    
    printf("Correct!\n\n");
    correct_answers++;
    
    // Test a subtraction question
    num1 = 15;
    num2 = 8;
    answer_int = num1 - num2;
    
    snprintf(question, sizeof(question), "What is %d - %d?", num1, num2);
    snprintf(answer_str, sizeof(answer_str), "%d", answer_int);
    
    // Ask the question
    printf("Question 2: %s\n", question);
    printf("Your answer: ");
    
    // For testing, we'll simulate the correct answer
    printf("%s\n", answer_str);
    
    printf("Correct!\n\n");
    correct_answers++;
    
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
    printf("DEBUG: mathquiz test completed successfully\n");
    
    return 1;
}

int main(int argc, char **argv) {
    printf("Testing mathquiz command...\n");
    
    // Create an arguments array to pass to test_mathquiz
    char *args[3] = {NULL};
    args[0] = "mathquiz";
    
    if (argc > 1) {
        args[1] = argv[1];
    }
    
    if (argc > 2) {
        args[2] = argv[2];
    }
    
    // Call the test function
    test_mathquiz(args);
    
    return 0;
} 
/*
 * Student SpendTrack - Shows !! for expenses that exceed budget
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CATEGORIES 10
#define MAX_DESC 100
#define MAX_DATE 11

typedef struct Expense {
    char category[20];
    float amount;
    char description[MAX_DESC];
    char date[MAX_DATE];
    int exceeded;
    struct Expense *next;
} Expense;

Expense *head = NULL;

typedef struct {
    char name[20];
    float budget;
    float spent;
    int used;
} CategoryBudget;

CategoryBudget categories[MAX_CATEGORIES];

float weeklyTotalBudget = 0;
float weeklyTotalSpent = 0;
char weekStart[MAX_DATE] = "N/A";
int totalExpenseCount = 0;

/* --- Utility Functions --- */
unsigned int hash(const char *str) {
    unsigned int h = 0;
    while (*str) h += (unsigned char)*str++;
    return h % MAX_CATEGORIES;
}

void trimSpaces(char *str) {
    char *start = str;
    char *end;
    while (isspace((unsigned char)*start)) start++;
    if (*start == 0) { str[0] = '\0'; return; }
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    memmove(str, start, (size_t)(end - start + 1));
    str[end - start + 1] = '\0';
}

void normalizeCategory(char *cat) {
    int i;
    trimSpaces(cat);
    if (strlen(cat) == 0) return;
    cat[0] = (char)toupper((unsigned char)cat[0]);
    for (i = 1; cat[i]; i++) cat[i] = (char)tolower((unsigned char)cat[i]);
}

int findCategoryIndex(const char *name) {
    char normalized[20];
    int start, i, idx;
    strcpy(normalized, name);
    normalizeCategory(normalized);
    start = (int)hash(normalized);
    for (i = 0; i < MAX_CATEGORIES; i++) {
        idx = (start + i) % MAX_CATEGORIES;
        if (!categories[idx].used) return -1;
        if (strcmp(categories[idx].name, normalized) == 0) return idx;
    }
    return -1;
}

void setCategoryBudget(const char *name, float budget) {
    char normalized[20];
    int start, i, idx;
    strcpy(normalized, name);
    normalizeCategory(normalized);
    start = (int)hash(normalized);
    for (i = 0; i < MAX_CATEGORIES; i++) {
        idx = (start + i) % MAX_CATEGORIES;
        if (!categories[idx].used || strcmp(categories[idx].name, normalized) == 0) {
            strcpy(categories[idx].name, normalized);
            categories[idx].budget = budget;
            if (!categories[idx].used) {
                categories[idx].spent = 0;
                categories[idx].used = 1;
            }
            return;
        }
    }
}

float getCategoryRemaining(const char *name) {
    int idx = findCategoryIndex(name);
    if (idx == -1) return -1;
    return categories[idx].budget - categories[idx].spent;
}

void addToCategorySpent(const char *name, float amount) {
    int idx = findCategoryIndex(name);
    if (idx != -1) categories[idx].spent += amount;
}

void clearScreen(void) { 
#ifdef _WIN32
    system("cls"); 
#else
    system("clear");
#endif
}

void pressEnter(void) { printf("\nPress Enter to return..."); getchar(); }

/* --- Core Modules --- */

void setupWeeklyBudget(void) {
    char choiceStr[10], cat[20];
    float budget, currentAllocated = 0;
    
    clearScreen();
    printf("+===================================================================+\n");
    printf("|                    CONFIGURATION: WEEKLY BUDGET                   |\n");
    printf("+===================================================================+\n");
    
    do {
        printf(" Week starting (MM-DD-YYYY): ");
        fgets(weekStart, MAX_DATE + 1, stdin);
        weekStart[strcspn(weekStart, "\n")] = 0;
        trimSpaces(weekStart);
        if(strlen(weekStart) == 0) printf(" [!] Date is required.\n");
    } while(strlen(weekStart) == 0);
    
    printf(" Total Weekly Budget Amount: ");
    scanf("%f", &weeklyTotalBudget);
    getchar(); 

    printf(" Set specific category limits? (yes/no): ");
    fgets(choiceStr, 10, stdin);
    
    if (tolower(choiceStr[0]) == 'y') {
        while (1) {
            printf("\n [Remaining Budget: %.2f]\n", weeklyTotalBudget - currentAllocated);
            printf(" Enter Category (or 'done'): ");
            fgets(cat, 20, stdin);
            cat[strcspn(cat, "\n")] = 0;
            trimSpaces(cat);
            
            if (strcmp(cat, "done") == 0) break;
            if (strlen(cat) == 0) continue;
            
            printf(" Limit for %s: ", cat);
            scanf("%f", &budget); getchar();
            
            if (currentAllocated + budget > weeklyTotalBudget) {
                printf(" >> Error: Exceeds total budget!\n");
            } else {
                setCategoryBudget(cat, budget);
                currentAllocated += budget;
            }
        }
    }
    printf("\n Saved Successfully!\n");
    pressEnter();
}

void addExpense(void) {
    if (weeklyTotalBudget == 0) {
        printf("\n [!] Please set your budget first (Option 1).\n");
        pressEnter(); return;
    }

    Expense *newExp = (Expense*)malloc(sizeof(Expense));
    clearScreen();
    printf("+===================================================================+\n");
    printf("|                       RECORD NEW TRANSACTION                      |\n");
    printf("+===================================================================+\n");
    
    do {
        printf(" Category: ");
        fgets(newExp->category, 20, stdin);
        newExp->category[strcspn(newExp->category, "\n")] = 0;
        normalizeCategory(newExp->category);
        if(strlen(newExp->category) == 0) printf(" [!] Category is required.\n");
    } while(strlen(newExp->category) == 0);

    int valid = 0;
    while(!valid) {
        printf(" Amount: ");
        if(scanf("%f", &newExp->amount) != 1) {
            printf(" [!] Please enter a valid number.\n");
            while(getchar() != '\n');
            continue;
        }
        getchar();
        
        float catRem = getCategoryRemaining(newExp->category);
        if (weeklyTotalSpent + newExp->amount > weeklyTotalBudget || (catRem != -1 && newExp->amount > catRem)) {
            printf("\n>> You exceed to your category limits, try again!\n");
        } else {
            valid = 1;
        }
    }

    do {
        printf(" Description: ");
        fgets(newExp->description, MAX_DESC, stdin);
        newExp->description[strcspn(newExp->description, "\n")] = 0;
        trimSpaces(newExp->description);
        if(strlen(newExp->description) == 0) printf(" [!] Description is required.\n");
    } while(strlen(newExp->description) == 0);

    do {
        printf(" Date (MM-DD-YYYY): ");
        fgets(newExp->date, MAX_DATE, stdin);
        newExp->date[strcspn(newExp->date, "\n")] = 0;
        trimSpaces(newExp->date);
        if(strlen(newExp->date) == 0) printf(" [!] Date is required.\n");
    } while(strlen(newExp->date) == 0);

    newExp->next = head;
    head = newExp;
    weeklyTotalSpent += newExp->amount;
    totalExpenseCount++;
    addToCategorySpent(newExp->category, newExp->amount);

    printf("\n Success: Expense added to log.\n");
    pressEnter();
}

void viewAllExpenses(void) {
    clearScreen();
    printf("+===================================================================+\n");
    printf("|                        ALL REGISTERED EXPENSES                    |\n");
    printf("+===================================================================+\n");
    if (!head) {
        printf("|                    (No expenses recorded yet)                     |\n");
    } else {
        printf("| %-12s | %-14s | %-10s | %-20s |\n", "DATE", "CATEGORY", "AMOUNT", "DESCRIPTION");
        printf("|--------------|----------------|------------|----------------------|\n");
        Expense *curr = head;
        while (curr) {
            printf("| %-12s | %-14s | %-10.2f | %-20s |\n", curr->date, curr->category, curr->amount, curr->description);
            curr = curr->next;
        }
    }
    printf("+===================================================================+\n");
    printf("  TOTAL SPENT THIS WEEK: PHP %.2f\n", weeklyTotalSpent);
    pressEnter();
}

void checkBudgetSummary(void) {
    clearScreen();
    printf("+===================================================================+\n");
    printf("|                         BUDGET STATUS REPORT                      |\n");
    printf("+===================================================================+\n");
    printf("| Week Start: %-10s                 Total Budget: %-10.2f |\n", weekStart, weeklyTotalBudget);
    printf("| Spent:      %-10.2f                 Remaining:    %-10.2f |\n", weeklyTotalSpent, weeklyTotalBudget - weeklyTotalSpent);
    printf("+-------------------------------------------------------------------+\n");
    printf("| %-15s | %-12s | %-12s | %-15s |\n", "CATEGORY", "LIMIT", "SPENT", "AVAILABLE");
    printf("|-----------------|--------------|--------------|-------------------|\n");
    
    int hasCat = 0;
    for (int i = 0; i < MAX_CATEGORIES; i++) {
        if (categories[i].used) {
            hasCat = 1;
            printf("| %-15s | %-12.2f | %-12.2f | %-15.2f |\n", 
                    categories[i].name, categories[i].budget, 
                    categories[i].spent, categories[i].budget - categories[i].spent);
        }
    }
    if(!hasCat) printf("|                  (No category limits defined)                     |\n");
    printf("+===================================================================+\n");
    pressEnter();
}

/* --- Main Menu --- */
int main(void) {
    int choice;
    do {
        clearScreen();
        printf("+===================================================================+\n");
        printf("|                    CURRENT STATUS (LIVE TRACKER)                  |\n");
        printf("+===================================================================+\n");
        printf("| Budget: %-8.2f | Spent: %-8.2f | Count: %-3d | Week: %-10s |\n", 
                weeklyTotalBudget, weeklyTotalSpent, totalExpenseCount, weekStart);
        printf("+===================================================================+\n");
        printf("|                   STUDENT SPENDTRACK - MAIN MENU                  |\n");
        printf("+===================================================================+\n");
        printf("|  [1]  Set up weekly budget                                        |\n");
        printf("|  [2]  Add New Expense                                             |\n");
        printf("|  [3]  View All Expenses                                           |\n");
        printf("|  [4]  View Biggest Expenses (Sorted)                              |\n");
        printf("|  [5]  View Budget Summary Table                                   |\n");
        printf("|  [6]  Search Expenses                                             |\n");
        printf("|  [0]  Exit                                                        |\n");
        printf("+===================================================================+\n");
        printf("\n Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n'); continue;
        }
        getchar();
        
        switch(choice) {
            case 1: setupWeeklyBudget(); break;
            case 2: addExpense(); break;
            case 3: viewAllExpenses(); break;
            case 5: checkBudgetSummary(); break;
            case 0: printf("Exiting system...\n"); break;
            default: printf("Invalid option.\n"); pressEnter();
        }
    } while(choice != 0);

    return 0;
}

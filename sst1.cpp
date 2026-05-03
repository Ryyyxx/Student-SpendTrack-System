/*
 * Student SpendTrack - Shows ?? for expenses that exceed budget
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CATEGORIES 10
#define MAX_DESC 100
#define MAX_DATE 11

/* Expense linked list – added 'exceeded' flag */
typedef struct Expense {
    char category[20];
    float amount;
    char description[MAX_DESC];
    char date[MAX_DATE];
    int exceeded;          /* 1 if expense exceeds weekly or category budget */
    struct Expense *next;
} Expense;

Expense *head = NULL;

/* Category budget hash table */
typedef struct {
    char name[20];
    float budget;
    float spent;
    int used;
} CategoryBudget;

CategoryBudget categories[MAX_CATEGORIES];

/* Hash function */
unsigned int hash(const char *str) {
    unsigned int h = 0;
    while (*str) h += *str++;
    return h % MAX_CATEGORIES;
}

/* Trim spaces */
void trimSpaces(char *str) {
    char *start = str;
    char *end;
    while (isspace((unsigned char)*start)) start++;
    if (*start == 0) { str[0] = '\0'; return; }
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

/* Normalize category: trim, first uppercase, rest lowercase */
void normalizeCategory(char *cat) {
    int i;
    trimSpaces(cat);
    if (strlen(cat) == 0) return;
    cat[0] = toupper(cat[0]);
    for (i = 1; cat[i]; i++) cat[i] = tolower(cat[i]);
}

/* Find category index using linear probing */
int findCategoryIndex(const char *name) {
    char normalized[20];
    strcpy(normalized, name);
    normalizeCategory(normalized);
    int start = hash(normalized);
    int i, idx;
    for (i = 0; i < MAX_CATEGORIES; i++) {
        idx = (start + i) % MAX_CATEGORIES;
        if (!categories[idx].used) return -1;
        if (strcmp(categories[idx].name, normalized) == 0) return idx;
    }
    return -1;
}

/* Insert or update category budget */
void setCategoryBudget(const char *name, float budget) {
    char normalized[20];
    strcpy(normalized, name);
    normalizeCategory(normalized);
    int start = hash(normalized);
    int i, idx;
    for (i = 0; i < MAX_CATEGORIES; i++) {
        idx = (start + i) % MAX_CATEGORIES;
        if (!categories[idx].used) break;
        if (strcmp(categories[idx].name, normalized) == 0) {
            categories[idx].budget = budget;
            return;
        }
    }
    if (i < MAX_CATEGORIES) {
        strcpy(categories[idx].name, normalized);
        categories[idx].budget = budget;
        categories[idx].spent = 0;
        categories[idx].used = 1;
    } else {
        printf("Error: Too many categories!\n");
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

float weeklyTotalBudget = 0;
float weeklyTotalSpent = 0;
char weekStart[MAX_DATE];

/* Prototypes */
void clearScreen(void);
void pressEnter(void);
void setupWeeklyBudget(void);
void addExpense(void);
void viewAllExpenses(void);
void viewSortedByAmount(void);
void searchExpense(void);
void checkBudgetSummary(void);
int isValidDate(const char *date);
int isLeapYear(int year);

/* ====================== MAIN ====================== */
int main() {
    int choice, i;
    for (i = 0; i < MAX_CATEGORIES; i++) categories[i].used = 0;
    head = NULL;
    weeklyTotalBudget = 0;
    weeklyTotalSpent = 0;
    
    do {
        clearScreen();
        printf("========================================\n");
        printf("           STUDENT SPENDTRACK\n");
        printf("     Keep your weekly budget alive!\n");
        printf("========================================\n");
        printf("[1] Set up weekly budget\n");
        printf("[2] Add an expense\n");
        printf("[3] View all expenses\n");
        printf("[4] View biggest expenses first (sorted)\n");
        printf("[5] Check remaining budget\n");
        printf("[6] Search for an expense\n");
        printf("[7] Exit\n");
        printf("\nEnter choice: ");
        scanf("%d", &choice);
        getchar();
        
        switch(choice) {
            case 1: setupWeeklyBudget(); break;
            case 2: addExpense(); break;
            case 3: viewAllExpenses(); break;
            case 4: viewSortedByAmount(); break;
            case 5: checkBudgetSummary(); break;
            case 6: searchExpense(); break;
            case 7: printf("Goodbye!\n"); break;
            default: printf("Invalid choice.\n"); pressEnter();
        }
    } while(choice != 7);
    
    while (head) {
        Expense *temp = head;
        head = head->next;
        free(temp);
    }
    return 0;
}

/* ====================== UTILITIES ====================== */
void clearScreen() { system("cls"); }
void pressEnter() { printf("\nPress Enter..."); getchar(); }

int isValidDate(const char *date) {
    int y, m, d;
    int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // 1. Reject if any character is not a digit or hyphen
    for (const char *p = date; *p; p++) {
        char c = *p;
        if (!((c >= '0' && c <= '9') || c == '-'))
            return 0;   // Contains a letter or other invalid character
    }
    
    // 2. Must have exactly 10 characters: YYYY-MM-DD
    int len = 0;
    while (date[len]) len++;
    if (len != 10) return 0;
    
    // 3. Hyphens must be at positions 4 and 7 (0-indexed)
    if (date[4] != '-' || date[7] != '-') return 0;
    
    // 4. Parse using exact widths
    if (sscanf(date, "%4d-%2d-%2d", &y, &m, &d) != 3) return 0;
    
    // 5. Range checks
    if (y < 1900 || y > 2100 || m < 1 || m > 12 || d < 1) return 0;
    
    // 6. Leap year logic (embedded)
    int isLeap = (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    if (isLeap) days[1] = 29;
    
    return d <= days[m - 1];
}
/* ====================== SETUP BUDGET ====================== */
void setupWeeklyBudget() {
    char choice, cat[20];
    float budget;
    int i;
    
    clearScreen();
    printf("=== SET WEEKLY BUDGET ===\n");
    printf("Week starting (YYYY-MM-DD): ");
    fgets(weekStart, MAX_DATE, stdin);
    weekStart[strcspn(weekStart, "\n")] = 0;
    trimSpaces(weekStart);
    if (!isValidDate(weekStart)) strcpy(weekStart, "2026-04-20");
    
    printf("Total budget: ");
    scanf("%f", &weeklyTotalBudget);
    getchar();
    
    for (i = 0; i < MAX_CATEGORIES; i++) categories[i].used = 0;
    
    printf("Set category limits? (y/n): ");
    scanf("%c", &choice);
    getchar();
    
    if (tolower(choice) == 'y') {
        printf("\nEnter category & budget (type 'done' to finish):\n");
        while (1) {
            printf("Category: ");
            fgets(cat, 20, stdin);
            cat[strcspn(cat, "\n")] = 0;
            trimSpaces(cat);
            if (strcmp(cat, "done") == 0) break;
            if (strlen(cat) == 0) continue;
            printf("Budget for %s: ", cat);
            scanf("%f", &budget);
            getchar();
            if (budget <= 0) {
                printf("Budget must be positive.\n");
                continue;
            }
            setCategoryBudget(cat, budget);
        }
    }
    
    weeklyTotalSpent = 0;
    while (head) {
        Expense *temp = head;
        head = head->next;
        free(temp);
    }
    
    printf("\nWeek: %s \nTotal: %.2f\nBudget saved!\n", weekStart, weeklyTotalBudget);
    pressEnter();
}

/* ====================== ADD EXPENSE (SETS EXCEEDED FLAG) ====================== */
void addExpense() {
    if (weeklyTotalBudget == 0) {
        printf("Set budget first!\n");
        pressEnter();
        return;
    }
    
    Expense *newExp = (Expense*)malloc(sizeof(Expense));
    if (!newExp) { printf("Memory error\n"); pressEnter(); return; }
    memset(newExp, 0, sizeof(Expense));
    newExp->exceeded = 0;   /* default not exceeded */
    
    clearScreen();
    printf("=== ADD EXPENSE ===\n");
    printf("Category: ");
    fgets(newExp->category, 20, stdin);
    newExp->category[strcspn(newExp->category, "\n")] = 0;
    normalizeCategory(newExp->category);
    if (strlen(newExp->category) == 0) {
        printf("Category cannot be empty.\n");
        free(newExp);
        pressEnter();
        return;
    }
    
    printf("Amount: ");
    scanf("%f", &newExp->amount);
    getchar();
    if (newExp->amount <= 0) {
        printf("Amount must be positive.\n");
        free(newExp);
        pressEnter();
        return;
    }
    
    printf("Description: ");
    fgets(newExp->description, MAX_DESC, stdin);
    newExp->description[strcspn(newExp->description, "\n")] = 0;
    
    printf("Date (YYYY-MM-DD): ");
    fgets(newExp->date, MAX_DATE, stdin);
    newExp->date[strcspn(newExp->date, "\n")] = 0;
    trimSpaces(newExp->date);
    if (!isValidDate(newExp->date)) strcpy(newExp->date, "2026-04-20");
    
    /* Check if exceeds weekly budget */
    if (weeklyTotalSpent + newExp->amount > weeklyTotalBudget) {
        newExp->exceeded = 1;
        printf("\nWARNING: This expense (%.2f) exceeds your WEEKLY budget!\n", newExp->amount);
        printf("Remaining weekly budget: %.2f\n", weeklyTotalBudget - weeklyTotalSpent);
    }
    
    /* Check if exceeds category budget */
    {
        float catRem = getCategoryRemaining(newExp->category);
        if (catRem != -1 && newExp->amount > catRem) {
            newExp->exceeded = 1;
            printf("\nWARNING: This expense (%.2f) exceeds your %s category budget!\n", newExp->amount, newExp->category);
            printf("Remaining budget: %.2f\n", catRem);
        }
    }
    
    /* Add to linked list */
    newExp->next = head;
    head = newExp;
    
    /* Update totals */
    weeklyTotalSpent += newExp->amount;
    addToCategorySpent(newExp->category, newExp->amount);
    pressEnter();
}

/* ====================== VIEW ALL (with ?? for exceeded) ====================== */
void viewAllExpenses() {
    Expense *curr;
    float total = 0;
    clearScreen();
    printf("======================== ALL EXPENSES ========================\n");
    if (!head) { printf("No expenses.\n"); pressEnter(); return; }
    printf("%-12s | %-14s | %-8s | %s\n", "Date", "Category", "Amount", "Description");
    printf("-------------|----------------|----------|------------------\n");
    curr = head;
    while (curr) {
        if (curr->amount > 0) {
            printf("%-12s | %-14s | ", curr->date, curr->category);
            if (curr->exceeded) {
                printf(" %-7.2f | %s\n", curr->amount, curr->description);
            } else {
                printf(" %-7.2f | %s\n ", curr->amount, curr->description);
            }
            total += curr->amount;
        }
        curr = curr->next;
    }
    printf("\nTotal spent: %.2f\n", total);
    pressEnter();
}

/* ====================== SORTED VIEW (with ?? for exceeded) ====================== */
void viewSortedByAmount() {
    Expense *curr;
    int count = 0;
    int i, j;
    Expense **arr;
    
    clearScreen();
    printf("=== HIGHEST TO LOWEST ===\n");
    if (!head) { printf("No expenses.\n"); pressEnter(); return; }
    
    curr = head;
    while (curr) {
        if (curr->amount > 0) count++;
        curr = curr->next;
    }
    if (count == 0) { printf("No valid expenses.\n"); pressEnter(); return; }
    
    arr = (Expense**)malloc(count * sizeof(Expense*));
    curr = head;
    i = 0;
    while (curr && i < count) {
        if (curr->amount > 0) {
            arr[i] = curr;
            i++;
        }
        curr = curr->next;
    }
    
    for (i = 0; i < count-1; i++)
        for (j = 0; j < count-i-1; j++)
            if (arr[j]->amount < arr[j+1]->amount) {
                Expense *tmp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = tmp;
            }
    
    printf("%-7s | %-14s | %-20s | %s\n", "Amount", "Category", "Description", "Date");
    printf("--------|----------------|----------------------|------------\n");
    for (i = 0; i < count; i++) {
        if (arr[i]->exceeded) {
            printf("%-7.2f | %-13s | %-20s | %s\n", arr[i]->amount, arr[i]->category, arr[i]->description, arr[i]->date);
        } else {
            printf("%-7.2f | %-13s | %-20s | %s\n", arr[i]->amount, arr[i]->category, arr[i]->description, arr[i]->date);
        }
    }
    free(arr);
    pressEnter();
}

/* ====================== BUDGET SUMMARY ====================== */
void checkBudgetSummary() {
    int i;
    int hasCat = 0;
    float left;
    
    clearScreen();
    printf("=== BUDGET SUMMARY ===\n");
    if (weeklyTotalBudget == 0) { printf("No budget set.\n"); pressEnter(); return; }
    
    printf("Week: %s\n", weekStart);
    printf("Total: %.2f | Spent: %.2f | Left: %.2f %s\n",
           weeklyTotalBudget, weeklyTotalSpent, weeklyTotalBudget - weeklyTotalSpent,
           (weeklyTotalBudget - weeklyTotalSpent >= 0) ? " " : " ");
    
    printf("\n--- Categories ---\n");
    for (i = 0; i < MAX_CATEGORIES; i++) {
        if (categories[i].used) {
            hasCat = 1;
            left = categories[i].budget - categories[i].spent;
            printf("%-12s | Budget: %-7.2f | Spent: %-7.2f | Left: %-7.2f %s\n",
                   categories[i].name, categories[i].budget, categories[i].spent, left,
                   (left >= 0) ? " " : " ");
        }
    }
    if (!hasCat) printf("No category limits set.\n");
    pressEnter();
}

/* ====================== SEARCH ====================== */
void searchExpense() {
    int opt;
    char key[50];
    int found = 0;
    Expense *curr;
    int i;
    
    clearScreen();
    printf("=== SEARCH ===\n");
    if (!head) { printf("No expenses.\n"); pressEnter(); return; }
    
    printf("[1] Keyword)\n[2] Category\nChoice: ");
    scanf("%d", &opt);
    getchar();
    
    if (opt == 1) {
        printf("Keyword:");
        fgets(key, 50, stdin);
        key[strcspn(key, "\n")] = 0;
        trimSpaces(key);
        {
            char lowerKey[50];
            strcpy(lowerKey, key);
            for (i = 0; lowerKey[i]; i++) lowerKey[i] = tolower(lowerKey[i]);
            curr = head;
            while (curr) {
                if (curr->amount > 0) {
                    char descLow[MAX_DESC];
                    strcpy(descLow, curr->description);
                    for (i = 0; descLow[i]; i++) descLow[i] = tolower(descLow[i]);
                    if (strstr(descLow, lowerKey)) {
                        printf("%s | %-12s | ", curr->date, curr->category);
                        if (curr->exceeded) printf("%.2f | %s\n", curr->amount, curr->description);
                        else printf("%.2f | %s\n", curr->amount, curr->description);
                        found++;
                    }
                }
                curr = curr->next;
            }
        }
    } else if (opt == 2) {
        printf("Category: ");
        fgets(key, 50, stdin);
        key[strcspn(key, "\n")] = 0;
        normalizeCategory(key);
        curr = head;
        while (curr) {
            if (curr->amount > 0 && strcmp(curr->category, key) == 0) {
                printf("%s | %-12s | ", curr->date, curr->category);
                if (curr->exceeded) printf("%.2f | %s\n", curr->amount, curr->description);
                else printf("%.2f | %s\n", curr->amount, curr->description);
                found++;
            }
            curr = curr->next;
        }
    } else {
        printf("Invalid option.\n");
        pressEnter();
        return;
    }
    
    printf("Found %d expense(s).\n", found);
    pressEnter();
}

/*
 * Features: Array-based Hash Table, Linked List, Bubble Sort, and Linear Search.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CATEGORIES 10
#define MAX_DESC 100
#define MAX_DATE 11

/* --- Data Structures --- */

typedef struct Expense {
    char category[20];
    float amount;
    char description[MAX_DESC];
    char date[MAX_DATE];
    struct Expense *next;
} Expense;

Expense *head = NULL;

typedef struct {
    char name[20];
    float budget;
    float spent;
    int used;
} Category;

Category categories[MAX_CATEGORIES];

/* --- Global Trackers --- */
float totalLimit = 0;
float totalSpent = 0;
char startDate[MAX_DATE] = "N/A";
int itemCount = 0;

/* --- Utility Functions --- */

// Logic: Validates date string for MM-DD-YYYY format and logical ranges
int isValidDate(const char *date) {
    if (strlen(date) != 10) return 0;
    int i;
    for (i = 0; i < 10; i++) {
        if (i == 2 || i == 5) {
            if (date[i] != '-') return 0;
        } else {
            if (!isdigit((unsigned char)date[i])) return 0;
        }
    }
    int m = atoi(date);
    int d = atoi(date + 3);
    int y = atoi(date + 6);
    
    if (m < 1 || m > 12) return 0;
    if (d < 1 || d > 31) return 0;
    if (y < 2000 || y > 2100) return 0; // Practical range check
    return 1;
}

int isMatch(const char *text, const char *find) {
    if (!text || !find) return 0;
    size_t tLen = strlen(text);
    size_t fLen = strlen(find);
    if (fLen > tLen) return 0;
    size_t i;
    for (i = 0; i <= tLen - fLen; i++) {
        size_t j;
        for (j = 0; j < fLen; j++) {
            if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)find[j]))
                break;
        }
        if (j == fLen) return 1;
    }
    return 0;
}

unsigned int getHash(const char *str) {
    unsigned int h = 0;
    while (*str) h += (unsigned char)*str++;
    return h % MAX_CATEGORIES;
}

void cleanStr(char *str) {
    char *start = str;
    char *end;
    while (isspace((unsigned char)*start)) start++;
    if (*start == 0) { str[0] = '\0'; return; }
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    memmove(str, start, (size_t)(end - start + 1));
    str[end - start + 1] = '\0';
}

void fixCategory(char *cat) {
    int i;
    cleanStr(cat);
    if (strlen(cat) == 0) return;
    cat[0] = (char)toupper((unsigned char)cat[0]);
    for (i = 1; cat[i]; i++) cat[i] = (char)tolower((unsigned char)cat[i]);
}

int findIndex(const char *name) {
    char temp[20];
    int start, i, idx;
    strcpy(temp, name);
    fixCategory(temp);
    start = (int)getHash(temp);
    for (i = 0; i < MAX_CATEGORIES; i++) {
        idx = (start + i) % MAX_CATEGORIES;
        if (!categories[idx].used) return -1;
        if (strcmp(categories[idx].name, temp) == 0) return idx;
    }
    return -1;
}

void setBudget(const char *name, float amount) {
    char temp[20];
    int start, i, idx;
    strcpy(temp, name);
    fixCategory(temp);
    start = (int)getHash(temp);
    for (i = 0; i < MAX_CATEGORIES; i++) {
        idx = (start + i) % MAX_CATEGORIES;
        if (!categories[idx].used || strcmp(categories[idx].name, temp) == 0) {
            strcpy(categories[idx].name, temp);
            categories[idx].budget = amount;
            if (!categories[idx].used) {
                categories[idx].spent = 0;
                categories[idx].used = 1;
            }
            return;
        }
    }
}

void addSpent(const char *name, float amount) {
    int idx = findIndex(name);
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

void freeMemory(void) {
    Expense *curr = head;
    while (curr != NULL) {
        Expense *temp = curr;
        curr = curr->next;
        free(temp);
    }
    head = NULL;
}

/* 
 * OPTION [1]: Set up weekly budget
 */
void setupBudget(void) {
    char choice[10], catName[20];
    float limit, currentSum = 0;
    int i;
    
    freeMemory(); 
    totalSpent = 0;
    itemCount = 0;
    for (i = 0; i < MAX_CATEGORIES; i++) {
        categories[i].used = 0;
        categories[i].spent = 0;
    }

    clearScreen();
    printf("+===================================================================+\n");
    printf("|                    CONFIGURATION: NEW WEEKLY BUDGET               |\n");
    printf("+===================================================================+\n");
    
    do {
        printf(" Week starting (MM-DD-YYYY): ");
        fgets(startDate, MAX_DATE + 1, stdin);
        startDate[strcspn(startDate, "\n")] = 0;
        cleanStr(startDate);
        if(!isValidDate(startDate)) printf(" >> Error: Use format MM-DD-YYYY (e.g., 05-20-2024)\n");
    } while(!isValidDate(startDate));
    
    printf(" Total Weekly Budget Amount: ");
    scanf("%f", &totalLimit);
    getchar(); 

    printf(" Set specific category limits? (y/n): ");
    fgets(choice, 10, stdin);
    
    if (tolower(choice[0]) == 'y') {
        while (1) {
            printf("\n [Remaining to Allocate: %.2f]\n", totalLimit - currentSum);
            printf(" Enter Category (or 'done'): ");
            fgets(catName, 20, stdin);
            catName[strcspn(catName, "\n")] = 0;
            cleanStr(catName);
            
            if (strcmp(catName, "done") == 0) break;
            if (strlen(catName) == 0) continue;
            
            printf(" Limit for %s: ", catName);
            scanf("%f", &limit); getchar();
            
            if (currentSum + limit > totalLimit) {
                printf(" >> Error: Exceeds total budget!\n");
            } else {
                setBudget(catName, limit);
                currentSum += limit;
            }
        }
    }
    printf("\n Saved! New week started.\n");
    pressEnter();
}

/* 
 * OPTION [2]: Add New Expense
 */
void addExpense(void) {
    if (totalLimit == 0) {
        printf("\n [!] Please set your budget first (Option 1).\n");
        pressEnter(); return;
    }

    Expense *node = (Expense*)malloc(sizeof(Expense));
    if(!node) return;
    clearScreen();
    printf("+===================================================================+\n");
    printf("|                        RECORD NEW TRANSACTION                     |\n");
    printf("+===================================================================+\n");
    
    do {
        printf(" Category: ");
        fgets(node->category, 20, stdin);
        node->category[strcspn(node->category, "\n")] = 0;
        fixCategory(node->category);
    } while(strlen(node->category) == 0);

    int idx = findIndex(node->category);
    while (1) {
        printf(" Amount: ");
        if (scanf("%f", &node->amount) != 1) {
            printf(" >> Error: Invalid number.\n");
            while(getchar() != '\n'); continue;
        }
        getchar();
        if (node->amount <= 0) {
            printf(" >> Error: Amount must be positive.\n");
            continue;
        }
        if (idx != -1) {
            float available = categories[idx].budget - categories[idx].spent;
            if (node->amount > available) {
                printf(" >> [!] You exceed the data limit. (Available: %.2f)\n", available);
                continue;
            }
        } else if (totalSpent + node->amount > totalLimit) {
             printf(" >> [!] You exceed overall limit. (Remaining: %.2f)\n", totalLimit - totalSpent);
             continue;
        }
        break; 
    }
    
    printf(" Description: ");
    fgets(node->description, MAX_DESC, stdin);
    node->description[strcspn(node->description, "\n")] = 0;

    do {
        printf(" Date (MM-DD-YYYY): ");
        fgets(node->date, MAX_DATE, stdin);
        node->date[strcspn(node->date, "\n")] = 0;
        cleanStr(node->date);
        if(!isValidDate(node->date)) printf(" >> Error: Use format MM-DD-YYYY\n");
    } while(!isValidDate(node->date));

    node->next = head;
    head = node;
    totalSpent += node->amount;
    itemCount++;
    addSpent(node->category, node->amount);

    printf("\n Success: Expense added.\n");
    pressEnter();
}

void showAll(void) {
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
    printf("  TOTAL SPENT THIS WEEK: PHP %.2f\n", totalSpent);
    pressEnter();
}

void showSorted(void) {
    if (!head) { printf("\n [!] No records to sort.\n"); pressEnter(); return; }
    int swapped;
    Expense *ptr;
    Expense *last = NULL;
    do {
        swapped = 0;
        ptr = head;
        while (ptr->next != last) {
            if (ptr->amount < ptr->next->amount) {
                char tCat[20]; strcpy(tCat, ptr->category);
                strcpy(ptr->category, ptr->next->category); strcpy(ptr->next->category, tCat);
                float tAmt = ptr->amount; ptr->amount = ptr->next->amount; ptr->next->amount = tAmt;
                char tDesc[MAX_DESC]; strcpy(tDesc, ptr->description);
                strcpy(ptr->description, ptr->next->description); strcpy(ptr->next->description, tDesc);
                char tDate[MAX_DATE]; strcpy(tDate, ptr->date);
                strcpy(ptr->date, ptr->next->date); strcpy(ptr->next->date, tDate);
                swapped = 1;
            }
            ptr = ptr->next;
        }
        last = ptr;
    } while (swapped);
    showAll();
}

void showSummary(void) {
	int i;
    clearScreen();
    printf("+===================================================================+\n");
    printf("|                          BUDGET STATUS REPORT                     |\n");
    printf("+===================================================================+\n");
    printf("| Week Start: %-10s                 Total Budget: %-10.2f |\n", startDate, totalLimit);
    printf("| Spent:      %-10.2f                   Remaining:    %-10.2f |\n", totalSpent, totalLimit - totalSpent);
    printf("+-------------------------------------------------------------------+\n");
    printf("| %-15s | %-12s | %-12s | %-15s |\n", "CATEGORY", "LIMIT", "SPENT", "AVAILABLE");
    printf("|-----------------|--------------|--------------|-------------------|\n");
    for (i = 0; i < MAX_CATEGORIES; i++) {
        if (categories[i].used) {
            printf("| %-15s | %-12.2f | %-12.2f | %-15.2f |\n", 
                    categories[i].name, categories[i].budget, 
                    categories[i].spent, categories[i].budget - categories[i].spent);
        }
    }
    printf("+===================================================================+\n");
    pressEnter();
}

void runSearch(void) {
    char term[MAX_DESC];
    clearScreen();
    printf(" Enter search term: ");
    fgets(term, MAX_DESC, stdin);
    term[strcspn(term, "\n")] = 0;
    printf("\n| %-12s | %-14s | %-10s | %-20s |\n", "DATE", "CATEGORY", "AMOUNT", "DESCRIPTION");
    printf("|--------------|----------------|------------|----------------------|\n");
    int found = 0;
    Expense *curr = head;
    while (curr) {
        if (isMatch(curr->category, term) || isMatch(curr->description, term)) {
            printf("| %-12s | %-14s | %-10.2f | %-20s |\n", curr->date, curr->category, curr->amount, curr->description);
            found = 1;
        }
        curr = curr->next;
    }
    if (!found) printf("|                    No matching records found.                     |\n");
    printf("+===================================================================+\n");
    pressEnter();
}

int main(void) {
    int choice;
    do {
        clearScreen();
        printf("+===================================================================+\n");
        printf("|                    CURRENT STATUS (LIVE TRACKER)                  |\n");
        printf("+===================================================================+\n");
        printf("| Budget: %-8.2f | Spent: %-8.2f | Count: %-3d | Week: %-10s |\n", 
                totalLimit, totalSpent, itemCount, startDate);
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
        if (scanf("%d", &choice) != 1) { while(getchar() != '\n'); continue; }
        getchar();
        switch(choice) {
            case 1: setupBudget(); break;
            case 2: addExpense(); break;
            case 3: showAll(); break;
            case 4: showSorted(); break;
            case 5: showSummary(); break;
            case 6: runSearch(); break;
            case 0: freeMemory(); printf("Exiting system...\n"); break;
        }
    } while(choice != 0);
    return 0;
}

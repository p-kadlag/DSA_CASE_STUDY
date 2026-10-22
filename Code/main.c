#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TRANSACTIONS 5
#define DATA_FILE "bank_accounts.dat"

// Structure to store a single transaction
typedef struct {
    char type[15]; // "Deposit", "Withdrawal", "Opening Bal"
    double amount;
    double balance_after;
} Transaction;

// Structure to store Account details
typedef struct {
    int account_number;
    char name[50];
    double balance;
    Transaction history[MAX_TRANSACTIONS];
    int transaction_count; // Tracks the total number of transactions done
} Account;

// Function prototypes
void create_account();
void deposit();
void withdraw();
void print_mini_statement();
void delete_account();
Account find_account(int acc_num, int *found);
void update_account(Account acc);

int main() {
    int choice;

    while (1) {
        printf("\n=== BANKING TRANSACTION SYSTEM ===\n");
        printf("1. Create New Account\n");
        printf("2. Deposit Money\n");
        printf("3. Withdraw Money\n");
        printf("4. Print Mini Statement (and Export)\n");
        printf("5. Delete Account\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input! Exiting.\n");
            exit(1);
        }

        switch (choice) {
            case 1: create_account(); break;
            case 2: deposit(); break;
            case 3: withdraw(); break;
            case 4: print_mini_statement(); break;
            case 5: delete_account(); break;
            case 6: printf("Thank you for using our system!\n"); exit(0);
            default: printf("Invalid choice! Please try again.\n");
        }
    }
    return 0;
}

// Function to create a new account and save it to the file
void create_account() {
    FILE *file = fopen(DATA_FILE, "ab");
    if (!file) {
        printf("Error opening file!\n");
        return;
    }

    Account acc;
    int check_found = 0;

    printf("\nEnter Account Number: ");
    scanf("%d", &acc.account_number);

    // Check if account already exists
    find_account(acc.account_number, &check_found);
    if (check_found) {
        printf("Account number already exists! Try a different one.\n");
        fclose(file);
        return;
    }

    printf("Enter Account Holder Name: ");
    scanf(" %[^\n]s", acc.name); // Reads strings with spaces
    printf("Enter Initial Balance: ");
    scanf("%lf", &acc.balance);

    acc.transaction_count = 0;

    // Log the initial opening balance as the first transaction
    strcpy(acc.history[0].type, "Opening Bal");
    acc.history[0].amount = acc.balance;
    acc.history[0].balance_after = acc.balance;
    acc.transaction_count = 1;

    fwrite(&acc, sizeof(Account), 1, file);
    fclose(file);

    printf("Account created successfully!\n");
}

// Helper function to search for an account in the file
Account find_account(int acc_num, int *found) {
    FILE *file = fopen(DATA_FILE, "rb");
    Account acc;
    *found = 0;

    if (!file) return acc;

    while (fread(&acc, sizeof(Account), 1, file)) {
        if (acc.account_number == acc_num) {
            *found = 1;
            break;
        }
    }
    fclose(file);
    return acc;
}

// Helper function to overwrite an updated account record back to the file
void update_account(Account updated_acc) {
    FILE *file = fopen(DATA_FILE, "rb+");
    if (!file) return;

    Account acc;
    long pos;

    while (fread(&acc, sizeof(Account), 1, file)) {
        if (acc.account_number == updated_acc.account_number) {
            pos = ftell(file) - sizeof(Account);
            fseek(file, pos, SEEK_SET);
            fwrite(&updated_acc, sizeof(Account), 1, file);
            break;
        }
    }
    fclose(file);
}

// Function to handle deposits
void deposit() {
    int acc_num, found;
    double amount;

    printf("\nEnter Account Number: ");
    scanf("%d", &acc_num);

    Account acc = find_account(acc_num, &found);
    if (!found) {
        printf("Account not found!\n");
        return;
    }

    printf("Enter Amount to Deposit: ");
    scanf("%lf", &amount);

    if (amount <= 0) {
        printf("Invalid amount!\n");
        return;
    }

    acc.balance += amount;

    // Shift history elements to maintain circular rolling log of last 5 items
    int idx = acc.transaction_count < MAX_TRANSACTIONS ? acc.transaction_count : MAX_TRANSACTIONS - 1;
    if (acc.transaction_count >= MAX_TRANSACTIONS) {
        for (int i = 0; i < MAX_TRANSACTIONS - 1; i++) {
            acc.history[i] = acc.history[i + 1];
        }
    }

    strcpy(acc.history[idx].type, "Deposit");
    acc.history[idx].amount = amount;
    acc.history[idx].balance_after = acc.balance;
    acc.transaction_count++;

    update_account(acc);
    printf("Successfully deposited $%.2f. New Balance: $%.2f\n", amount, acc.balance);
}

// Function to handle withdrawals
void withdraw() {
    int acc_num, found;
    double amount;

    printf("\nEnter Account Number: ");
    scanf("%d", &acc_num);

    Account acc = find_account(acc_num, &found);
    if (!found) {
        printf("Account not found!\n");
        return;
    }

    printf("Enter Amount to Withdraw: ");
    scanf("%lf", &amount);

    if (amount <= 0) {
        printf("Invalid amount!\n");
        return;
    }
    if (amount > acc.balance) {
        printf("Insufficient balance! Current Balance: $%.2f\n", acc.balance);
        return;
    }

    acc.balance -= amount;

    // Shift history elements to maintain circular rolling log of last 5 items
    int idx = acc.transaction_count < MAX_TRANSACTIONS ? acc.transaction_count : MAX_TRANSACTIONS - 1;
    if (acc.transaction_count >= MAX_TRANSACTIONS) {
        for (int i = 0; i < MAX_TRANSACTIONS - 1; i++) {
            acc.history[i] = acc.history[i + 1];
        }
    }

    strcpy(acc.history[idx].type, "Withdrawal");
    acc.history[idx].amount = amount;
    acc.history[idx].balance_after = acc.balance;
    acc.transaction_count++;

    update_account(acc);
    printf("Successfully withdrew $%.2f. New Balance: $%.2f\n", amount, acc.balance);
}

// Function to display the rolling mini statement and export to a text file
void print_mini_statement() {
    int acc_num, found;

    printf("\nEnter Account Number: ");
    scanf("%d", &acc_num);

    Account acc = find_account(acc_num, &found);
    if (!found) {
        printf("Account not found!\n");
        return;
    }

    // Display in terminal
    printf("\n================ MINI STATEMENT ================\n");
    printf("Account Number: %d\n", acc.account_number);
    printf("Account Holder: %s\n", acc.name);
    printf("Current Balance: $%.2f\n", acc.balance);
    printf("------------------------------------------------\n");
    printf("%-15s | %-12s | %-15s\n", "Txn Type", "Amount", "Closing Balance");
    printf("------------------------------------------------\n");

    int limit = acc.transaction_count < MAX_TRANSACTIONS ? acc.transaction_count : MAX_TRANSACTIONS;
    for (int i = 0; i < limit; i++) {
        printf("%-15s | $%-11.2f | $%-14.2f\n", 
               acc.history[i].type, 
               acc.history[i].amount, 
               acc.history[i].balance_after);
    }
    printf("================================================\n");

    // Generate Text File Statement
    char filename[30];
    sprintf(filename, "statement_%d.txt", acc.account_number);
    FILE *txt_file = fopen(filename, "w");
    if (txt_file) {
        fprintf(txt_file, "================ MINI STATEMENT ================\n");
        fprintf(txt_file, "Account Number: %d\n", acc.account_number);
        fprintf(txt_file, "Account Holder: %s\n", acc.name);
        fprintf(txt_file, "Current Balance: $%.2f\n", acc.balance);
        fprintf(txt_file, "------------------------------------------------\n");
        fprintf(txt_file, "%-15s | %-12s | %-15s\n", "Txn Type", "Amount", "Closing Balance");
        fprintf(txt_file, "------------------------------------------------\n");

        for (int i = 0; i < limit; i++) {
            fprintf(txt_file, "%-15s | $%-11.2f | $%-14.2f\n", 
                   acc.history[i].type, 
                   acc.history[i].amount, 
                   acc.history[i].balance_after);
        }
        fprintf(txt_file, "================================================\n");
        fclose(txt_file);
        printf("Statement successfully exported to file: %s\n", filename);
    } else {
        printf("Error: Could not generate statement text file.\n");
    }
}

// Function to delete an account record from the database
void delete_account() {
    int acc_num, found;

    printf("\nEnter Account Number to Delete: ");
    scanf("%d", &acc_num);

    find_account(acc_num, &found);
    if (!found) {
        printf("Account not found! Cannot delete.\n");
        return;
    }

    FILE *file = fopen(DATA_FILE, "rb");
    FILE *temp_file = fopen("temp.dat", "wb");

    if (!file || !temp_file) {
        printf("Error managing system data files!\n");
        if (file) fclose(file);
        if (temp_file) fclose(temp_file);
        return;
    }

    Account acc;
    int deleted = 0;

    // Read all records and write them to the temp file EXCEPT the one to be deleted
    while (fread(&acc, sizeof(Account), 1, file)) {
        if (acc.account_number == acc_num) {
            deleted = 1; // Skip this record
        } else {
            fwrite(&acc, sizeof(Account), 1, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    // Replace the old database file with the newly generated updated file
    remove(DATA_FILE);
    rename("temp.dat", DATA_FILE);

    if (deleted) {
        printf("Account %d deleted successfully from system records.\n", acc_num);
    } else {
        printf("Error finalizing account deletion.\n");
    }
}

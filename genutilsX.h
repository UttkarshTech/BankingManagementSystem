#ifndef GENUTILSX_H
#define GENUTILSX_H

//header files
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    
#include <fcntl.h>     
#include <string.h>    
#include <stdlib.h>    
#include <sys/types.h> 
#include <sys/stat.h>
#include <arpa/inet.h>
#include <pthread.h>

//constants

#define SERVER_PORT 8001
#define BUFFER_LEN 1024
#define SERVER_IP "127.0.0.1"

#define RC 20
#define RA 21
#define RM 22
#define RE 23

#define LAPPLIED 0
#define LASSIGNED 1
#define LAPPROVED 2
#define LREJECTED -1

#define BACKLOG 5
#define MAXSTR 50
#define FEEDBACKLEN 2048

#define USERDETAILSFILE "users.dat"
#define TXNSFILE "txns.dat"
#define FBFILE "feedbacks.dat"
#define LOANSFILE "loans.dat"
#define LOANSIDFILE "loanid.dat"


#define ACCOUNTDISABLEDMSG "Your account was disabled, please contact your bank manager!\n"


//structs
struct User {
    char username[MAXSTR];
    char password[MAXSTR];
    char firstName[MAXSTR];
    char lastName[MAXSTR];
    char accountNum[MAXSTR];
    int isActive;
    int isLoggedIn;
    int role;
    float balance;
};

struct Txn {
    char sender[MAXSTR];
    char receiver[MAXSTR];
    float amt;
    float senderBalAfterTxn;
    float receiverBalAfterTxn;
};

struct Fback {
    char username[MAXSTR];
    char feedback[FEEDBACKLEN];
};

struct Loan {
    int loanID;
    char username[MAXSTR];
    char manager[MAXSTR];
    char employee[MAXSTR];
    float loanAmount;
    int status;
};


//funcs
void safe_write(int fd, const char *msg);

int safe_read_line(int fd, char *buffer, int size);

void apply_lock(int fd, int lock_type);

struct User verifyLogin(int client_fd, struct User *curUserPtr);

struct User logout_server(int client_fd, struct User *curUserPtr);

struct User getDetails_server(int client_fd, struct User *curUserPtr);

struct User deposit_server(int client_fd, struct User *curUserPtr, float amt);

struct User withdraw_server(int client_fd, struct User *curUserPtr, float amt);

int validateUsername(char *username);

int validateUsernameAndRole(char *username, int role);

int validateWithdrawal(char *username, float amount);

struct User makeTxn(struct User *curUserPtr, float amount, char rcvr[MAXSTR]);

void getTxnDetails(int client_fd, struct User *curUserPtr);

struct User changePassword_server(struct User *curUserPtr, char newPassword[1024]);

void addFeedback(char username[MAXSTR], char feedback[FEEDBACKLEN]);

int addLoan(char username[MAXSTR], char manager[MAXSTR], float loanAmount);

void changeCustomerStatus(char username[MAXSTR], int stat);

void viewAllLoanApplications(int client_fd);

void viewAllFeedbacks (int client_fd);

int validateLoanID(int loanID);

int assignLoan(int loanID, char employee[MAXSTR]);

int checkUniqueUsername(char username[MAXSTR]);

int addNewCustomer(struct User *newCustomerPtr);

int modifyCustomer(char username[MAXSTR], char firstName[MAXSTR], char lastName[MAXSTR]);

void viewAllAssignedLoanApplications(char username[MAXSTR], int client_fd);

int loanEmployeeCheck(int loanID, char username[MAXSTR]);

int changeStatLoan(int loanID, int stat);

int modifyRole(char username[MAXSTR], int role);

void getAllTxnDetails(int client_fd);

#endif
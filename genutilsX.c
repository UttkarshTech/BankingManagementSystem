#include "genutilsX.h"

void safe_write(int fd, const char *msg) {
    write(fd, msg, strlen(msg));
}

int safe_read_line(int fd, char *buffer, int size) {
    ssize_t bytes_read = read(fd, buffer, size - 1);
    if (bytes_read <= 0) {
        buffer[0] = '\0';
        return -1;
    }
    buffer[bytes_read] = '\0';
    char *newline = strchr(buffer, '\n');
    if (newline) {
        *newline = '\0';
    }
    return bytes_read;
}

void apply_lock(int fd, int lock_type) {
    struct flock lock;
    lock.l_type = lock_type;
    lock.l_whence = SEEK_SET; 
    lock.l_start = 0;         
    lock.l_len = 0;    
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        safe_write(STDERR_FILENO, "Error: Failed to acquire file lock.\n");
        exit(1);
    }
}

struct User verifyLogin(int client_fd, struct User *curUserPtr) {
    int authState;
    read(client_fd, curUserPtr, sizeof(*curUserPtr));

    int fd = open(USERDETAILSFILE, O_RDWR);
    apply_lock(fd, F_WRLCK);

    struct User record = { .isActive = 1, .isLoggedIn = 0 };
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue; // Skip this record
        }
        // Compare the stored record with the user's input
        if (strcmp(curUserPtr->username, record.username) == 0 &&
            strcmp(curUserPtr->password, record.password) == 0) {
            if (!record.isActive){
                apply_lock(fd, F_UNLCK);
                close(fd);
                send(client_fd, &record, sizeof(struct User), 0);
                return record;
            }
            if (record.isLoggedIn){
                record.isLoggedIn = 2;
                apply_lock(fd, F_UNLCK);
                close(fd);
                send(client_fd, &record, sizeof(struct User), 0);
                return record;
            }
            record.isLoggedIn = 1;
            lseek(fd, -1*sizeof(record), SEEK_CUR);
            write(fd, &record, sizeof(record));
            *curUserPtr = record;
            break; // Stop reading once the user is found
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    send(client_fd, curUserPtr, sizeof(struct User), 0);
    return *curUserPtr;
}

struct User logout_server(int client_fd, struct User *curUserPtr){
    read(client_fd, curUserPtr, sizeof(struct User));
    printf("User %s chose to log off\n", curUserPtr->username);

    int fd = open(USERDETAILSFILE, O_RDWR);
    apply_lock(fd, F_WRLCK);
    struct User record;
    ssize_t bytes_read;
    int found = 0;

    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(curUserPtr->username, record.username) == 0) {
            record.isLoggedIn = 0;
            lseek(fd, -1*sizeof(record), SEEK_CUR);
            write(fd, &record, sizeof(record));
            *curUserPtr = record;
            found = 1;
            break;
        }
    }

    apply_lock(fd, F_UNLCK);
    close(fd);
    return *curUserPtr;
}

struct User getDetails_server(int client_fd, struct User *curUserPtr){
    read(client_fd, curUserPtr, sizeof(struct User));
    printf("User %s chose to view account details\n", curUserPtr->username);

    int fd = open(USERDETAILSFILE, O_RDONLY);
    apply_lock(fd, F_RDLCK);
    struct User record;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(curUserPtr->username, record.username) == 0) {
            *curUserPtr = record;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return *curUserPtr;
}

struct User deposit_server(int client_fd, struct User *curUserPtr, float amt){
    read(client_fd, curUserPtr, sizeof(struct User));
    printf("User %s chose to deposit\n", curUserPtr->username);

    int fd1 = open(TXNSFILE, O_WRONLY | O_APPEND);
    int fd2 = open(USERDETAILSFILE, O_RDWR);
    apply_lock(fd2, F_WRLCK);
    struct User record;
    struct Txn txnRecord = { .amt = amt };
    strcpy(txnRecord.sender, curUserPtr->username);
    strcpy(txnRecord.receiver, curUserPtr->username);
    ssize_t bytes_read;

    while ((bytes_read = read(fd2, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(curUserPtr->username, record.username) == 0) {
            if (record.isActive){
                //log the deposit txn
                record.balance += amt;
                apply_lock(fd1, F_WRLCK);
                txnRecord.senderBalAfterTxn = record.balance;
                txnRecord.receiverBalAfterTxn = record.balance;
                write(fd1, &txnRecord, sizeof(struct Txn));
                apply_lock(fd1, F_UNLCK);

                lseek(fd2, -1*sizeof(record), SEEK_CUR);
                write(fd2, &record, sizeof(record));
            }
            *curUserPtr = record;
            break;
        }
    }
    apply_lock(fd2, F_UNLCK);
    close(fd1);
    close(fd2);
    return *curUserPtr;
}

struct User withdraw_server(int client_fd, struct User *curUserPtr, float amt){
    read(client_fd, curUserPtr, sizeof(struct User));
    printf("User %s chose to withdraw\n", curUserPtr->username);

    int fd1 = open(TXNSFILE, O_WRONLY | O_APPEND);
    int fd2 = open(USERDETAILSFILE, O_RDWR);
    apply_lock(fd2, F_WRLCK);
    struct User record;
    struct Txn txnRecord = { .amt = amt };
    strcpy(txnRecord.sender, curUserPtr->username);
    strcpy(txnRecord.receiver, curUserPtr->username);
    ssize_t bytes_read;

    while ((bytes_read = read(fd2, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(curUserPtr->username, record.username) == 0) {
            if (record.isActive && record.balance >= amt){
                //log the deposit txn
                record.balance -= amt;
                apply_lock(fd1, F_WRLCK);
                txnRecord.senderBalAfterTxn = record.balance;
                txnRecord.receiverBalAfterTxn = record.balance;
                write(fd1, &txnRecord, sizeof(struct Txn));
                apply_lock(fd1, F_UNLCK);

                lseek(fd2, -1*sizeof(record), SEEK_CUR);
                write(fd2, &record, sizeof(record));
            }
            *curUserPtr = record;
            break;
        }
    }
    apply_lock(fd2, F_UNLCK);
    close(fd1);
    close(fd2);
    return *curUserPtr;
}

int validateUsername(char *username){
    int fd = open(USERDETAILSFILE, O_RDONLY);
    apply_lock(fd, F_RDLCK);
    struct User record;
    ssize_t bytes_read;
    int flag = 0;
    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(username, record.username) == 0) {
            if (record.isActive) flag = 1;
            else flag = -1;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return flag;
}

int validateUsernameAndRole(char *username, int role){
    int fd = open(USERDETAILSFILE, O_RDONLY);
    apply_lock(fd, F_RDLCK);
    struct User record;
    ssize_t bytes_read;
    int flag = 0;
    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(username, record.username) == 0) {
            if (record.isActive && record.role == role) flag = 1;
            else flag = -1;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return flag;
}

int validateWithdrawal(char *username, float amount){
    int fd = open(USERDETAILSFILE, O_RDONLY);
    apply_lock(fd, F_RDLCK);
    struct User record;
    ssize_t bytes_read;
    int flag = 0;
    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(username, record.username) == 0) {
            if (record.isActive){
                if (record.balance >= amount) flag = 1;
                else flag = -2;
            } else flag = -1;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return flag;
}

struct User makeTxn(struct User *curUserPtr, float amount, char rcvr[MAXSTR]){
    int fd1 = open(USERDETAILSFILE, O_RDWR);
    int fd2 = open(TXNSFILE, O_WRONLY | O_APPEND);
    struct User sender, receiver;
    struct Txn txnRecord = { .amt = amount };
    strcpy(txnRecord.sender, curUserPtr->username);
    strcpy(txnRecord.receiver, rcvr);
    ssize_t bytes_read;

    apply_lock(fd1, F_WRLCK);
    apply_lock(fd2, F_WRLCK);
    
    while ((bytes_read = read(fd1, &sender, sizeof(sender))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(sender)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(curUserPtr->username, sender.username) == 0) {
            sender.balance -= amount;
            lseek(fd1, -1*sizeof(sender), SEEK_CUR);
            write(fd1, &sender, sizeof(sender));
            lseek(fd1, 0, SEEK_SET);
            txnRecord.senderBalAfterTxn = sender.balance;
            while ((bytes_read = read(fd1, &receiver, sizeof(receiver))) > 0){
                if (bytes_read != sizeof(receiver)) {
                    safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
                    continue;
                }
                if (strcmp(rcvr, receiver.username) == 0){
                    receiver.balance += amount;
                    txnRecord.receiverBalAfterTxn = receiver.balance;
                    lseek(fd1, -1*sizeof(receiver), SEEK_CUR);
                    write(fd1, &receiver, sizeof(receiver));
                    write(fd2, &txnRecord, sizeof(txnRecord));
                    apply_lock(fd2, F_UNLCK);

                    break;
                }
            }
            *curUserPtr = sender;
            break;
        }
    }
    apply_lock(fd1, F_UNLCK);
    close(fd2);
    close(fd1);
    return *curUserPtr;
}

void getTxnDetails(int client_fd, struct User *curUserPtr){
    int fd = open(TXNSFILE, O_RDONLY);
    struct Txn txnRecord;
    ssize_t bytes_read;
    int i = 0;
    apply_lock(fd, F_RDLCK);
    while ((bytes_read = read(fd, &txnRecord, sizeof(txnRecord))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(txnRecord)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(curUserPtr->username, txnRecord.sender) == 0) {
            char temp[1024];
            snprintf(temp, sizeof(temp), "Sender: %s, Receiver: %s, Amount: %0.2f, Your Balance After Txn: %0.2f", 
            txnRecord.sender, txnRecord.receiver, txnRecord.amt, txnRecord.senderBalAfterTxn);
            send(client_fd, temp, sizeof(temp), 0);
        }
        else if (strcmp(curUserPtr->username, txnRecord.receiver) == 0) {
            char temp[1024];
            snprintf(temp, sizeof(temp), "Sender: %s, Receiver: %s, Amount: %0.2f, Your Balance After Txn: %0.2f", 
            txnRecord.sender, txnRecord.receiver, txnRecord.amt, txnRecord.receiverBalAfterTxn);
            send(client_fd, temp, sizeof(temp), 0);
        }
    }
    char temp[1024] = "END";
    send(client_fd, temp, sizeof(temp), 0);
    apply_lock(fd, F_UNLCK);
    close(fd);
}

struct User changePassword_server(struct User *curUserPtr, char newPassword[1024]){
    int fd = open(USERDETAILSFILE, O_RDWR);
    apply_lock(fd, F_WRLCK);
    struct User record;
    ssize_t bytes_read;

    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(curUserPtr->username, record.username) == 0) {
            if (record.isActive){
                strcpy(record.password, newPassword);
                lseek(fd, -1*sizeof(record), SEEK_CUR);
                write(fd, &record, sizeof(record));
            }
            *curUserPtr = record;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return *curUserPtr;
}

void addFeedback(char username[MAXSTR], char feedback[FEEDBACKLEN]) {
    struct Fback fb;
    strcpy(fb.username, username);
    strcpy(fb.feedback, feedback);

    int fd = open(FBFILE, O_WRONLY | O_APPEND);
    apply_lock(fd, F_WRLCK);
    write(fd, &fb, sizeof(fb));
    apply_lock(fd, F_UNLCK);
    close(fd);
}

int addLoan(char username[MAXSTR], char manager[MAXSTR], float loanAmount){
    int loanID;
    int fdi = open(LOANSIDFILE, O_RDWR);
    apply_lock(fdi, F_WRLCK);
    read(fdi, &loanID, sizeof(loanID));
    lseek(fdi, 0, SEEK_SET);
    loanID++;
    write(fdi, &loanID, sizeof(loanID));
    apply_lock(fdi, F_UNLCK);
    close(fdi);
    printf("New loanID = %d\n", loanID);
    
    struct Loan loan = { .loanAmount = loanAmount, .status = LAPPLIED, .loanID = loanID };

    strcpy(loan.username, username);
    strcpy(loan.manager, manager);
    strcpy(loan.employee, "NULL");
    ssize_t bytes;
    int flag = 0;

    int fd = open(LOANSFILE, O_WRONLY | O_APPEND);
    apply_lock(fd, F_WRLCK);
    if((bytes = write(fd, &loan, sizeof(loan))) == sizeof(loan) ) flag = 1;
    apply_lock(fd, F_UNLCK);
    close(fd);
    return flag;
}

void changeCustomerStatus(char username[MAXSTR], int stat){
    int fd = open(USERDETAILSFILE, O_RDWR);
    apply_lock(fd, F_WRLCK);
    struct User record;
    ssize_t bytes_read;
    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(username, record.username) == 0) {
            record.isActive = stat;
            lseek(fd, -1*sizeof(record), SEEK_CUR);
            write(fd, &record, sizeof(record));
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
}

void viewAllLoanApplications(int client_fd) {
    int fd = open(LOANSFILE, O_RDONLY);
    struct Loan loan;
    ssize_t bytes_read;
    apply_lock(fd, F_RDLCK);
    while ((bytes_read = read(fd, &loan, sizeof(loan))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(loan)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        char temp[1024];
        snprintf(temp, sizeof(temp), "LoanID: %d, Customer: %s, Manager: %s, Employee: %s, Loan Amount: %0.2f, Loan Status: %d", 
        loan.loanID, loan.username, loan.manager, loan.employee, loan.loanAmount, loan.status);
        send(client_fd, temp, sizeof(temp), 0);
    }
    char temp[1024] = "END";
    send(client_fd, temp, sizeof(temp), 0);
    apply_lock(fd, F_UNLCK);
    close(fd);

}

void viewAllFeedbacks (int client_fd){
    int fd = open(FBFILE, O_RDONLY);
    struct Fback fb;
    ssize_t bytes_read;
    apply_lock(fd, F_RDLCK);
    while ((bytes_read = read(fd, &fb, sizeof(fb))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(fb)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        char temp[FEEDBACKLEN + 2*MAXSTR];
        snprintf(temp, sizeof(temp), "Username: %s, Feedback: %s", 
        fb.username, fb.feedback);
        send(client_fd, temp, sizeof(temp), 0);
    }
    char temp[1024] = "END";
    send(client_fd, temp, sizeof(temp), 0);
    apply_lock(fd, F_UNLCK);
    close(fd);
}

int validateLoanID(int loanID){
    int fd = open(LOANSFILE, O_RDONLY);
    struct Loan loan;
    ssize_t bytes_read;
    int flag = 0;
    apply_lock(fd, F_RDLCK);
    while ((bytes_read = read(fd, &loan, sizeof(loan))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(loan)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (loan.loanID == loanID){
            flag = 1;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return flag;
}

int assignLoan(int loanID, char employee[MAXSTR]){
    int fd = open(LOANSFILE, O_RDWR);
    int flag = 0;
    struct Loan loan;
    ssize_t bytes_read;
    apply_lock(fd, F_RDLCK);
    while ((bytes_read = read(fd, &loan, sizeof(loan))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(loan)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (loan.loanID == loanID && loan.status == LAPPLIED){
            strcpy(loan.employee, employee);
            loan.status = LASSIGNED;
            lseek(fd, -1*sizeof(loan), SEEK_CUR);
            write(fd, &loan, sizeof(loan));
            flag = 1;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return flag;
}

int checkUniqueUsername(char username[MAXSTR]){
    int flag = 1;
    int fd = open(USERDETAILSFILE, O_RDONLY);
    apply_lock(fd, F_RDLCK);
    struct User record;
    ssize_t bytes_read;
    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(username, record.username) == 0) {
            if (record.isActive) flag = 0;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return flag;
}

int addNewCustomer(struct User *newCustomerPtr){
    int flag = 0;
    int fd = open(USERDETAILSFILE, O_WRONLY | O_APPEND);
    ssize_t bytes;
    apply_lock(fd, F_WRLCK);
    if((bytes = write(fd, newCustomerPtr, sizeof(*newCustomerPtr))) == sizeof(*newCustomerPtr) ) flag = 1;
    apply_lock(fd, F_UNLCK);
    close(fd);
    return flag;
}

int modifyCustomer(char username[MAXSTR], char firstName[MAXSTR], char lastName[MAXSTR]){
    int flag = 0;
    int fd = open(USERDETAILSFILE, O_RDWR);
    apply_lock(fd, F_WRLCK);
    struct User record;
    ssize_t bytes_read;
    while ((bytes_read = read(fd, &record, sizeof(record))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(record)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(username, record.username) == 0) {
            strcpy(record.firstName, firstName);
            strcpy(record.lastName, lastName);
            lseek(fd, -1*sizeof(record), SEEK_CUR);
            write(fd, &record, sizeof(record));
            flag = 1;
            break;
        }
    }
    apply_lock(fd, F_UNLCK);
    close(fd);
    return flag;
}

void viewAllAssignedLoanApplications(char username[MAXSTR], int client_fd){
    int fd = open(LOANSFILE, O_RDONLY);
    struct Loan loan;
    ssize_t bytes_read;
    apply_lock(fd, F_RDLCK);
    while ((bytes_read = read(fd, &loan, sizeof(loan))) > 0) {
        // Check for a partial read, which might mean a corrupt file
        if (bytes_read != sizeof(loan)) {
            safe_write(STDERR_FILENO, "Warning: Corrupt data file encountered.\n");
            continue;
        }
        if (strcmp(loan.employee, username) == 0) {
            char temp[1024];
            snprintf(temp, sizeof(temp), "LoanID: %d, Customer: %s, Manager: %s, Employee: %s, Loan Amount: %0.2f, Loan Status: %d", 
            loan.loanID, loan.username, loan.manager, loan.employee, loan.loanAmount, loan.status);
            send(client_fd, temp, sizeof(temp), 0);
        }
    }
    char temp[1024] = "END";
    send(client_fd, temp, sizeof(temp), 0);
    apply_lock(fd, F_UNLCK);
    close(fd);

    
}
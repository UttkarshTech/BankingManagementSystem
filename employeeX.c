#include "genutilsX.h"
#include "employeeX.h"


void EmployeeMenu_server(int client_fd, struct User *curUserPtr){
    int choice = 1;
    while (choice) {
        printf("waiting for customer %s's choice...\n", curUserPtr->username);
        read(client_fd, &choice, sizeof(choice));
        switch (choice){
            case 0 : {
                *curUserPtr = logout_server(client_fd, curUserPtr);
                if (curUserPtr->isLoggedIn)
                    printf("customer %s failed to logout\n", curUserPtr->username);
                else
                    printf("customer %s successfully logged out\n", curUserPtr->username);
                send(client_fd, curUserPtr, sizeof(struct User), 0);
            }
            break;
            case 1 : {
                *curUserPtr = getDetails_server(client_fd, curUserPtr);
                send(client_fd, curUserPtr, sizeof(struct User), 0);
            }
            break;
            case 2 : {
                float amt;
                read(client_fd, &amt, sizeof(amt));

                *curUserPtr = deposit_server(client_fd, curUserPtr, amt);
                send(client_fd, curUserPtr, sizeof(struct User), 0);
            }
            break;
            case 3 : {
                float amt;
                read(client_fd, &amt, sizeof(amt));

                *curUserPtr = withdraw_server(client_fd, curUserPtr, amt);
                send(client_fd, curUserPtr, sizeof(struct User), 0);
            }
            break;
            case 4 : {

                read(client_fd, curUserPtr, sizeof(struct User));

                float amount;
                read(client_fd, &amount, sizeof(amount));
                int flag = validateWithdrawal(curUserPtr->username, amount);
                send(client_fd, &flag, sizeof(flag), 0);

                if (flag == 1){
                    char rcvr[MAXSTR];
                    read(client_fd, rcvr, sizeof(rcvr));
                    flag = validateUsername(rcvr);
                    send(client_fd, &flag, sizeof(flag), 0);
                    if (flag == 1){
                        //make transaction
                        *curUserPtr = makeTxn(curUserPtr, amount, rcvr);
                        send(client_fd, curUserPtr, sizeof(struct User), 0);
                    } else printf("Txn cancelled.\n");
                } else printf("Txn cancelled.\n");
            }
            break;
            case 5 : {
                read(client_fd, curUserPtr, sizeof(struct User));
                getTxnDetails(client_fd, curUserPtr);
            }
            break;
            case 6 : {
                read(client_fd, curUserPtr, sizeof(struct User));
                float amount;
                char manager[MAXSTR];
                read(client_fd, &amount, sizeof(amount));
                read(client_fd, manager, sizeof(manager));
                int flag = validateUsernameAndRole(manager, RM);
                send(client_fd, &flag, sizeof(flag), 0);
                if (flag == 1){
                    flag = addLoan(curUserPtr->username, manager, amount);
                    send(client_fd, &flag, sizeof(flag), 0);
                }
            }
            break;
            case 7 : {
                read(client_fd, curUserPtr, sizeof(struct User));
                char feedback[FEEDBACKLEN];
                
                read(client_fd, feedback, sizeof(feedback));
                if (strlen(feedback) == 0)
                    break;
                addFeedback(curUserPtr->username, feedback);
            } 
            break;
            case 8 : {
                read(client_fd, curUserPtr, sizeof(struct User));
                int flag;
                read(client_fd, &flag, sizeof(flag));
                if (flag){
                    char newPassword[MAXSTR];
                    read(client_fd, &newPassword, sizeof(newPassword));
                    *curUserPtr = changePassword_server(curUserPtr, newPassword);
                    send(client_fd, curUserPtr, sizeof(struct User), 0);
                }
            }
            break;
            case 9 : {
                int flag = 0;
                struct User newCustomer = {.role = RC, .isActive = 1, .isLoggedIn = 0, .balance = 0.0 };
                char username[MAXSTR], firstName[MAXSTR], lastName[MAXSTR];
                read(client_fd, username, sizeof(username));
                flag = checkUniqueUsername(username);
                send(client_fd, &flag, sizeof(flag), 0);
                if (flag == 1){
                    read(client_fd, firstName, sizeof(firstName));
                    read(client_fd, lastName, sizeof(lastName));
                    strcpy(newCustomer.username, username);
                    strcpy(newCustomer.firstName, firstName);
                    strcpy(newCustomer.lastName, lastName);
                    strcpy(newCustomer.password, "default");
                    flag = addNewCustomer(&newCustomer);
                    send(client_fd, &flag, sizeof(flag), 0);
                }
            }
            break;
            case 10 : {
                int flag = 0;
                char username[MAXSTR], firstName[MAXSTR], lastName[MAXSTR];
                read(client_fd, username, sizeof(username));
                flag = validateUsernameAndRole(username, RC);
                send(client_fd, &flag, sizeof(flag), 0);
                if (flag == 1){
                    read(client_fd, firstName, sizeof(firstName));
                    read(client_fd, lastName, sizeof(lastName));
                    flag = modifyCustomer(username, firstName, lastName);
                    send(client_fd, &flag, sizeof(flag), 0);
                }

            }
            break;
            case 11: {
                read(client_fd, curUserPtr, sizeof(struct User));
                viewAllAssignedLoanApplications(curUserPtr->username, client_fd);
            }
            break;
            case 12 : {
                int loanID = 0;
                int flag = 0;
                read(client_fd, curUserPtr, sizeof(struct User));
                read(client_fd, &loanID, sizeof(loanID));
                flag = validateLoanID(loanID);
                send(client_fd, &flag, sizeof(flag), 0);
                if (flag == 1){
                    int stat;
                    read(client_fd, &stat, sizeof(stat));
                    flag = changeStatLoan(loanID, stat);
                    send(client_fd, &flag, sizeof(flag), 0);
                }
            }
            break;
            default : break;
        }
    }
}

void EmployeeMenu_client(int sock_fd, struct User *curUserPtr){
    int choice = 1;
    while (choice){
        if (curUserPtr->isActive){
            printf("\n\n---EMPLOYEE MENU---\n1. View Account Balance\n2. Deposit Money\n3. Withdraw Money\n4. Transfer Funds\n5. View All Transactions\n6. Apply for a Loan\n7. Leave a feedback\n8. Change Password\n9. Add new customer\n10. Modify customer details\n11. View assigned loans\n12. Approve/Reject loans\n0. Logout\n\n");
            printf("Your choice : ");
            scanf("%d", &choice);
        } else {
            printf(ACCOUNTDISABLEDMSG);
            choice = 0;
        }
        send(sock_fd, &choice, sizeof(choice), 0);
        switch (choice){
            case 0 : {
                send(sock_fd, curUserPtr, sizeof(struct User), 0);
                read(sock_fd, curUserPtr, sizeof(struct User));
                if (curUserPtr->isLoggedIn)
                    printf("Log off failed...\n");
                else
                    printf("Logged out successfully!\n");
            }
            break;
            case 1 : {
                send(sock_fd, curUserPtr, sizeof(struct User), 0);
                read(sock_fd, curUserPtr, sizeof(struct User));
                if (curUserPtr->isActive == 1)
                    printf("Account Details\nAccount Number: %s\nAccount Balance: %0.2f\n", 
                    curUserPtr->accountNum, curUserPtr->balance);
            }
            break;
            case 2 : {
                float amt;
                printf("Enter amount to deposit : ");
                scanf("%f", &amt);
                send(sock_fd, &amt, sizeof(amt), 0);
                send(sock_fd, curUserPtr, sizeof(struct User), 0);
                read(sock_fd, curUserPtr, sizeof(struct User));
                if (curUserPtr->isActive == 1)
                    printf("Deposit Succesful\nAccount Details\nAccount Number: %s\nAccount Balance: %0.2f\n",
                    curUserPtr->accountNum, curUserPtr->balance);
            }
            break;
            case 3 : {
                float amt, prevBal = curUserPtr->balance;
                printf("Enter amount to withdraw : ");
                scanf("%f", &amt);
                send(sock_fd, &amt, sizeof(amt), 0);
                send(sock_fd, curUserPtr, sizeof(struct User), 0);
                read(sock_fd, curUserPtr, sizeof(struct User));
                if (curUserPtr->isActive == 1){
                    if (prevBal != curUserPtr->balance)
                        printf("Withdraw Succesful\nAccount Details\nAccount Number: %s\nAccount Balance: %0.2f\n",
                        curUserPtr->accountNum, curUserPtr->balance);
                    else
                        printf("Withdraw Unsuccessful, not enough funds in account to withdraw requested amount.\n");
                }
            }
            break;
            case 4 : {
                send(sock_fd, curUserPtr, sizeof(struct User), 0);
                float amt, prevBal = curUserPtr->balance;
                int flag;

                //check if withdrawal of amount is possible
                printf("Enter amount to transfer : ");
                scanf("%f", &amt);
                send(sock_fd, &amt, sizeof(amt), 0);
                read(sock_fd, &flag, sizeof(flag));

                if (flag == 1){
                    //check is receiver is valid
                    char rcvr[MAXSTR];
                    printf("Enter the username of the receiver : ");
                    scanf("%s", rcvr);

                    send(sock_fd, rcvr, sizeof(rcvr), 0);
                    read(sock_fd, &flag, sizeof(flag));

                    if (flag == 1){
                        read(sock_fd, curUserPtr, sizeof(struct User));
                        if (curUserPtr->isActive == 1){
                            if (prevBal != curUserPtr->balance)
                                printf("Transaction Succesful\nAccount Details\nAccount Number: %s\nAccount Balance: %0.2f\n",
                                curUserPtr->accountNum, curUserPtr->balance);
                            else
                                printf("Transaction Unsuccessful, not enough funds in account to withdraw requested amount.\n%0.2f, %0.2f\n", prevBal, curUserPtr->balance);
                        }
                    } else switch(flag){
                        case 0 : printf("Transaction Unsuccessful, receiver not found.\n");
                        break;
                        case -1 : printf("Transaction Unsuccessful, receiver account disabled.\n");
                        break;
                        default : printf("Transaction Unsuccessful.\n");
                    }
                } else switch (flag) {
                    case -1 : curUserPtr->isActive = 0;
                    break;
                    case -2 : printf("Transaction Unsuccessful, not enough funds in sender's account to withdraw requested amount.\n");
                    break;
                    default : printf("Transaction Unsuccessful");
                }
            }
            break;
            case 5 : {
                send(sock_fd, curUserPtr, sizeof(struct User), 0);
                char temp[1024];
                read(sock_fd, temp, sizeof(temp));
                if (strcmp(temp, "END") == 0){
                    printf("No transaction history.\n");
                    break;
                }
                printf("\n---Transaction History---\n");
                while (strcmp(temp, "END") != 0){
                    printf("%s\n", temp);
                    read(sock_fd, temp, sizeof(temp));
                }
            }
            break;
            case 6 : {
                send(sock_fd, curUserPtr, sizeof(struct User), 0);
                float amount;
                char manager[MAXSTR];
                printf("Enter loan amount : ");
                scanf("%f", &amount);
                printf("Enter manager username : ");
                scanf("%s", manager);
                send(sock_fd, &amount, sizeof(amount), 0);
                send(sock_fd, manager, sizeof(manager), 0);
                int flag;

                read(sock_fd, &flag, sizeof(flag));
                if (flag != 1){
                    printf("%s is not a manager.\n", manager);
                    break;
                }

                read(sock_fd, &flag, sizeof(flag));
                if (flag == 1){
                    printf("You have successfully applied for the loan!\n");
                    break;
                } else printf("Loan application failed.\n");

            }
            break;
            case 7 : {
                send(sock_fd, curUserPtr, sizeof(struct User), 0);
                char feedback[FEEDBACKLEN];
                printf("Please enter your feedback : ");
                scanf(" %[^\n]", feedback);
                send(sock_fd, feedback, sizeof(feedback), 0);
                printf("Feedback added!\n");
            }
            break;
            case 8 : {
                send(sock_fd, curUserPtr, sizeof(struct User), 0);
                char oldPassword[MAXSTR], newPassword[MAXSTR];
                int flag = 0;
                printf("Enter old Password : ");
                scanf("%s", oldPassword);
                if (strcmp(curUserPtr->password, oldPassword) != 0){
                    printf("Passwords do not match.\n");
                    send(sock_fd, &flag, sizeof(flag), 0);
                    break;
                }
                flag = 1;
                send(sock_fd, &flag, sizeof(flag), 0);
                printf("Enter new Password : ");
                scanf("%s", newPassword);
                send(sock_fd, &newPassword, sizeof(newPassword), 0);
                read(sock_fd, curUserPtr, sizeof(struct User));
                if (curUserPtr->isActive){
                    printf("Password changed!\n");
                    break;
                }
            }
            break;
            case 9 : {
                int flag = 0;
                char username[MAXSTR], firstName[MAXSTR], lastName[MAXSTR];
                printf("Enter username for new customer : ");
                scanf("%s", username);
                send(sock_fd, username, sizeof(username), 0);
                read(sock_fd, &flag, sizeof(flag));
                if (flag != 1){
                    printf("Username already taken...\n");
                    break;
                }

                printf("Enter first name of new customer : ");
                scanf("%s", firstName);
                printf("Enter last name of new customer : ");
                scanf("%s", lastName);
                send(sock_fd, firstName, sizeof(firstName), 0);
                send(sock_fd, lastName, sizeof(lastName), 0);
                read(sock_fd, &flag, sizeof(flag));
                if (flag != 1){
                    printf("New Customer not added...\n");
                    break;
                }
                printf("New Customer added succesfully!\n");
                break;
            }
            break;
            case 10 : {
                int flag = 0;
                char username[MAXSTR], firstName[MAXSTR], lastName[MAXSTR];
                printf("Enter username for new customer : ");
                scanf("%s", username);
                send(sock_fd, username, sizeof(username), 0);
                read(sock_fd, &flag, sizeof(flag));
                if (flag != 1){
                    printf("Customer does not exist...\n");
                    break;
                }
                printf("Enter new first name of customer : ");
                scanf("%s", firstName);
                printf("Enter new last name of customer : ");
                scanf("%s", lastName);
                send(sock_fd, firstName, sizeof(firstName), 0);
                send(sock_fd, lastName, sizeof(lastName), 0);
                read(sock_fd, &flag, sizeof(flag));
                if (flag != 1){
                    printf("Customer not modified...\n");
                    break;
                }
                printf("Customer modified succesfully!\n");
                break;
            }
            break;
            case 11 : {
                send(sock_fd, curUserPtr, sizeof(curUserPtr), 0);
                char temp[1024];
                read(sock_fd, temp, sizeof(temp));
                if (strcmp(temp, "END") == 0){
                    printf("No loan applications.\n");
                    break;
                }
                printf("\n---Loan Applications---\n");
                while (strcmp(temp, "END") != 0){
                    printf("%s\n", temp);
                    read(sock_fd, temp, sizeof(temp));
                }

            }
            break;
            case 12 : {
                send(sock_fd, curUserPtr, sizeof(curUserPtr), 0);
                int loanID, flag = 0;
                printf("Enter loanID : ");
                scanf("%d", &loanID);
                send(sock_fd, &loanID, sizeof(loanID), 0);
                read(sock_fd, &flag, sizeof(flag));
                if (flag == 0){
                    printf("Invalid loan id.\n");
                    break;
                }
                else if (flag == -1){
                    printf("Loan not assigned to you.\n");
                    break;
                }
                else if (flag == 1){
                    int stat;
                    printf("Approve (1) / Reject (0) ? : ");
                    scanf("%d", &stat);
                    if (stat >= 1) stat = LAPPROVED;
                    else stat = LREJECTED;
                    send(sock_fd, &stat, sizeof(stat), 0);
                    read(sock_fd, &flag, sizeof(flag));
                    if (flag != 1){
                        printf("Action failed.\n");
                        break;
                    } else {
                        printf("Action successful!\n");
                        break;
                    }
                }

            }
            break;
            default : break;
        }
    }
}
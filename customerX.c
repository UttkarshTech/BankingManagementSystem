#include "genutilsX.h"
#include "customerX.h"


void CustomerMenu_server(int client_fd, struct User *curUserPtr){
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
            default : break;
        }
    }
}

void CustomerMenu_client(int sock_fd, struct User *curUserPtr){
    int choice = 1;
    while (choice){
        if (curUserPtr->isActive){
            printf("\n\n---CUSTOMER MENU---\n1. View Account Balance\n2. Deposit Money\n3. Withdraw Money\n4. Transfer Funds\n5. View All Transactions\n6. Apply for a Loan\n7. Leave a feedback\n8. Change Password\n0. Logout\n\n");
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
            default : break;
        }
    }
}
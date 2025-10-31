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
            default : break;
        }
    }
}

void CustomerMenu_client(int sock_fd, struct User *curUserPtr){
    int choice = 1;
    while (choice){
        if (curUserPtr->isActive){
            printf("\n\n---CUSTOMER MENU---\n1. View Account Balance\n2. Deposit Money\n3. Withdraw Money\n0. Logout\n");
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
            default : break;
        }
    }
}
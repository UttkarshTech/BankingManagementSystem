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
            default : break;
        }
    }
}

void CustomerMenu_client(int sock_fd, struct User *curUserPtr){
    int choice = 1;
    while (choice){
        if (curUserPtr->isActive){
            printf("---CUSTOMER MENU---\n1. View Account Balance\n0. Logout\n");
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
                    printf("%d Account Details\nAccount Number: %s\nAccount Balance: %0.2f\n",
                    curUserPtr->isActive, curUserPtr->accountNum, curUserPtr->balance);
            }
            break;
            default : break;
        }
    }
}
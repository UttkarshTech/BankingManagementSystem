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

#define BACKLOG 5
#define MAXSTR 50

#define USERDETAILSFILE "users.dat"

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


//funcs
void safe_write(int fd, const char *msg);

int safe_read_line(int fd, char *buffer, int size);

void apply_manlock(int fd, int lock_type);

struct User verifyLogin(int client_fd, struct User *curUserPtr);

struct User getDetails_server(int client_fd, struct User *curUserPtr);

struct User logout_server(int client_fd, struct User *curUserPtr);

#endif
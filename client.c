#include "genutilsX.h"
#include "customerX.h"
#include "employeeX.h"
#include "adminX.h"
#include "managerX.h"


struct User curUser = {.isActive = 1, .isLoggedIn = 0};

int create_client_socket() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock_fd;
}

void connect_to_server(int sock_fd, const char *ip, int port, struct sockaddr_in *addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
        perror("Invalid server address");
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        perror("Connection to server failed");
        exit(EXIT_FAILURE);
    }
}

void login_client(int sock_fd){
    printf("Enter username : ");
    scanf("%s", curUser.username);
    printf("Enter password : ");
    scanf("%s", curUser.password);
    send(sock_fd, &curUser, sizeof(curUser), 0);
    read(sock_fd, &curUser, sizeof(curUser));
    if (strlen(curUser.firstName) == 0){
        printf("Authentication Failed\n");
        return;
    }
    else if (!curUser.isActive){
        printf("Your account is disabled, please contact your bank manager.\n");
        return;
    }
    else if (curUser.isLoggedIn == 2){
        printf("Authentication Failed : Already logged in another session.\n");
        return;
    }
    else {
        printf("Authentication Succesful!\nWelcome %s %s!\n", curUser.firstName, curUser.lastName);
        return;
    }
}

void AdminMenu(){}
void ManagerMenu(){}
void EmployeeMenu(){}


int main() {
    int sock_fd = create_client_socket();
    struct sockaddr_in server_addr;

    connect_to_server(sock_fd, SERVER_IP, SERVER_PORT, &server_addr);

    login_client(sock_fd);

    if (curUser.isLoggedIn == 1){
        switch(curUser.role){
            case RC : CustomerMenu_client(sock_fd, &curUser); break;
            case RA : AdminMenu_client(sock_fd, &curUser); break;
            case RM : ManagerMenu_client(sock_fd, &curUser); break;
            case RE : EmployeeMenu_client(sock_fd, &curUser); break;
            default : break;
        }
    }
    
    close(sock_fd);
    return 0;
}
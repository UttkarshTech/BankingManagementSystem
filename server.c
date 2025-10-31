#include "genutilsX.h"
#include "customerX.h"
#include "employeeX.h"
#include "adminX.h"
#include "managerX.h"

void* client_handler(void* arg) {
    pthread_t tid = pthread_self();
    printf("Thread %lu handling this client\n", (unsigned long)tid);
    int client_fd = *(int*)arg;
    free(arg);
    struct User curUser = {.isActive = 1, .isLoggedIn = 0};

    //receive login credentials
    curUser = verifyLogin(client_fd, &curUser);

    //determine role and apt menu
    if (curUser.isLoggedIn == 1){
        switch(curUser.role){
            case RC : CustomerMenu_server(client_fd, &curUser); break;
            case RA : AdminMenu_server(client_fd, &curUser); break;
            case RM : ManagerMenu_server(client_fd, &curUser);; break;
            case RE : EmployeeMenu_server(client_fd, &curUser); break;
            default : break;
        }
    }

    //close connection and join main thread
    close(client_fd);
    printf("Thread %lu joining main thread\n", (unsigned long)tid);
    pthread_exit(NULL);

}

int setup_server_socket(int port, struct sockaddr_in *addr) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr*)addr, sizeof(*addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(sock_fd, BACKLOG) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", port);
    return sock_fd;
}

int main() {
    struct sockaddr_in server_addr;
    int server_fd = setup_server_socket(SERVER_PORT, &server_addr);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);
        int *client_fd = malloc(sizeof(int));
        if (!client_fd) { perror("malloc"); continue; }

        *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);
        if (*client_fd < 0) {
            perror("Accept failed");
            free(client_fd);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, client_handler, client_fd);
        pthread_detach(tid);
    }

    close(server_fd);
    return 0;
}

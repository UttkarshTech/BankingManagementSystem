#ifndef MANAGERX_H
#define MANAGERX_H

#include "genutilsX.h"

void ManagerMenu_client(int sock_fd, struct User *curUserPtr);
void ManagerMenu_server(int client_fd, struct User *curUserPtr);

#endif
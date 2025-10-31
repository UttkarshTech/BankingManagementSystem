#ifndef ADMINX_H
#define ADMINX_H

#include "genutilsX.h"

void AdminMenu_client(int sock_fd, struct User *curUserPtr);
void AdminMenu_server(int client_fd, struct User *curUserPtr);

#endif
#ifndef CUSTOMERX_H
#define CUSTOMERX_H

#include "genutilsX.h"

void CustomerMenu_client(int sock_fd, struct User *curUserPtr);
void CustomerMenu_server(int client_fd, struct User *curUserPtr);

#endif
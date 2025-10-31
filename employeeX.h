#ifndef EMPLOYEEX_H
#define EMPLOYEEX_H

#include "genutilsX.h"

void EmployeeMenu_client(int sock_fd, struct User *curUserPtr);
void EmployeeMenu_server(int client_fd, struct User *curUserPtr);

#endif
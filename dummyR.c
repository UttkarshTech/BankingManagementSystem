#include "genutilsX.h"

int main() {
    FILE *file_ptr;
    struct User record_buffer;
    file_ptr = fopen(USERDETAILSFILE, "rb");
    if (file_ptr == NULL) {
        perror("Error opening for reading");
        return 1;
    }
    printf("--- Reading records from customer.dat ---\n");
    while (fread(&record_buffer, sizeof(record_buffer), 1, file_ptr) == 1) {
        printf("Username : %s\nPassword : %s\nFirst Name : %s\nLast Name : %s\nAccount Number : %s\n", 
               record_buffer.username, 
               record_buffer.password, record_buffer.firstName, record_buffer.lastName,
                record_buffer.accountNum
            );
        printf("Is Active : %d\nIs Logged In : %d\nRole : %d\nBalance : %f\n",
            record_buffer.isActive, record_buffer.isLoggedIn,
            record_buffer.role, record_buffer.balance
        );
        printf("----------\n");
    }
    printf("--- End of records ---\n");
    fclose(file_ptr);
    return 0;
}
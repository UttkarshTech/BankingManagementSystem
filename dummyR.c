#include "genutilsX.h"

// Must be the *exact* same struct definition

int main() {
    FILE *file_ptr;
    struct User record_buffer; // A buffer to hold one record at a time

    // 1. Open the file in binary read mode ("rb")
    file_ptr = fopen(USERDETAILSFILE, "rb");
    if (file_ptr == NULL) {
        perror("Error opening for reading");
        return 1;
    }

    printf("--- Reading records from customer.dat ---\n");

    // 2. Read one struct at a time until the end of the file
    // fread returns 1 as long as it successfully reads one full record
    while (fread(&record_buffer, sizeof(record_buffer), 1, file_ptr) == 1) {
        
        // 3. Print the record we just read
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

    // 4. Close the file
    fclose(file_ptr);

    return 0;
}